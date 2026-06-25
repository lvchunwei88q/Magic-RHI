#include "RHISPIRV.h"
#include "RHIResource.h"
#include "IO.h"

namespace RHI
{
    // ========== SPIR-V Compiler ==========
    // Get SPIR-V compiler instance
    SPIRVProcessor* IRHIModule::GetSPIRVCompiler(){
        return &HLSLToSPIRVCompiler::Get();
    }

    // ========== SPIR-V Reflection ==========
    // Get SPIR-V reflection instance
    SPIRVProcessor* IRHIModule::GetSPIRVReflection(){
        return &SPIRVGenerationReflection::Get();
    }

    HLSLToSPIRVCompiler::HLSLToSPIRVCompiler() {
        m_Context = std::make_unique<ShaderCompilerContext>();
        m_Initialized = m_Context->Initialize();
        if (!m_Initialized) {
            // init shader compiler failed
            Core::ErrorCapture::Capture("Failed to initialize shader compiler!");
        }
    }

    HLSLToSPIRVCompiler::~HLSLToSPIRVCompiler() = default;

    // ========== Compile from source string ==========
    SPIRVCompileResult HLSLToSPIRVCompiler::CompileFromString(
        const std::string& hlslSource,
        const SPIRVCompileOptions& options) {
        return CompileInternal(hlslSource, "", options);
    }

    // ========== Compile from file ==========
    SPIRVCompileResult HLSLToSPIRVCompiler::CompileFromFile(
        const std::string& filePath,
        const SPIRVCompileOptions& options) {
        std::wstring filePathW = IO::ToWideString(filePath);
        if (!IO::Exists(filePathW)) {
            // File not found, return error
            SPIRVCompileResult result;
            result.success = false;
            result.errorMessage = "Shader file not found: " + filePath;
            return result;
        }

        std::string content = IO::ReadAllText(filePathW);
        return CompileInternal(content, filePath, options);
    }

    // ========== Internal compile core function ==========
    SPIRVCompileResult HLSLToSPIRVCompiler::CompileInternal(
        const std::string& hlslSource,
        const std::string& sourcePath,
        const SPIRVCompileOptions& options) {

        SPIRVCompileResult result;

        // Check ShaderCompilerContext is initialized
        if (!m_Initialized) {
            result.errorMessage = "Shader compiler context not initialized!";
            return result;
        }

        // Create source Blob
        std::wstring wSource = IO::ToWideString(hlslSource);
        ComPtr<IDxcBlobEncoding> pSourceBlob;
        HRESULT hr = m_Context->utils->CreateBlob(
            wSource.c_str(),
            static_cast<UINT32>(wSource.size() * sizeof(wchar_t)),
            CP_UTF8,
            &pSourceBlob
        );
        if (FAILED(hr)) {
            result.errorMessage = "Failed to create source blob. HRESULT: " + std::to_string(hr);
            return result;
        }

        // Create DxcBuffer wrapper
        DxcBuffer sourceBuffer;
        sourceBuffer.Ptr = pSourceBlob->GetBufferPointer();
        sourceBuffer.Size = pSourceBlob->GetBufferSize();
        sourceBuffer.Encoding = 0;

        // Build argument list
        std::vector<std::wstring> m_ArgTempStorage;
        auto args = BuildArguments(options, sourcePath,m_ArgTempStorage);

        // Execute compilation
        ComPtr<IDxcResult> pResult;
        hr = m_Context->compiler->Compile(
            &sourceBuffer,
            args.data(),
            static_cast<UINT32>(args.size()),
            m_Context->includeHandler.Get(),
            IID_PPV_ARGS(&pResult)
        );
        if (FAILED(hr)) {
            result.errorMessage = "Compile() failed. HRESULT: " + std::to_string(hr);
            return result;
        }

        // Get compilation status
        HRESULT status;
        if (FAILED(pResult->GetStatus(&status))) {
            result.errorMessage = "Failed to get compilation status!";
            return result;
        }
        if (FAILED(status)) {
            ComPtr<IDxcBlobEncoding> pErrors;
            if (SUCCEEDED(pResult->GetErrorBuffer(&pErrors)) && pErrors) {
                result.errorMessage = reinterpret_cast<const char*>(pErrors->GetBufferPointer());
            } else {
                result.errorMessage = "Unknown compilation error!";
            }
            return result;
        }

        // Get errors (optional)
        ComPtr<IDxcBlobEncoding> pErrors;
        if (SUCCEEDED(pResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr)) &&
            pErrors && pErrors->GetBufferSize() > 0) {
            result.errorMessage = reinterpret_cast<const char*>(pErrors->GetBufferPointer());
        }

