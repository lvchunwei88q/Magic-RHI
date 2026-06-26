#include "RHIShaderCompiler.h"
#include "RHIResource.h"
#include "IO.h"

namespace RHI
{
    // ========== SPIR-V Compiler ==========
    // Get SPIR-V compiler instance
    IShaderCompiler* IRHIModule::GetSPIRVCompiler(){
        return &HLSLToSPIRVCompiler::Get();
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
    ShaderCompileResult HLSLToSPIRVCompiler::SPIRVCompileFromString(
        const std::string& hlslSource,
        const ShaderCompileOptions& options) {
        
        LocalShaderCompileOption localOptions = options;
        localOptions.targetCompilerMode = "-spirv";
        localOptions.targetEnv = GetSPIRVTargetEnv();
        return CompileInternal(hlslSource, localOptions, *m_Context);
    }

    // ========== Compile from file ==========
    ShaderCompileResult HLSLToSPIRVCompiler::SPIRVCompileFromFile(
        const std::string& filePath,
        const ShaderCompileOptions& options) {
        std::wstring filePathW = IO::ToWideString(filePath);
        if (!IO::Exists(filePathW)) {
            // File not found, return error
            ShaderCompileResult result;
            result.success = false;
            result.errorMessage = "Shader file not found: " + filePath;
            return result;
        }

        std::string content = IO::ReadAllText(filePathW);
        
        LocalShaderCompileOption localOptions = options;
        localOptions.targetCompilerMode = "-spirv";
        localOptions.targetEnv = GetSPIRVTargetEnv();
        return CompileInternal(content, localOptions, *m_Context);
    }
}
