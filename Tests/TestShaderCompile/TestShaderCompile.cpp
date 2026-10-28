#include <Subsystem/Subsystem.h>
#include <CoreMinimal.h>
#include <RHI.hpp>
#include <IO.h>
#include "../Common/Test.h"
#include "RHIPlatformDetection.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

namespace
{
    constexpr size_t WorkerCount = 3;

    struct ShaderTestInput
    {
        std::string fileName;
        std::string profile;
        std::string sourceCode;
    };

    struct WorkerResult
    {
        bool passed = true;
        std::ostringstream report;
    };

    std::mutex LogMutex;

    void LogCapture(const char* severity, const std::string& message)
    {
        std::lock_guard<std::mutex> lock(LogMutex);
        std::cout << severity << ": " << message << std::endl;
    }

    void LogError(const std::string& message)
    {
        LogCapture("Error", message);
    }

    void LogWarning(const std::string& message)
    {
        LogCapture("Warning", message);
    }

    void LogInfo(const std::string& message)
    {
        LogCapture("Info", message);
    }

    bool ReadTextFile(const std::string& path, std::string& contents)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            return false;
        }

        contents.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        return true;
    }

    void RecordFailure(WorkerResult& result, const std::string& message)
    {
        result.passed = false;
        result.report << "  FAIL: " << message << '\n';
    }

    bool HasResource(
        const RHI::SPIRVReflection& reflection,
        bool RHI::SPIRVResource::* property)
    {
        for (const RHI::SPIRVResource& resource : reflection.resources)
        {
            if (resource.*property)
            {
                return true;
            }
        }
        return false;
    }

    void PrintReflection(
        size_t workerIndex,
        const std::string& shaderName,
        const RHI::SPIRVReflection& reflection,
        WorkerResult& result)
    {
        result.report << "  [" << workerIndex << "] " << shaderName
                      << " reflection: entry=" << reflection.entryPoint
                      << ", resources=" << reflection.resources.size()
                      << ", total cbuffer bytes=" << reflection.totalConstantBufferSize << '\n';

        for (const RHI::SPIRVResource& resource : reflection.resources)
        {
            result.report << "    resource=" << resource.name
                          << ", set=" << resource.set
                          << ", binding=" << resource.binding
                          << ", size=" << resource.size
                          << ", cbuffer=" << resource.isConstantBuffer
                          << ", texture=" << resource.isTexture
                          << ", sampler=" << resource.isSampler << '\n';

            for (const auto& member : resource.members)
            {
                result.report << "      member=" << member.first
                              << ", offset=" << member.second << '\n';
            }

            if (resource.isConstantBuffer && (resource.size == 0 || resource.members.empty()))
            {
                RecordFailure(result, "constant buffer reflection is missing size or member data");
            }
        }

        size_t groupedResourceCount = 0;
        for (const auto& [set, resources] : reflection.resourcesBySet)
        {
            (void)set;
            groupedResourceCount += resources.size();
        }

        if (reflection.entryPoint != "main")
        {
            RecordFailure(result, shaderName + " reflection entry point was not main");
        }
        if (groupedResourceCount != reflection.resources.size())
        {
            RecordFailure(result, shaderName + " resourcesBySet does not match the resource list");
        }

        if (shaderName == "vertex")
        {
            if (!HasResource(reflection, &RHI::SPIRVResource::isConstantBuffer))
            {
                RecordFailure(result, "vertex shader reflection did not contain a constant buffer");
            }
        }
        else if (shaderName == "pixel")
        {
            if (!HasResource(reflection, &RHI::SPIRVResource::isConstantBuffer)
                || !HasResource(reflection, &RHI::SPIRVResource::isTexture)
                || !HasResource(reflection, &RHI::SPIRVResource::isSampler))
            {
                RecordFailure(result, "pixel shader reflection is missing expected buffer/texture/sampler resources");
            }
        }
        else if (shaderName == "compute")
        {
            const bool hasConstantBuffer = HasResource(reflection, &RHI::SPIRVResource::isConstantBuffer);
            const bool hasStorageBuffer = std::any_of(
                reflection.resources.begin(),
                reflection.resources.end(),
                [](const RHI::SPIRVResource& resource)
                {
                    return !resource.isConstantBuffer && !resource.isTexture && !resource.isSampler;
                });
            if (!hasConstantBuffer || !hasStorageBuffer)
            {
                RecordFailure(result, "compute shader reflection is missing expected constant/storage buffers");
            }
        }
    }

    void CompileShader(
        size_t workerIndex,
        const std::string& shaderName,
        const ShaderTestInput& shader,
        const std::string& shaderPath,
        bool useSourceCode,
        bool optimize,
        bool debugInfo,
        RHI::RHIType backendType,
        RHI::IShaderCompiler& compiler,
        WorkerResult& result)
    {
        RHI::ShaderCompileOptions options{};
        options.targetProfile = shader.profile;
        options.entryPoint = "main";
        options.optimize = optimize;
        options.debugInfo = debugInfo;
        options.includePaths.push_back(shaderPath + "\\");

        RHI::ShaderCompileSource source{};
        source.sourceType = useSourceCode
            ? RHI::ShaderCompileSource::SourceType::SourceCode
            : RHI::ShaderCompileSource::SourceType::SourcePath;
        source.sourceDescription = useSourceCode
            ? shader.sourceCode
            : shaderPath + "\\" + shader.fileName;

        if (shaderName == "vertex")
        {
            std::vector<RHI::InputElementDesc> inputLayout = {
                { "POSITION", 0, RHI::RHITextureFormat::R32G32B32A32_FLOAT, 0,
                    RHI::InputElementDesc::APPEND_ALIGNED_ELEMENT, RHI::InputClassification::PerVertexData, 0 },
                { "COLOR", 0, RHI::RHITextureFormat::R32G32B32A32_FLOAT, 0,
                    RHI::InputElementDesc::APPEND_ALIGNED_ELEMENT, RHI::InputClassification::PerVertexData, 0 },
            };
            source.postProcessArgs = std::make_shared<RHI::VertexShaderPostProcessArgs>(inputLayout);
        }
        else if (shaderName == "pixel")
        {
            source.postProcessArgs = std::make_shared<RHI::PixelShaderPostProcessArgs>();
        }
        else
        {
            source.postProcessArgs = std::make_shared<RHI::ComputeShaderPostProcessArgs>();
        }

        result.report << "  [" << workerIndex << "] compiling " << shaderName
                      << " (" << shader.profile << ", source="
                      << (useSourceCode ? "memory" : "file")
                      << ", optimize=" << optimize << ", debug=" << debugInfo << ")\n";

        RHI::ShaderCompileResult compileResult = compiler.Compile(options, source);
        if (!compileResult.success)
        {
            RecordFailure(result, shaderName + " compilation failed: " + compileResult.errorMessage);
            return;
        }
        if (!compileResult.warningMessage.empty())
        {
            result.report << "  [" << workerIndex << "] " << shaderName
                          << " warning: " << compileResult.warningMessage << '\n';
        }
        if (compileResult.byteCode.empty())
        {
            RecordFailure(result, shaderName + " compilation returned empty bytecode");
            return;
        }
        if (compileResult.bytecodeSize != compileResult.byteCode.size())
        {
            RecordFailure(result, shaderName + " bytecodeSize does not match the bytecode buffer");
        }

        RHI::CreateShaderDesc shaderDesc = compiler.CreateShaderDescription();
        const size_t descriptionByteCount = std::visit(
            [](const auto& byteCode) { return byteCode.size() * sizeof(typename std::decay_t<decltype(byteCode)>::value_type); },
            shaderDesc.byteCode);
        result.report << "  [" << workerIndex << "] " << shaderName
                      << " shader description: entry=" << shaderDesc.entryPoint
                      << ", bytecode bytes=" << descriptionByteCount
                      << ", format=" << (shaderDesc.byteCode.index() == 0 ? "uint8" : "uint32") << '\n';
        if (shaderDesc.entryPoint != options.entryPoint || descriptionByteCount == 0)
        {
            RecordFailure(result, shaderName + " shader description is invalid");
        }
        const size_t expectedByteCodeVariant = backendType == RHI::RHIType::D3D12 ? 0 : 1;
        if (shaderDesc.byteCode.index() != expectedByteCodeVariant)
        {
            RecordFailure(result, shaderName + " shader description uses the wrong backend bytecode format");
        }

        RHI::SPIRVReflection reflection = compiler.Reflection();
        PrintReflection(workerIndex, shaderName, reflection, result);
    }

    void RunCompilerWorker(
        size_t workerIndex,
        RHI::RHIType backendType,
        RHI::IShaderCompiler* compiler,
        const std::string& shaderPath,
        const std::array<ShaderTestInput, 3>& shaders,
        WorkerResult& result)
    {
        std::unique_ptr<RHIShaderCompiler> context = RHIModule::GetCompilerContextController();
        if (!context)
        {
            RecordFailure(result, "failed to create a compiler context controller");
            return;
        }

        bool contextInitialized = false;
        bool compilerStarted = false;

        try
        {
            contextInitialized = context->InitializeCompilerThreadContext();
            if (!contextInitialized)
            {
                RecordFailure(result, "failed to initialize thread-local compiler context");
                context->ShutdownCompilerThreadContext();
                return;
            }

            compiler->BeginCompiler();
            compilerStarted = true;

            const bool useSourceCode = workerIndex == 1;
            const bool optimize = workerIndex != 2;
            const bool debugInfo = workerIndex == 0;

            CompileShader(workerIndex, "vertex", shaders[0], shaderPath, useSourceCode,
                optimize, debugInfo, backendType, *compiler, result);
            CompileShader(workerIndex, "pixel", shaders[1], shaderPath, useSourceCode,
                optimize, debugInfo, backendType, *compiler, result);
            CompileShader(workerIndex, "compute", shaders[2], shaderPath, useSourceCode,
                optimize, debugInfo, backendType, *compiler, result);

            compiler->EndCompiler();
            compilerStarted = false;
        }
        catch (const std::exception& exception)
        {
            RecordFailure(result, std::string("worker threw an exception: ") + exception.what());
        }
        catch (...)
        {
            RecordFailure(result, "worker threw an unknown exception");
        }

        if (compilerStarted)
        {
            try
            {
                compiler->EndCompiler();
            }
            catch (...)
            {
                RecordFailure(result, "failed to end compiler after worker error");
            }
        }

        try
        {
            context->ShutdownCompilerThreadContext();
        }
        catch (const std::exception& exception)
        {
            RecordFailure(result, std::string("failed to shut down compiler context: ") + exception.what());
        }
        catch (...)
        {
            RecordFailure(result, "failed to shut down compiler context with an unknown exception");
        }
    }

    bool RunBackendTest(
        RHI::RHIType backendType,
        RHIAPILoader& loader,
        const std::string& shaderPath,
        const std::array<ShaderTestInput, 3>& shaders)
    {
        std::cout << "\nRunning shader tests with " << RHI::RHITypeToString(backendType) << std::endl;
        if (!loader.Load(backendType))
        {
            std::cerr << "Failed to load " << RHI::RHITypeToString(backendType) << std::endl;
            return false;
        }

        RHI::IShaderCompiler* compiler = RHIModule::GetCompilerPipeline();
        if (compiler == nullptr)
        {
            std::cerr << "Failed to get the shader compiler pipeline." << std::endl;
            loader.Unload();
            return false;
        }

        std::array<WorkerResult, WorkerCount> results;
        std::array<std::thread, WorkerCount> workers;
        try
        {
            for (size_t i = 0; i < WorkerCount; ++i)
            {
                workers[i] = std::thread(
                    RunCompilerWorker,
                    i,
                    backendType,
                    compiler,
                    std::cref(shaderPath),
                    std::cref(shaders),
                    std::ref(results[i]));
            }
        }
        catch (const std::exception& exception)
        {
            std::cerr << "Failed to start shader compiler worker: " << exception.what() << std::endl;
            for (std::thread& worker : workers)
            {
                if (worker.joinable())
                {
                    worker.join();
                }
            }
            loader.Unload();
            return false;
        }

        for (std::thread& worker : workers)
        {
            worker.join();
        }

        bool allWorkersPassed = true;
        for (size_t i = 0; i < WorkerCount; ++i)
        {
            std::cout << "\nWorker " << i << " report:\n" << results[i].report.str();
            allWorkersPassed = allWorkersPassed && results[i].passed;
        }

        loader.Unload();
        return allWorkersPassed;
    }
}

