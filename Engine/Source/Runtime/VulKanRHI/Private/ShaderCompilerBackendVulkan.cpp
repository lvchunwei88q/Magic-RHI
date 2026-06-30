#include "RHIVulkan.h"

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

    
    ShaderReflectionGenerationMode ShaderCompilerBackendVulKan::GetShaderReflectionGenerationMode()
    {
        return ShaderReflectionGenerationMode{ShaderReflectionGenerationMode::ReflectionGenerationMode::Use_SourceCacheCompile};
    }

    // ==================================================== Tools ====================================================
    std::string ShaderCompilerBackendVulKan::SPIRVCompileEnvironment() const
    {
        return "vulkan1.3";
    }
}
