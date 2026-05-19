#pragma once
#include <d3d12.h>
#include "DirectXHelper.h"
#include "RHIResource.h"
#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    class SamplerStateDirectX12 : public RHISamplerState
    {
    public:
        SamplerStateDirectX12(D3D12_SAMPLER_DESC desc, RHIDescriptorHandle handle) 
            : m_SamplerDesc(desc), RHISamplerState(handle) {} // 初始化 Handle

        ~SamplerStateDirectX12() override = default;

        const D3D12_SAMPLER_DESC& GetSamplerDesc() const { return m_SamplerDesc; }

    private:
        D3D12_SAMPLER_DESC m_SamplerDesc;
    };

    class BufferDirectX12 : public RHIBuffer
    {
    public:
        BufferDirectX12(ID3D12Resource* pResource, const BufferDesc& InDesc, ID3D12Device* InDevice)
            : RHIBuffer(InDesc, RRT_Buffer)
            , m_pResource(pResource)
            , m_Device(InDevice)
        {
        }

        ~BufferDirectX12() override = default;

        ID3D12Resource* GetResource() const { return m_pResource.Get(); }

        void* Map() override
        {
            if (GetHeapType() != BufferHeapType::Default){
                void* pData = nullptr;
                D3D12_RANGE readRange(0, 0);
                HRESULT hr = m_pResource->Map(0, &readRange, &pData);
                if (FAILED(hr))
                {
                    return nullptr;
                }
                return pData;
            }
#if RHI_ENABLE_RESOURCE_INFO
            ThrowIfFailed("Failed to unmap default buffer");
#endif
            return nullptr;
        }

        void Unmap() override
        {
            if (GetHeapType() != BufferHeapType::Default)
            {
                D3D12_RANGE writeRange(0, 0);
                m_pResource->Unmap(0, &writeRange);
            }
#if RHI_ENABLE_RESOURCE_INFO
            ThrowIfFailed("Failed to unmap default buffer");
#endif
        }

    private:
        ComPtr<ID3D12Resource> m_pResource;
        ID3D12Device* m_Device;
    };

    // CommandList
    class CommandListDirectX12 : public RHICommandList
    {
    public:
        CommandListDirectX12(RHICmdListType InType, ID3D12GraphicsCommandList* pCmdList, ID3D12CommandAllocator* pAllocator)
            : RHICommandList(InType)
            , m_pCommandList(pCmdList)
            , m_pCommandAllocator(pAllocator) {}
        ~CommandListDirectX12() override = default;

        void BeginRecording() override;
        void EndRecording() override;

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

        ID3D12GraphicsCommandList* GetCommandList() const { return m_pCommandList.Get(); }

    private:
        ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
        ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
    };

    using GraphicsCommandListDirectX12 = CommandListDirectX12;
    using ComputeCommandListDirectX12 = CommandListDirectX12;
    using CopyCommandListDirectX12 = CommandListDirectX12;

    // CommandQueue
    class CommandQueueDirectX12 : public RHICommandQueue
    {
    public:
        CommandQueueDirectX12(RHICmdListType InType, ID3D12CommandQueue* pQueue, ID3D12Device* InDevice)
            : RHICommandQueue(InType)
            , m_pCommandQueue(pQueue)
            , m_Device(InDevice)
            , m_FenceValue(0) {
                ThrowIfFailed(GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
            }
        ~CommandQueueDirectX12() override = default;

        void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) override;
        void WaitForIdle() override;

        // 同步操作
        uint64_t Signal() override;
        bool GetTimestampFrequency(uint64_t* frequency) override;
        bool SetEventOnCompletion(uint64_t fenceValue, void* hEvent) override;
        uint64_t GetCompletedValue() const override;

        ID3D12CommandQueue* GetCommandQueue() const { return m_pCommandQueue.Get(); }
        ID3D12Device* GetDevice() const { return m_Device; }

    private:
        ComPtr<ID3D12CommandQueue> m_pCommandQueue;
        ComPtr<ID3D12Fence> m_Fence;
        ID3D12Device* m_Device;
        uint64_t m_FenceValue;
    };

    using GraphicsCommandQueueDirectX12 = CommandQueueDirectX12;
    using ComputeCommandQueueDirectX12 = CommandQueueDirectX12;
    using CopyCommandQueueDirectX12 = CommandQueueDirectX12;
}