void TestShaderCompilation()
{
    Core::SubsystemControl::Init();
    {
        Core::ErrorCapture::RegisterCaptureFunction(LogError);
        Core::WarningCapture::RegisterCaptureFunction(LogWarning);
        Core::InfoCapture::RegisterCaptureFunction(LogInfo);

        std::cout << "Testing concurrent shader compilation and reflection..." << std::endl;

        RHIAPILoader* loader = RHIModule::GetRHILoader();
        const std::string executablePath = IO::ToNarrowString(IO::AbsolutePath::Get().GetExecutableDirectory());
        const std::string shaderPath = executablePath + "\\..\\..\\Tests\\TestShaderCompile\\Shader";
        std::array<ShaderTestInput, 3> shaders = {{
            { "testVS.hlsl", "vs_6_0", {} },
            { "testPS.hlsl", "ps_6_0", {} },
            { "testCS.hlsl", "cs_6_0", {} },
        }};

        bool filesLoaded = true;
        for (ShaderTestInput& shader : shaders)
        {
            const std::string path = shaderPath + "\\" + shader.fileName;
            if (!ReadTextFile(path, shader.sourceCode))
            {
                std::cerr << "Failed to read shader test file: " << path << std::endl;
                filesLoaded = false;
            }
        }

        if (!filesLoaded)
        {
            EXPECT_TRUE(filesLoaded);
            Core::SubsystemControl::Uninstall();
            return;
        }

        const RHI::RHIType availableRhis = RHI::GetBestAvailableRHI();
        bool anyBackendTested = false;
        bool allBackendsPassed = true;
        const std::array<RHI::RHIType, 2> backendTypes = {
            RHI::RHIType::D3D12,
            RHI::RHIType::VulKan
        };
        for (RHI::RHIType backendType : backendTypes)
        {
            if (!EnumHasAnyFlags(availableRhis, backendType))
            {
                continue;
            }

            anyBackendTested = true;
            const bool backendPassed = RunBackendTest(backendType, *loader, shaderPath, shaders);
            EXPECT_TRUE(backendPassed);
            allBackendsPassed = allBackendsPassed && backendPassed;
        }

        if (!anyBackendTested)
        {
            std::cerr << "No supported RHI found for shader compilation test." << std::endl;
            EXPECT_TRUE(anyBackendTested);
        }

        std::cout << (allBackendsPassed && anyBackendTested
        ? "Shader compilation, concurrency, and reflection tests completed successfully."
        : "Shader compilation, concurrency, or reflection tests failed.") << std::endl;
    }

    Core::SubsystemControl::Uninstall();
}

TEST_MAIN(TestShaderCompilation)
