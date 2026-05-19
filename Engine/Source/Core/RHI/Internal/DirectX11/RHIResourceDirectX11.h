#pragma once
#include <d3d11.h>
#include "DirectXHelper.h"
#include "RHIResource.h"

#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
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

        ID3D11Buffer* GetBuffer() const { return m_pBuffer.Get(); }

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
                ThrowIfFailed("Failed to map default buffer");
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
                ThrowIfFailed("Failed to unmap default buffer");
#endif
        }

    private:
        ComPtr<ID3D11Buffer> m_pBuffer;
        ID3D11DeviceContext* m_DeviceContext;
    };

    class CommandListDirectX11 : public RHICommandList
    {
    public:
        CommandListDirectX11(RHICmdListType InType, ID3D11DeviceContext* pContext)
            : RHICommandList(InType), m_pDeviceContext(pContext) {}
        ~CommandListDirectX11() override = default;

        /* DX 11 不支持命令列表记录 */ 
        void BeginRecording() override {}
        void EndRecording() override {}

        // 输入装配器
        void IASetPrimitiveTopology(RHIPrimitiveTopology topology, uint32_t controlPointCount = 1) override;
        void IASetVertexBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers, const uint64_t* pOffsets = nullptr) override;
        void IASetIndexBuffer(RHIBuffer* pIndexBuffer, RHIIndexFormat format, uint64_t offset = 0) override;

        // 着色器
        void VSSetShader(RHIVertexShader* pShader) override;
        void PSSetShader(RHIPixelShader* pShader) override;
        void GSSetShader(RHIGeometryShader* pShader) override;
        void HSSetShader(RHIHullShader* pShader) override;
        void DSSetShader(RHIDomainShader* pShader) override;
        void CSSetShader(RHIComputeShader* pShader) override;

        // 着色器资源
        void VSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) override;
        void PSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) override;
        void GSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) override;
        void HSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) override;
        void DSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) override;
        void CSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) override;

        void VSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) override;
        void PSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) override;
        void GSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) override;
        void HSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) override;
        void DSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) override;
        void CSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) override;

        void VSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) override;
        void PSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) override;
        void GSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) override;
        void HSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) override;
        void DSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) override;
        void CSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) override;

        // 光栅器
        void RSSetViewports(uint32_t numViewports, const RHIViewport* pViewports) override;
        void RSSetScissorRects(uint32_t numRects, const RHIRect* pRects) override;
        void RSSetState(RHIRasterizerState* pState) override;

        // 输出合并器
        void OMSetRenderTargets(uint32_t numRenderTargets, RHIRenderTargetView* const* ppViews, RHIDepthStencilView* pDepthStencilView = nullptr) override;
        void OMSetBlendState(RHIBlendState* pState, const float* blendFactor = nullptr, uint32_t sampleMask = 0xFFFFFFFF) override;
        void OMSetDepthStencilState(RHIDepthStencilState* pState, uint32_t stencilRef = 0) override;

        // 绘制
        void Draw(uint32_t vertexCount, uint32_t startVertexLocation) override;
        void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation) override;
        void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) override;
        void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) override;
        void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;

        // 清除
        void ClearRenderTargetView(RHIRenderTargetView* pView, const float* colorRGBA) override;
        void ClearDepthStencilView(RHIDepthStencilView* pView, RHIClearFlags clearFlags, float depth, uint8_t stencil) override;

        // 资源操作
        void CopyResource(RHIResource* pDstResource, RHIResource* pSrcResource) override;
        void CopyBufferRegion(RHIBuffer* pDstBuffer, uint64_t dstOffset, RHIBuffer* pSrcBuffer, uint64_t srcOffset, uint64_t numBytes) override;

    private:
        ID3D11DeviceContext* m_pDeviceContext;
    };

    class CommandQueueDirectX11 : public RHICommandQueue
    {
    public:
        CommandQueueDirectX11(RHICmdListType InType, ID3D11DeviceContext* pContext)
            : RHICommandQueue(InType)
            , m_pDeviceContext(pContext) {}
        ~CommandQueueDirectX11() override = default;

        void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) override {}
        void WaitForIdle() override {}

        // 同步操作 但是 DX 11 不支持
        uint64_t Signal() override { return 0; }
        bool GetTimestampFrequency(uint64_t* frequency) override { return false; }
        bool SetEventOnCompletion(uint64_t fenceValue, void* hEvent) override { return false; }
        uint64_t GetCompletedValue() const override { return 0; }
    private:
        ID3D11DeviceContext* m_pDeviceContext;
    };
}