        // Get SPIR-V bytecode
        ComPtr<IDxcBlob> pBytecode;
        if (FAILED(pResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pBytecode), nullptr))) {
            result.errorMessage = "Failed to get output bytecode!";
            return result;
        }

        // Copy bytecode to result
        const uint32_t* data = reinterpret_cast<const uint32_t*>(pBytecode->GetBufferPointer());
        size_t wordCount = pBytecode->GetBufferSize() / sizeof(uint32_t);
        result.spirv.assign(data, data + wordCount);
        result.bytecodeSize = wordCount * sizeof(uint32_t);
        result.success = true;

        return result;
    }

    // ========== Build DXC arguments ==========
    std::vector<const wchar_t*> HLSLToSPIRVCompiler::BuildArguments(
        const SPIRVCompileOptions& options,
        const std::string& sourcePath,
        std::vector<std::wstring>& m_ArgStorage) {
        
        std::vector<const wchar_t*> args;

        auto addArg = [&](const std::string& str) {
            m_ArgStorage.push_back(IO::ToWideString(str));
            args.push_back(m_ArgStorage.back().c_str());  
        };

        // Base arguments
        args.push_back(L"-spirv");                  // Output SPIR-V bytecode
        args.push_back(L"-E");
        addArg(options.entryPoint);
        args.push_back(L"-T");
        addArg(options.targetProfile);

        // Optimization level
        if (options.optimize) {
            args.push_back(L"-O3");
        } else {
            args.push_back(L"-O0");
        }

        // Debug information
        if (options.debugInfo) {
            args.push_back(DXC_ARG_DEBUG);
            args.push_back(L"-fspv-debug=vulkan-with-source");
            args.push_back(L"-fspv-extension=SPV_KHR_non_semantic_info");
        }

        // HLSL version
        args.push_back(L"-HV");
        addArg(options.hlslVersion);

        // SPIR-V target environment (GetSPIRVTargetEnv)
        std::string targetEnv = GetSPIRVTargetEnv();
        args.push_back(L"-fspv-target-env");
        addArg(targetEnv);

        // 16-bit types enabled
        if (options.enable16BitTypes) {
            args.push_back(L"-enable-16bit-types");
        }

        // Matrix packing enabled
        if (options.enableMatrixPacking) {
            args.push_back(L"-fspv-matrix-packing");
        }

        // Macro definitions
        for (const auto& macro : options.defines) {
            std::string def = macro.name + "=" + macro.definition;
            args.push_back(L"-D");
            addArg(def);
        }

        // Include paths
        for (const auto& path : options.includePaths) {
            args.push_back(L"-I");
            addArg(path);
        }

        // Other common flags
#if RHI_ENABLE_DEBUG_INFO
        args.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);       // Warning as error
#else
        args.push_back(L"-no-warn");  // Reduce warnings
#endif

        // Add source path if provided
        if (!sourcePath.empty()) {
            addArg(sourcePath);
        }

        return args;
    }

    SPIRVReflection SPIRVGenerationReflection::ExtractReflection(const std::vector<uint32_t>& spirv){
        SPIRVReflection reflection;
        return reflection;
    }
}
