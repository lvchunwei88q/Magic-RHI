#include "RHIShaderCompiler.h"
#include "IO.h"

namespace RHI {
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

    // ========== HLSL Compiler ==========
    // Get HLSL compiler instance
    IShaderCompiler* IRHIModule::GetHLSLCompiler(){
        return &HLSLCompiler::Get();
    }

    HLSLCompiler::HLSLCompiler() = default;
    HLSLCompiler::~HLSLCompiler() = default;

    // ========== HLSL Compiler Functions ==========
    // Compile from source string
    ShaderCompileResult HLSLCompiler::HLSLCompileFromString(
        const std::string& hlslSource,
        const ShaderCompileOptions& options
    ) {
        LocalShaderCompileOption localOptions = options;
        // Set default target compiler mode to HLSL
        localOptions.targetCompilerMode = "-fcgl";

        const ShaderCompilerContext* context = GetCompilerContext();
        if (context == nullptr) {
            // Compiler context not initialized, return error
            ShaderCompileResult result;
            result.errorMessage = "Shader compiler context not initialized!";
            result.success = false;
            return result;
        }

        return CompileInternal(hlslSource, localOptions, *context);
    }

    // Compile from file
    ShaderCompileResult HLSLCompiler::HLSLCompileFromFile(
        const std::string& filePath,
        const ShaderCompileOptions& options
    ) {
        LocalShaderCompileOption localOptions = options;
        // Set default target compiler mode to HLSL
        localOptions.targetCompilerMode = "-fcgl";

        std::wstring filePathW = IO::ToWideString(filePath);
        if (!IO::Exists(filePathW)) {
            // File not found, return error
            ShaderCompileResult result;
            result.success = false;
            result.errorMessage = "Shader file not found: " + filePath;
            return result;
        }

        std::string content = IO::ReadAllText(filePathW);

        const ShaderCompilerContext* context = GetCompilerContext();
        if (context == nullptr) {
            // Compiler context not initialized, return error
            ShaderCompileResult result;
            result.errorMessage = "Shader compiler context not initialized!";
            result.success = false;
            return result;
        }

        return CompileInternal(content, localOptions, *context);
    }
}
