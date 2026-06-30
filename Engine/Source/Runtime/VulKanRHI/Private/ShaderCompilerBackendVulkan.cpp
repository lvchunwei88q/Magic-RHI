#include "RHIVulKan.h"

namespace RHI
{
    namespace
    {
    }

    bool ShaderCompilerBackendVulKan::Initialize()
    {
        return true;
    }

    void ShaderCompilerBackendVulKan::Shutdown()
    {
    }

    bool ShaderCompilerBackendVulKan::IsValid() const
    {
        return true;
    }

    ShaderCompileOptionInternal ShaderCompilerBackendVulKan::AddBackendArguments(const ShaderCompileOptions& options)
    {
        ShaderCompileOptionInternal internalOptions = options;
        // The target compiler for DXC SPIR-V is -spirv
        internalOptions.targetCompilerMode = "-spirv";
        internalOptions.SPIR_V_TargetEnv = SPIRVCompileEnvironment().c_str();
        return internalOptions;
    }

    void ShaderCompilerBackendVulKan::PostProcessShader(const ShaderCompileOptions& options, const ShaderPostProcessArgs* postProcessArgs, 
        const ShaderCompileResult& in_result, ShaderCompileResult& out_result)
    {
        // Vulkan doesn't need post-processing
        if(postProcessArgs != nullptr){
            // ... Post-process shader
        }
        out_result = in_result;
    }
    
    ShaderReflectionGenerationMode ShaderCompilerBackendVulKan::GetShaderReflectionGenerationMode()
    {
        return ShaderReflectionGenerationMode{ShaderReflectionGenerationMode::ReflectionGenerationMode::Use_CompileResultCache};
    }

    // ==================================================== Tools ====================================================
    std::string ShaderCompilerBackendVulKan::SPIRVCompileEnvironment() const
    {
        return "vulkan1.3";
    }
}
