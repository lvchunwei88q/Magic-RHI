#include <Common/Check.h>
#include "RHID3D11.h"
#include "RHIRootSignatureD3D11.h"

namespace RHI
{
    // --------------------------------- 
    // RootConstantDataD3D11
    // Normally, we shouldn't be creating code like Buffers at the spot with the signature, 
    // but we need to do it this way because we want DX11 to be compatible with root constants and be managed directly by the root signature.
    // --------------------------------- 

    RootConstantDataD3D11::RootConstantDataD3D11(ID3D11Device* pDevice,size_t location)
    {
        Location = location;
        ThrowIfFailed(CreateBuffer(pDevice));
    }
        
    RootConstantDataD3D11::~RootConstantDataD3D11()
    {
        ReleaseBuffer();
    }

    HRESULT RootConstantDataD3D11::CreateBuffer(ID3D11Device* pDevice)
    {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = MAX_BUFFER_SIZE;            // 256 bytes
        desc.Usage = D3D11_USAGE_DYNAMIC;            // Allows CPU update frequently
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        HRESULT hr = pDevice->CreateBuffer(&desc, nullptr, ConstantBuffer.GetAddressOf());
        if (SUCCEEDED(hr))
            UsedSize = 0;
        return hr;
    }

    HRESULT RootConstantDataD3D11::UpdateBuffer(ID3D11DeviceContext* pContext, const void* pData, uint32_t sizeInBytes, uint32_t destOffsetIn32BitValues)
    {
        if (!ConstantBuffer || !pData || sizeInBytes == 0 || sizeInBytes > MAX_BUFFER_SIZE)
            return E_INVALIDARG;

        // Convert to 1-byte units.
        uint32_t offsetInBytes = destOffsetIn32BitValues * 4;

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        HRESULT hr = pContext->Map(ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr))
            return hr;
        
        // Copy data to mapped memory
        memcpy((uint8_t*)mapped.pData + offsetInBytes, pData, sizeInBytes);            
        // Update cache as well
        memcpy(Cache.data() + offsetInBytes, pData, sizeInBytes);
        uint32_t endOffset = offsetInBytes + sizeInBytes;
        if (endOffset > UsedSize)
            UsedSize = endOffset;

