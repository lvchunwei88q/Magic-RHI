#include "RHIResourceD3D12.h"
#include <Common/RHIDefinitions.h>
#include <Common/Check.h>
#include "RHID3D12.h"

#include <IO.h>

namespace RHI
{
    bool ShaderCompilerBackendD3D12::Initialize()
    {

        m_Initialization = InitialState::Initialize;
        return true;
    }

    void ShaderCompilerBackendD3D12::Shutdown()
    {
        m_Initialization = InitialState::Shutdown;
    }

    bool ShaderCompilerBackendD3D12::IsValid() const
    {
        return m_Initialization == InitialState::Initialize;
    }

    ShaderCompileOptionInternal ShaderCompilerBackendD3D12::AddBackendArguments(const ShaderCompileOptions& options)
    {
        ShaderCompileOptionInternal internalOptions = options;
        // The target compiler for DXC HLSL is empty
        internalOptions.targetCompilerMode = "";
        return internalOptions;
    }

    void ShaderCompilerBackendD3D12::PostProcessShader(const ShaderCompileOptions& options, const ShaderCompileResult& in_result, ShaderCompileResult& out_result)
    {
        // DX12 doesn't need post-processing
        out_result = in_result;
    }

    ShaderReflectionGenerationMode ShaderCompilerBackendD3D12::GetShaderReflectionGenerationMode()
    {
        return ShaderReflectionGenerationMode{ShaderReflectionGenerationMode::ReflectionGenerationMode::Use_SourceCacheCompile};
    }

    // ==================================================== Tools ====================================================
    std::string ShaderCompilerBackendD3D12::SPIRVCompileEnvironment() const
    {
        return "vulkan1.3";
    }
}
