#include "RHIShaderCompiler.h"
#include "IO.h"

namespace RHI {
    namespace {
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
        
        const IShaderCompiler* compilerContext = GetCompilerContext();
        const ShaderCompilerContext* context = GetLocalThreadCompilerContext(compilerContext);
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
        
        const IShaderCompiler* compilerContext = GetCompilerContext();
        const ShaderCompilerContext* context = GetLocalThreadCompilerContext(compilerContext);
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
