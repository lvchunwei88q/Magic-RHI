#include "RHIResourceD3D11.h"
#include <Common/RHIDefinitions.h>
#include <Common/Check.h>
#include "RHID3D11.h"
#include <d3dcompiler.h>

#include <IO.h>

namespace RHI
{
    namespace {
        bool CompileShaderToBlob(const std::string& source, const std::string& entryPoint, 
                            const std::string& profile, bool enableDebug,const std::vector<ShaderMacro>& macros,
                            ComPtr<ID3DBlob>& outBlob)
        {
            UINT flags = 0;
#ifdef _DEBUG
            flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
            if (enableDebug)
            {
                flags |= D3DCOMPILE_DEBUG;
            }

            std::vector<D3D_SHADER_MACRO> dx11Macros;
            for (const auto& macro : macros)
            {
                dx11Macros.push_back({macro.name.c_str(), macro.definition.c_str()});
            }
            dx11Macros.push_back({nullptr, nullptr});

            ComPtr<ID3DBlob> errorBlob;
            HRESULT hr = D3DCompile(
                source.c_str(),
                source.size(),
                nullptr,
                dx11Macros.data(),
                nullptr,
                entryPoint.c_str(),
                profile.c_str(),
                flags,
                0,
                outBlob.GetAddressOf(),
                errorBlob.GetAddressOf()
            );

            if (FAILED(hr))
            {
                std::string errorMsg = "Shader compilation failed";
                
                // 添加 HRESULT 信息
                char hrMsg[64];
                sprintf_s(hrMsg, " (HRESULT: 0x%08X)", static_cast<unsigned int>(hr));
                errorMsg += hrMsg;
                errorMsg += ":\n";
                
                if (errorBlob)
                {
                    errorMsg += reinterpret_cast<const char*>(errorBlob->GetBufferPointer());
                }
                else
                {
                    // 没有详细错误信息时，根据 HRESULT 提供提示
                    switch (hr)
                    {
                    case E_OUTOFMEMORY:
                        errorMsg += "Out of memory";
                        break;
                    case E_INVALIDARG:
                        errorMsg += "Invalid argument (check shader target or entry point)";
                        break;
                    default:
                        errorMsg += "Unknown error";
                        break;
                    }
                }
                
                Core::ErrorCapture::Capture(errorMsg.c_str());
#if RHI_ENABLE_DEBUG_INFO
                ThrowErrorMessage(errorMsg.c_str());
#endif
                return false;
            }

            return true;
        }
    }
    bool ShaderCompilerBackendD3D11::Initialize()
    {
        m_Initialization = InitialState::Initialize;
        return true;
    }

    void ShaderCompilerBackendD3D11::Shutdown()
    {
        m_Initialization = InitialState::Shutdown;
    }

    bool ShaderCompilerBackendD3D11::IsValid() const
    {
        return m_Initialization == InitialState::Initialize;
    }

    ShaderCompileOptionInternal ShaderCompilerBackendD3D11::AddBackendArguments(const ShaderCompileOptions& options)
    {
        ShaderCompileOptionInternal internalOptions = options;
        // The target compiler for DXC SPIR-V is -spirv
        internalOptions.targetCompilerMode = "-spirv";
        internalOptions.SPIR_V_TargetEnv = SPIRVCompileEnvironment().c_str();
        return internalOptions;
    }

    void ShaderCompilerBackendD3D11::PostProcessShader(const ShaderCompileOptions& options, const ShaderCompileResult& in_result, ShaderCompileResult& out_result)
    {
        // TODO: post-processing required for D3D11
        out_result = in_result;
    }

    ShaderReflectionGenerationMode ShaderCompilerBackendD3D11::GetShaderReflectionGenerationMode()
    {
        return ShaderReflectionGenerationMode{ShaderReflectionGenerationMode::ReflectionGenerationMode::Use_InitialCompileCache};
    }

    // ==================================================== Tools ====================================================
    std::string ShaderCompilerBackendD3D11::SPIRVCompileEnvironment() const
    {
        return "vulkan1.0";
    }
}
