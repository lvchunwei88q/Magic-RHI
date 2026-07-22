#include "RHIVulKan.h"

namespace RHI
{
    namespace
    {
    }

    bool ShaderCompilerBackendVulKan::Initialize()
    {
        m_Initialization = InitialState::Initialize;
        return true;
    }

    void ShaderCompilerBackendVulKan::Shutdown()
    {
        m_Initialization = InitialState::Shutdown;
    }

    bool ShaderCompilerBackendVulKan::IsValid() const
    {
        return m_Initialization == InitialState::Initialize;
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
    
    ShaderPipelineGenerationMode ShaderCompilerBackendVulKan::GetShaderPipelineGenerationMode()
    {
        return ShaderPipelineGenerationMode{
            ShaderPipelineGenerationMode::ReflectionGenerationMode::Use_CompileResultCache,ShaderPipelineGenerationMode::ShaderSaveMode::Use_UINT32};
    }

    // ==================================================== Tools ====================================================
    std::string ShaderCompilerBackendVulKan::SPIRVCompileEnvironment() const
    {
        return "vulkan1.3";
    }
}
