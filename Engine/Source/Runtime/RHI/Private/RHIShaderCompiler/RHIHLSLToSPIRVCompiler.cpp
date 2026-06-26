#include "RHIShaderCompiler.h"
#include "IO.h"

namespace RHI
{
    namespace {
        const ShaderCompilerContext* GetCompilerContext() {
            const ShaderCompilerContext* context = CompilerContextController::Get().GetCompilerContext();
            if (context == nullptr) {
                // Compiler context not initialized, return error
                return nullptr;
            }
            return context;
        }
    }
    // ========== SPIR-V Compiler ==========
    // Get SPIR-V compiler instance
    IShaderCompiler* IRHIModule::GetSPIRVCompiler(){
        return &HLSLToSPIRVCompiler::Get();
    }

    HLSLToSPIRVCompiler::HLSLToSPIRVCompiler() {}
    HLSLToSPIRVCompiler::~HLSLToSPIRVCompiler() = default;

    // ========== Compile from source string ==========
    ShaderCompileResult HLSLToSPIRVCompiler::SPIRVCompileFromString(
        const std::string& hlslSource,
        const ShaderCompileOptions& options) {
        
        LocalShaderCompileOption localOptions = options;
        localOptions.targetCompilerMode = "-spirv";
        localOptions.targetEnv = GetSPIRVTargetEnv();

        const ShaderCompilerContext* context = GetCompilerContext();
        if (context == nullptr) {
            // Compiler context not initialized, return error
            ShaderCompileResult result;
            result.success = false;
            result.errorMessage = "Shader compiler context not initialized!";
            return result;
        }

        return CompileInternal(hlslSource, localOptions, *context);
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

        const ShaderCompilerContext* context = GetCompilerContext();
        if (context == nullptr) {
            // Compiler context not initialized, return error
            ShaderCompileResult result;
            result.success = false;
            result.errorMessage = "Shader compiler context not initialized!";
            return result;
        }

        return CompileInternal(content, localOptions, *context);
    }
}
