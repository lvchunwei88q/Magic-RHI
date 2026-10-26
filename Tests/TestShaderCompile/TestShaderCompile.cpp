#include <Subsystem/Subsystem.h>
#include <iostream>
#include <CoreMinimal.h>
#include <RHI.hpp>
#include <IO.h>
#include "../Common/Test.h"
#include "RHIPlatformDetection.h"

void TestShaderCompilation()
{
    Core::SubsystemControl::Init();
    {
        std::cout << "Testing Shader Compilation..." << std::endl;

        // Initialize RHI Loader
        RHI::RHIType type = RHI::GetBestAvailableRHI();
        RHIAPILoader* loader = RHIModule::GetRHILoader();
        if (EnumHasAnyFlags(type, RHI::RHIType::D3D12)) {
            ASSERT_TRUE(loader->Load(RHI::RHIType::D3D12));
        } else if (EnumHasAnyFlags(type, RHI::RHIType::VulKan)) {
            ASSERT_TRUE(loader->Load(RHI::RHIType::VulKan));
        } else if (EnumHasAnyFlags(type, RHI::RHIType::D3D11)) {
            ASSERT_TRUE(loader->Load(RHI::RHIType::D3D11));
        } else {
            std::cerr << "No suitable RHI found for shader compilation test!" << std::endl;
            return;
        }
        
        std::unique_ptr<RHIShaderCompiler> compilerContextController = RHIModule::GetCompilerContextController();
        RHIShaderCompiler* Compiler = RHIModule::GetCompilerPipeline();

        ASSERT_TRUE(compilerContextController->InitializeCompilerThreadContext());
        ASSERT_TRUE(Compiler != nullptr);

        std::string exePath = IO::ToNarrowString(IO::AbsolutePath::Get().GetExecutableDirectory());
        // Note: You need to go back to the Test directory here
        std::string ShaderPath = exePath + "\\..\\..\\Tests\\TestShaderCompile\\Shader";
        
        RHI::ShaderCompileOptions vsOption{};
        vsOption.targetProfile = "vs_6_0";
        vsOption.debugInfo = true;
        vsOption.optimize = true;
        vsOption.entryPoint = "main";
        vsOption.includePaths.push_back(ShaderPath + "\\");

        RHI::ShaderCompileOptions psOption{};
        psOption.targetProfile = "ps_6_0";
        psOption.debugInfo = true;
        psOption.optimize = true;
        psOption.entryPoint = "main";
        psOption.includePaths.push_back(ShaderPath + "\\");

        Compiler->BeginCompiler();
        
        RHI::ShaderCompileSource Source{};
        Source.sourceType = RHI::ShaderCompileSource::SourceType::SourcePath;
        
        // Compile VS
        Source.sourceDescription = ShaderPath + "\\testVS.hlsl";
        std::vector<RHI::InputElementDesc> inputLayout = {
            { "POSITION", 0, RHI::RHITextureFormat::R32G32B32A32_FLOAT, 0, RHI::InputElementDesc::APPEND_ALIGNED_ELEMENT, RHI::InputClassification::PerVertexData, 0 },
            { "COLOR", 0, RHI::RHITextureFormat::R32G32B32A32_FLOAT, 0, RHI::InputElementDesc::APPEND_ALIGNED_ELEMENT, RHI::InputClassification::PerVertexData, 0 },
        };
        Source.postProcessArgs = std::make_shared<RHI::VertexShaderPostProcessArgs>(inputLayout);
        RHI::ShaderCompileResult vsResult = Compiler->Compile(vsOption, Source);
        RHI::CreateShaderDesc vsDesc = Compiler->CreateShaderDescription();

        // Compile PS
        Source.sourceDescription = ShaderPath + "\\testPS.hlsl";
        Source.postProcessArgs = nullptr;
        RHI::ShaderCompileResult psResult = Compiler->Compile(psOption, Source);
        RHI::CreateShaderDesc psDesc = Compiler->CreateShaderDescription();

        // Assertion result
        EXPECT_TRUE(vsResult.success);
        EXPECT_TRUE(psResult.success);
        
        if (!vsResult.success || !psResult.success) {
            std::cerr << "Error: " << vsResult.errorMessage + " or " + psResult.errorMessage << std::endl;
        }

        std::cout << "Shader compilation test completed." << std::endl;

        Compiler->EndCompiler();
        compilerContextController->ShutdownCompilerThreadContext();
    }
    Core::SubsystemControl::Uninstall();
}

TEST_MAIN(TestShaderCompilation)