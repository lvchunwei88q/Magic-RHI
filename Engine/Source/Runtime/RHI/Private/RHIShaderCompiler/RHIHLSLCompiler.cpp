#include "RHIShaderCompiler.h"
#include "IO.h"

namespace RHI {
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
        // TODO: Compile HLSL source string
        return ShaderCompileResult {};
    }

    // Compile from file
    ShaderCompileResult HLSLCompiler::HLSLCompileFromFile(
        const std::string& filePath,
        const ShaderCompileOptions& options
    ) {
        // TODO: Compile HLSL file
        return ShaderCompileResult {};
    }
}
