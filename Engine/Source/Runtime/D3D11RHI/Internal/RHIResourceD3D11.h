#pragma once
#include <d3d11.h>
#include <Common/Check.h>
#include <RHIResource.h>

#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    class RasterizerStateD3D11 : public RHIRasterizerState
    {
    public:
        RasterizerStateD3D11(ID3D11RasterizerState* pRasterizerState)
            : m_pRasterizerState(pRasterizerState) {}
        ~RasterizerStateD3D11() override = default;

        ID3D11RasterizerState* GetRasterizerState() const { return m_pRasterizerState.Get(); }

    private:
        ComPtr<ID3D11RasterizerState> m_pRasterizerState;
    };

    class BlendStateD3D11 : public RHIBlendState
    {
    public:
        BlendStateD3D11(ID3D11BlendState* pBlendState)
            : m_pBlendState(pBlendState) {}
        ~BlendStateD3D11() override = default;

        ID3D11BlendState* GetBlendState() const { return m_pBlendState.Get(); }

    private:
        ComPtr<ID3D11BlendState> m_pBlendState;
    };

    class DepthStencilStateD3D11 : public RHIDepthStencilState
    {
    public:
        DepthStencilStateD3D11(ID3D11DepthStencilState* pDepthStencilState)
            : m_pDepthStencilState(pDepthStencilState) {}
        ~DepthStencilStateD3D11() override = default;

        ID3D11DepthStencilState* GetDepthStencilState() const { return m_pDepthStencilState.Get(); }

    private:
        ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
    };

    class BufferD3D11 : public RHIBuffer
    {
    public:
        BufferD3D11(ID3D11Buffer* pBuffer, const BufferDesc& InDesc, ID3D11DeviceContext* InDeviceContext)
            : RHIBuffer(InDesc, RRT_Buffer)
            , m_pBuffer(pBuffer)
            , m_DeviceContext(InDeviceContext)
        {
        }

        ~BufferD3D11() override = default;

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

    class TextureD3D11 : public RHITexture
    {
    public:
        TextureD3D11(ID3D11Texture2D* pTexture, const TextureDesc& desc)
            : m_pTexture(pTexture)
            , RHITexture(desc) {}
        ~TextureD3D11() override = default;

        uint64_t GetSize() const override;
        uint32_t GetWidth() const override;
        uint32_t GetHeight() const override;

        ID3D11Texture2D* GetTexture() const { return m_pTexture.Get(); }

    private:
        void GetDesc(D3D11_TEXTURE2D_DESC& desc) const;

        ComPtr<ID3D11Texture2D> m_pTexture;
    };

    class ConstantBufferViewD3D11 : public RHIConstantBufferView
    {
    public:
        ConstantBufferViewD3D11(ID3D11Buffer* pCBV)
            : m_pCBV(pCBV) {}
        ~ConstantBufferViewD3D11() override = default;

        ID3D11Buffer* GetCBV() const { return m_pCBV.Get(); }
        uint64_t GetGPUVirtualAddress() const override { return (uint64_t)m_pCBV.Get(); }

    private:
        ComPtr<ID3D11Buffer> m_pCBV;
    };

    class ShaderResourceViewD3D11 : public RHIShaderResourceView
    {
    public:
        ShaderResourceViewD3D11(ID3D11ShaderResourceView* pSRV)
            : m_pSRV(pSRV) {}
        ~ShaderResourceViewD3D11() override = default;

        ID3D11ShaderResourceView* GetSRV() const { return m_pSRV.Get(); }
        uint64_t GetGPUVirtualAddress() const override { return (uint64_t)m_pSRV.Get(); }

    private:
        ComPtr<ID3D11ShaderResourceView> m_pSRV;
    };

    class UnorderedAccessViewD3D11 : public RHIUnorderedAccessView
    {
    public:
        UnorderedAccessViewD3D11(ID3D11UnorderedAccessView* pUAV)
            : m_pUAV(pUAV) {}
        ~UnorderedAccessViewD3D11() override = default;

        ID3D11UnorderedAccessView* GetUAV() const { return m_pUAV.Get(); }
        uint64_t GetGPUVirtualAddress() const override { return (uint64_t)m_pUAV.Get(); }

    private:
        ComPtr<ID3D11UnorderedAccessView> m_pUAV;
    };

    class RenderTargetViewD3D11 : public RHIRenderTargetView
    {
    public:
        RenderTargetViewD3D11(ID3D11RenderTargetView* pRTV)
            : m_pRTV(pRTV) {}
        ~RenderTargetViewD3D11() override = default;

        ID3D11RenderTargetView* GetRTV() const { return m_pRTV.Get(); }

    private:
        ComPtr<ID3D11RenderTargetView> m_pRTV;
    };

    class DepthStencilViewD3D11 : public RHIDepthStencilView
    {
    public:
        DepthStencilViewD3D11(ID3D11DepthStencilView* pDSV)
            : m_pDSV(pDSV) {}
        ~DepthStencilViewD3D11() override = default;

        ID3D11DepthStencilView* GetDSV() const { return m_pDSV.Get(); }

    private:
        ComPtr<ID3D11DepthStencilView> m_pDSV;
    };

    class VertexShaderD3D11 : public RHIVertexShader
    {
    public:
        VertexShaderD3D11(ID3D11VertexShader* pShader) : m_pVertexShader(pShader) {}
        ~VertexShaderD3D11() override = default;
        ID3D11VertexShader* GetShader() const { return m_pVertexShader.Get(); }
        ID3DBlob* GetVSBlob() const { return m_pVSBlob.Get(); }

        void SetVSBlob(ID3DBlob* pVSBlob) { m_pVSBlob = pVSBlob; }
    private:
        ComPtr<ID3D11VertexShader> m_pVertexShader;
        ComPtr<ID3DBlob> m_pVSBlob;
    };

    class PixelShaderD3D11 : public RHIPixelShader
    {
    public:
        PixelShaderD3D11(ID3D11PixelShader* pShader) : m_pPixelShader(pShader) {}
        ~PixelShaderD3D11() override = default;
        ID3D11PixelShader* GetShader() const { return m_pPixelShader.Get(); }
    private:
        ComPtr<ID3D11PixelShader> m_pPixelShader;
    };

    class GeometryShaderD3D11 : public RHIGeometryShader
    {
    public:
        GeometryShaderD3D11(ID3D11GeometryShader* pShader) : m_pGeometryShader(pShader) {}
        ~GeometryShaderD3D11() override = default;
        ID3D11GeometryShader* GetShader() const { return m_pGeometryShader.Get(); }
    private:
        ComPtr<ID3D11GeometryShader> m_pGeometryShader;
    };

    class HullShaderD3D11 : public RHIHullShader
    {
    public:
        HullShaderD3D11(ID3D11HullShader* pShader) : m_pHullShader(pShader) {}
        ~HullShaderD3D11() override = default;
        ID3D11HullShader* GetShader() const { return m_pHullShader.Get(); }
    private:
        ComPtr<ID3D11HullShader> m_pHullShader;
    };

    class DomainShaderD3D11 : public RHIDomainShader
    {
    public:
        DomainShaderD3D11(ID3D11DomainShader* pShader) : m_pDomainShader(pShader) {}
        ~DomainShaderD3D11() override = default;
        ID3D11DomainShader* GetShader() const { return m_pDomainShader.Get(); }
    private:
        ComPtr<ID3D11DomainShader> m_pDomainShader;
    };

    class ComputeShaderD3D11 : public RHIComputeShader
    {
    public:
        ComputeShaderD3D11(ID3D11ComputeShader* pShader) : m_pComputeShader(pShader) {}
        ~ComputeShaderD3D11() override = default;
        ID3D11ComputeShader* GetShader() const { return m_pComputeShader.Get(); }
    private:
        ComPtr<ID3D11ComputeShader> m_pComputeShader;
    };

    class SamplerStateD3D11 : public RHISamplerState
    {
    public:
        SamplerStateD3D11(ID3D11SamplerState* pSamplerState) : m_pSamplerState(pSamplerState),
         RHISamplerState({RHIDescriptorHeapType::Sampler, 0}) {}
        ~SamplerStateD3D11() override = default;

        ID3D11SamplerState* GetSamplerState() const { return m_pSamplerState.Get(); }

    private:
        ComPtr<ID3D11SamplerState> m_pSamplerState;
    };
}