        pContext->Unmap(ConstantBuffer.Get(), 0);
        return S_OK;
    }
        
    bool RootConstantDataD3D11::IsConsistentWithCache(const void* pData, uint32_t sizeInBytes, uint32_t destOffsetIn32BitValues) const
    {
        // Convert to 1-byte units.
        uint32_t offsetInBytes = destOffsetIn32BitValues * 4;
        if (offsetInBytes + sizeInBytes > UsedSize)
            return false;
        return memcmp(Cache.data() + offsetInBytes, pData, sizeInBytes) == 0;
    }

    // --------------------------------- 
    // RootConstantPoolD3D11
    // --------------------------------- 

    RootConstantPoolD3D11::RootConstantPoolD3D11(ID3D11Device* pDevice,ConstantSignatureLocation locations)
    {
        m_Pools.resize(locations.Location.size());
        for (size_t i = 0; i < locations.Location.size(); ++i)
        {
            // Create a buffer for each root constant.
            size_t rootConstantIndex = locations.Location[i];
            RootConstantDataD3D11 ConstantData(pDevice,rootConstantIndex);
            m_Pools[i] = ConstantData;
        }
    }

    RootConstantPoolD3D11::~RootConstantPoolD3D11()
    {
        for (size_t i = 0; i < m_Pools.size(); ++i)
        {
            m_Pools[i].ReleaseBuffer();
        }
    }

    // --------------------------------- 
    // RHIRootSignatureD3D11
    // --------------------------------- 

    RHIRootSignatureD3D11::RHIRootSignatureD3D11()
    {
    }

    RHIRootSignatureD3D11::~RHIRootSignatureD3D11()
    {
        Shutdown();
    }

    bool RHIRootSignatureD3D11::Initialize(Device* device, const RootSignatureDesc& desc)
    {
        if (device == nullptr){
#if RHI_ENABLE_DEBUG_INFO
                ThrowErrorMessage("Device is nullptr");
#endif
            return false;
        }
        RHID3D11* pDevice = SafeCast<RHID3D11>(device);
        m_Desc.Flags = desc.Flags;
        // We need to fill in the entire descriptor
        m_Desc.RootParameters.resize(desc.Parameters.size());
        for (size_t i = 0; i < desc.Parameters.size(); ++i)
        {   // Fill in the root parameters.
            const auto& param = desc.Parameters[i];
            m_Desc.RootParameters[i].Type = param.Type;
            m_Desc.RootParameters[i].Visibility = param.Visibility;

            // Fill in the root parameter union data.
            switch (param.Type)
            {
                case RootParameterType::Constants:
                    m_Desc.RootParameters[i].Constants.ShaderRegister = param.Constants.ShaderRegister;
                    break;
                case RootParameterType::CBV:
                case RootParameterType::SRV:
                case RootParameterType::UAV:
                    m_Desc.RootParameters[i].Descriptor.ShaderRegister = param.Descriptor.ShaderRegister;
                    break;
                default: // Unsupported root parameter type
                    break;
            }
        }

        // We need to create the corresponding Const Buffer based on the root constants in the root signature set by the user.
        ConstantSignatureLocation Locations;
        for (size_t i = 0; i < desc.Parameters.size(); ++i)
        {
            const auto& param = desc.Parameters[i];
            // if the parameter is a root constant, we need to create a buffer for it.
            if (param.Type == RootParameterType::Constants)
                // Record the location of the root constant in the root signature.
                Locations.Location.push_back(i);
        }

        m_Pool = std::make_unique<RootConstantPoolD3D11>(pDevice->GetDevice(), Locations);

        // TODO: Root signature data
        return true;
    }

    void RHIRootSignatureD3D11::Shutdown()
    {
        m_Pool.reset();
    }

    bool RHIRootSignatureD3D11::IsValid() const
    {
        return m_Pool != nullptr;
    }

    RootConstantDataD3D11* RHIRootSignatureD3D11::GetRootConstantData(uint32_t rootParameterIndex) const
    {
        bool existence = false;
        size_t pool_loction = 0;
        // We're checking if this slot exists
        for (const auto& pool : m_Pool->GetPools()) {
            if (pool.Location == rootParameterIndex) {
                existence = true;
                break;
            }
            // Finally, we add one to indicate the index
            pool_loction++;
        }
        if (!existence) {
            //Core::Error("Root constant index is out of range.");
            Core::ErrorCapture::Capture("Root constant index is out of range.");
            return nullptr;
        };
        return m_Pool->GetRootConstantData(pool_loction);
    }

    void RHIRootSignatureD3D11::SetRootSignatureResources(RHIRootSignatureD3D11* pRootSignature, ID3D11DeviceContext* pDeviceContext)
    {
        // Get the root signature descriptor.
        RootSignatureDescD3D11 Desc = pRootSignature->m_Desc;
        for (size_t i = 0; i < Desc.RootParameters.size(); ++i)
        {
            const auto& param = Desc.RootParameters[i];
            switch (param.Type) {
                case RootParameterType::Constants:
                    {
                        // Set the root constant. TODO: Set root constants for other shaders.
                        RootConstantDataD3D11* pRootConstantData = pRootSignature->GetRootConstantData(i);
                        pDeviceContext->PSSetConstantBuffers(param.Constants.ShaderRegister,1 ,pRootConstantData->ConstantBuffer.GetAddressOf());
                    }
                    break;
                case RootParameterType::CBV:
                case RootParameterType::SRV:
                case RootParameterType::UAV:
                    break;
                default: // Unsupported root parameter type
                    break;
            }
        }

        // TODO: Set root constants.
    }
}
