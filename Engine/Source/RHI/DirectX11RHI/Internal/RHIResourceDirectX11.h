#pragma once
#include <d3d11.h>
#include "DirectXHelper.h"
#include "RHIResource.h"

#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    class VertexShaderDirectX11 : public RHIVertexShader
    {
    public:
        VertexShaderDirectX11(ID3D11VertexShader* pShader) : m_pVertexShader(pShader) {}
        ~VertexShaderDirectX11() override = default;
        ID3D11VertexShader* GetShader() const { return m_pVertexShader.Get(); }
    private:
        ComPtr<ID3D11VertexShader> m_pVertexShader;
    };

    class PixelShaderDirectX11 : public RHIPixelShader
    {
    public:
        PixelShaderDirectX11(ID3D11PixelShader* pShader) : m_pPixelShader(pShader) {}
        ~PixelShaderDirectX11() override = default;
        ID3D11PixelShader* GetShader() const { return m_pPixelShader.Get(); }
    private:
        ComPtr<ID3D11PixelShader> m_pPixelShader;
    };

    class GeometryShaderDirectX11 : public RHIGeometryShader
    {
    public:
        GeometryShaderDirectX11(ID3D11GeometryShader* pShader) : m_pGeometryShader(pShader) {}
        ~GeometryShaderDirectX11() override = default;
        ID3D11GeometryShader* GetShader() const { return m_pGeometryShader.Get(); }
    private:
        ComPtr<ID3D11GeometryShader> m_pGeometryShader;
    };

    class HullShaderDirectX11 : public RHIHullShader
    {
    public:
        HullShaderDirectX11(ID3D11HullShader* pShader) : m_pHullShader(pShader) {}
        ~HullShaderDirectX11() override = default;
        ID3D11HullShader* GetShader() const { return m_pHullShader.Get(); }
    private:
        ComPtr<ID3D11HullShader> m_pHullShader;
    };

    class DomainShaderDirectX11 : public RHIDomainShader
    {
    public:
        DomainShaderDirectX11(ID3D11DomainShader* pShader) : m_pDomainShader(pShader) {}
        ~DomainShaderDirectX11() override = default;
        ID3D11DomainShader* GetShader() const { return m_pDomainShader.Get(); }
    private:
        ComPtr<ID3D11DomainShader> m_pDomainShader;
    };

    class ComputeShaderDirectX11 : public RHIComputeShader
    {
    public:
        ComputeShaderDirectX11(ID3D11ComputeShader* pShader) : m_pComputeShader(pShader) {}
        ~ComputeShaderDirectX11() override = default;
        ID3D11ComputeShader* GetShader() const { return m_pComputeShader.Get(); }
    private:
        ComPtr<ID3D11ComputeShader> m_pComputeShader;
    };

    class SamplerStateDirectX11 : public RHISamplerState
    {
    public:
        SamplerStateDirectX11(ID3D11SamplerState* pSamplerState) : m_pSamplerState(pSamplerState),
         RHISamplerState({RHIDescriptorHeapType::Sampler, 0}) {}
        ~SamplerStateDirectX11() override = default;

        ID3D11SamplerState* GetSamplerState() const { return m_pSamplerState.Get(); }

    private:
        ComPtr<ID3D11SamplerState> m_pSamplerState;
    };

    class BufferDirectX11 : public RHIBuffer
    {
    public:
        BufferDirectX11(ID3D11Buffer* pBuffer, const BufferDesc& InDesc)
            : RHIBuffer(InDesc, RRT_Buffer)
            , m_pBuffer(pBuffer)
            , m_DeviceContext(nullptr)
        {
        }

        ~BufferDirectX11() override = default;

        void SetDeviceContext(ID3D11DeviceContext* InDeviceContext)
        {
            m_DeviceContext = InDeviceContext;
        }

        ID3D11Buffer* GetResource() const { return m_pBuffer.Get(); }

        void* Map() override
        {
            if (m_DeviceContext && GetHeapType() != BufferHeapType::Default)
            {
                D3D11_MAPPED_SUBRESOURCE mappedResource;
                HRESULT hr = m_DeviceContext->Map(m_pBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
                if (SUCCEEDED(hr))
                {
                    return mappedResource.pData;
                }
            }
#if RHI_ENABLE_RESOURCE_INFO
            else if(GetHeapType() == BufferHeapType::Default) 
                ThrowErrorMessage("Failed to map default buffer");
#endif
            return nullptr;
        }

        void Unmap() override
        {
            if (m_DeviceContext && GetHeapType() != BufferHeapType::Default)
            {
                m_DeviceContext->Unmap(m_pBuffer.Get(), 0);
            }
#if RHI_ENABLE_RESOURCE_INFO
            else if(GetHeapType() == BufferHeapType::Default) 
                ThrowErrorMessage("Failed to unmap default buffer");
#endif
        }

    private:
        ComPtr<ID3D11Buffer> m_pBuffer;
        ID3D11DeviceContext* m_DeviceContext;
    };
}
