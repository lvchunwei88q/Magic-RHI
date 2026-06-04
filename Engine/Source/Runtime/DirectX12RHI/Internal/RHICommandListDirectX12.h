#pragma once

#include "d3dx12.h"
#include <d3d12.h>

#include <Common/RHIException.h>
#include <Config/ConfigBase.h> // RHI配置

#include <RHICommandList.h>
#include "RHIResourceDirectX12.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace RHI
{
    class CommandAllocatorDirectX12 : public RHICommandAllocator
    {
        public:
            CommandAllocatorDirectX12(RHICmdType type, ID3D12CommandAllocator* pCmdAllocator)
                : RHICommandAllocator(type)
                , m_pCommandAllocator(pCmdAllocator) {}
            virtual ~CommandAllocatorDirectX12() = default;

            ID3D12CommandAllocator* GetCommandAllocator() const { return m_pCommandAllocator.Get(); }

        private:
            ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
    };

    class CommandListDirectX12 : public RHICommandList
    {
    public:
        CommandListDirectX12(RHICommandAllocator* pCmdAllocator, ID3D12GraphicsCommandList* pCmdList)
            : RHICommandList(pCmdAllocator)
            , m_pCommandList(pCmdList) {}
        ~CommandListDirectX12() override = default;

        void BeginRecording() override;
        void EndRecording() override;

        // 输入装配器
        void IASetPrimitiveTopology(RHIPrimitiveTopology topology, uint32_t controlPointCount = 1) override;
        void IASetVertexBuffers(uint32_t startSlot, uint32_t numBuffers, RHIVertexBuffer* const* ppBuffers, const uint64_t* pOffsets = nullptr) override;
        void IASetIndexBuffer(RHIIndexBuffer* pIndexBuffer, RHIIndexFormat format, uint64_t offset = 0) override;

        // 光栅器
        void RSSetViewports(uint32_t numViewports, const RHIViewport* pViewports) override;
        void RSSetScissorRects(uint32_t numRects, const RHIRect* pRects) override;
        // 输出合并器
        void OMSetRenderTargets(uint32_t numRenderTargets, RHIRenderTargetView* const* ppViews, bool RTsSingleHandleToDescriptorRange, RHIDepthStencilView* pDepthStencilView = nullptr) override;
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

        // 屏障
        void ResourceBarrier(uint32_t numBarriers, const BarrierDesc* pBarriers) override;

        // 根签名设置
        void SetGraphicsRootSignature(RHIRootSignature* pRootSignature) override;
        void SetComputeRootSignature(RHIRootSignature* pRootSignature) override;
        void SetDescriptorHeaps(uint32_t numHeaps, RHIDescriptorHeap* const* ppHeaps) override;

        // PSO设置
        void SetPipelineState(RHIPipelineState* pPipelineState, PipelineStateType stateType) override;

        // 图形管线绑定
        void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, RHIDescriptorHeap* pDescriptorHeap, uint32_t offsetInDescriptorsFromTableStart) override;
        void SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetGraphicsRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues) override;
        void SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues) override;

        // 计算管线绑定
        void SetComputeRootDescriptorTable(uint32_t rootParameterIndex, RHIDescriptorHeap* pDescriptorHeap, uint32_t offsetInDescriptorsFromTableStart) override;
        void SetComputeRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetComputeRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetComputeRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues) override;
        void SetComputeRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues) override;

        ID3D12GraphicsCommandList* GetCommandList() const { return m_pCommandList.Get(); }

    private:
        ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
    };

    using GraphicsCommandListDirectX12 = CommandListDirectX12;
    using ComputeCommandListDirectX12 = CommandListDirectX12;
    using CopyCommandListDirectX12 = CommandListDirectX12;

    class CommandQueueDirectX12 : public RHICommandQueue
    {
    public:
        CommandQueueDirectX12(RHICmdType InType, ID3D12CommandQueue* pQueue, ID3D12Device* InDevice)
            : RHICommandQueue(InType)
            , m_pCommandQueue(pQueue)
            , m_Device(InDevice) {
                ThrowIfFailed(GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
                m_fenceEvent = CreateEvent(nullptr,FALSE,FALSE,nullptr);
                if (!m_fenceEvent) {
                    ThrowErrorMessage("Failed to create fence event");
                }
            }
        ~CommandQueueDirectX12() override {
            CloseHandle(m_fenceEvent);
        };

        void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) override;
        void WaitForGPU() override;

        // 同步操作
        void Signal(uint64_t fenceValue) override;
        bool GetTimestampFrequency(uint64_t* frequency) override; // Get timestamp frequency 只需要获取一次
        bool SetEventOnCompletion(uint64_t fenceValue, void* hEvent) override;
        uint64_t GetFrameIndex() const override;

        ID3D12CommandQueue* GetCommandQueue() const { return m_pCommandQueue.Get(); }
        ID3D12Device* GetDevice() const { return m_Device; }

    private:
        ComPtr<ID3D12CommandQueue> m_pCommandQueue;
        
        // Synchronization objects.
        HANDLE m_fenceEvent;
        ComPtr<ID3D12Fence> m_Fence;
        UINT64 m_fenceValues[RHI_MULTI_BUFFERING] = {0}; // 多缓冲区 Num
        UINT64 m_nextFenceValue = 1;
        UINT m_currentFrame = 0;  

        ID3D12Device* m_Device;
    };

    using GraphicsCommandQueueDirectX12 = CommandQueueDirectX12;
    using ComputeCommandQueueDirectX12 = CommandQueueDirectX12;
    using CopyCommandQueueDirectX12 = CommandQueueDirectX12;
}
