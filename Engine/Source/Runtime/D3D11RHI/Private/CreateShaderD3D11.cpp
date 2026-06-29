#include "RHID3D11.h"
#include <d3dcompiler.h>

namespace RHI
{
    namespace
    {
        ComPtr<ID3DBlob> CreateBlobFromVector(const std::vector<uint8_t>& bytecode)
        {
            ComPtr<ID3DBlob> pBlob;
            // create Blob [citation:1]
            HRESULT hr = D3DCreateBlob(bytecode.size(), &pBlob);
            if (FAILED(hr)) {
                // Error: create Blob failed
                return nullptr; 
            }

            // use memcpy_s to copy data to Blob buffer [citation:2]
            if (memcpy_s(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), bytecode.data(), bytecode.size())) {
                // Error: memcpy_s failed
                return nullptr; 
            }

            // return ComPtr
            return pBlob;
        }

        template<typename ShaderType, typename CreateFunc>
        std::unique_ptr<ShaderType> CompileShaderInternal(ID3D11Device* pDevice, const CreateShaderDesc& desc, CreateFunc createFunc)
        {
            // TODO: SPIRV shader type is not supported on D3D11
            if (desc.GetUINT8ByteCode().size() == 0)
            {
                ThrowErrorMessage("Shader byte code is empty");
            }

            ComPtr<ID3DBlob> blob = CreateBlobFromVector(desc.GetUINT8ByteCode());
            if(blob == nullptr){
#if RHI_ENABLE_DEBUG_INFO
                ThrowErrorMessage("CreateBlobFromVector failed");
#endif
                return nullptr;
            }

            if(createFunc == nullptr){
#if RHI_ENABLE_DEBUG_INFO
                ThrowErrorMessage("CreateFunc is nullptr");
#endif
                return nullptr;
            }

            std::unique_ptr<ShaderType> shader = std::make_unique<ShaderType>(createFunc(pDevice, blob));

            // Different shaders might have different needs for data retention, like VS needing the original bytecode.
            if constexpr (std::is_same_v<ShaderType, VertexShaderD3D11>) {
                // Process vertex shader bytecode
                shader->SetByteBlob(blob.Get());
            } else if constexpr (std::is_same_v<ShaderType, PixelShaderD3D11>) {
                // Process pixel shader bytecode
            } else if constexpr (std::is_same_v<ShaderType, ComputeShaderD3D11>) {
                // Process compute shader bytecode
            }
            return shader;
        }
    }
    
    std::unique_ptr<RHIVertexShader> DeviceD3D11::CreateVertexShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<VertexShaderD3D11>(GetDevice(), desc,
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11VertexShader* {
                ID3D11VertexShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::unique_ptr<RHIPixelShader> DeviceD3D11::CreatePixelShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<PixelShaderD3D11>(GetDevice(), desc,
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11PixelShader* {
                ID3D11PixelShader* pShader = nullptr;
                if (SUCCEEDED(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::unique_ptr<RHIGeometryShader> DeviceD3D11::CreateGeometryShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<GeometryShaderD3D11>(GetDevice(), desc,
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11GeometryShader* {
                ID3D11GeometryShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::unique_ptr<RHIHullShader> DeviceD3D11::CreateHullShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<HullShaderD3D11>(GetDevice(), desc,
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11HullShader* {
                ID3D11HullShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::unique_ptr<RHIDomainShader> DeviceD3D11::CreateDomainShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<DomainShaderD3D11>(GetDevice(), desc,
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11DomainShader* {
                ID3D11DomainShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::unique_ptr<RHIComputeShader> DeviceD3D11::CreateComputeShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<ComputeShaderD3D11>(GetDevice(), desc,
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11ComputeShader* {
                ID3D11ComputeShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    ShaderModelVersion DeviceD3D11::GetShaderModelVersion() const
    {
        // For DX11, it only supports SM5.0.
        return ShaderModelVersion::SM_5_0;
    }
}
