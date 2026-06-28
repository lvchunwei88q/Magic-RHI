#include "RHIResourceD3D11.h"
#include <Common/RHIDefinitions.h>
#include <Common/Check.h>
#include "RHID3D11.h"
#include <d3dcompiler.h>

#include <IO.h>

namespace RHI
{
    namespace
    {
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

        template<typename ShaderType, typename CreateFunc>
        std::unique_ptr<ShaderType> CompileShaderInternal(const CreateShaderDesc& desc,
                                                          const char* defaultProfile, CreateFunc createFunc)
        {
            // TODO: SPIRV shader type is not supported on D3D11
            if (desc.shaderType != CreateShaderDesc::ShaderType::SPIRV)
            {
                ThrowErrorMessage("SPIRV shader type is not supported on D3D11");
            }

            std::unique_ptr<ShaderType> shader = std::make_unique<ShaderType>(nullptr);
            return shader;
        }
    }

    bool CreateShaderD3D11::Initialize(Device* device)
    {
        m_Initialization = CoreDeviceInitialization::Initialize;
        m_pRHI = SafeCast<DeviceD3D11>(device);
        return true;
    }

    void CreateShaderD3D11::Shutdown()
    {
        m_Initialization = CoreDeviceInitialization::Shutdown;
        m_pRHI = nullptr;
    }

    bool CreateShaderD3D11::IsValid() const
    {
        return m_Initialization == CoreDeviceInitialization::Initialize;
    }

    std::unique_ptr<RHIVertexShader> CreateShaderD3D11::CreateVertexShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<VertexShaderD3D11>(desc, "vs_5_0",
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11VertexShader* {
                ID3D11VertexShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::unique_ptr<RHIPixelShader> CreateShaderD3D11::CreatePixelShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<PixelShaderD3D11>(desc, "ps_5_0",
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11PixelShader* {
                ID3D11PixelShader* pShader = nullptr;
                if (SUCCEEDED(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::unique_ptr<RHIGeometryShader> CreateShaderD3D11::CreateGeometryShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<GeometryShaderD3D11>(desc, "gs_5_0",
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11GeometryShader* {
                ID3D11GeometryShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::unique_ptr<RHIHullShader> CreateShaderD3D11::CreateHullShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<HullShaderD3D11>(desc, "hs_5_0",
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11HullShader* {
                ID3D11HullShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::unique_ptr<RHIDomainShader> CreateShaderD3D11::CreateDomainShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<DomainShaderD3D11>(desc, "ds_5_0",
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11DomainShader* {
                ID3D11DomainShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::unique_ptr<RHIComputeShader> CreateShaderD3D11::CreateComputeShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<ComputeShaderD3D11>(desc, "cs_5_0",
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11ComputeShader* {
                ID3D11ComputeShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    ShaderModelVersion CreateShaderD3D11::GetShaderModelVersion() const
    {
        // For DX11, it only supports SM5.0.
        return ShaderModelVersion::SM_5_0;
    }
}
