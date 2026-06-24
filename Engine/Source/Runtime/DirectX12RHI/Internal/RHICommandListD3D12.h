#pragma once

#include "d3dx12.h"
#include <d3d12.h>

#include <Common/Check.h>
#include <Common/RHIConfig.h> // RHI configuration

#include <RHICommandList.h>
#include "RHIResourceD3D12.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace RHI
{
    class CommandAllocatorD3D12 : public RHICommandAllocator
    {
        public:
            CommandAllocatorD3D12(RHICmdType type, ID3D12CommandAllocator* pCmdAllocator)
                : RHICommandAllocator(type)
                , m_pCommandAllocator(pCmdAllocator) {}
            virtual ~CommandAllocatorD3D12() = default;

            ID3D12CommandAllocator* GetCommandAllocator() const { return m_pCommandAllocator.Get(); }

        private:
            ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
    };

    class CommandListD3D12 : public RHICommandList
    {
    public:
        CommandListD3D12(RHICommandAllocator* pCmdAllocator, ID3D12GraphicsCommandList* pCmdList)
            : RHICommandList(pCmdAllocator)
            , m_pCommandList(pCmdList) {}
        ~CommandListD3D12() override = default;

        // The start and end of the recording
        void BeginRecording() override;
        void EndRecording() override;

        // Input assembler stage
        void IASetPrimitiveTopology(RHIPrimitiveTopology topology, uint32_t controlPointCount = 1) override;
        void IASetVertexBuffers(uint32_t startSlot, uint32_t numBuffers, RHIVertexBuffer* const* ppBuffers, const uint64_t* pOffsets = nullptr) override;
        void IASetIndexBuffer(RHIIndexBuffer* pIndexBuffer, RHIIndexFormat format, uint64_t offset = 0) override;

        // Rasterizer stage
        void RSSetViewports(uint32_t numViewports, const RHIViewport* pViewports) override;
        void RSSetScissorRects(uint32_t numRects, const RHIRect* pRects) override;
        // Output merger stage
        void OMSetRenderTargets(uint32_t numRenderTargets, RHIRenderTargetView* const* ppViews, bool RTsSingleHandleToDescriptorRange, RHIDepthStencilView* pDepthStencilView = nullptr) override;
        void OMSetBlendState(RHIBlendState* pState, const float* blendFactor = nullptr, uint32_t sampleMask = 0xFFFFFFFF) override;
        void OMSetDepthStencilState(RHIDepthStencilState* pState, uint32_t stencilRef = 0) override;

        // Draw
        void Draw(uint32_t vertexCount, uint32_t startVertexLocation) override;
        void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation) override;
        void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) override;
        void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) override;
        void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;

        // Clear
        void ClearRenderTargetView(RHIRenderTargetView* pView, const float* colorRGBA) override;
        void ClearDepthStencilView(RHIDepthStencilView* pView, RHIClearFlags clearFlags, float depth, uint8_t stencil) override;

        // Resource operation
        void CopyResource(RHIResource* pDstResource, RHIResource* pSrcResource) override;
        void CopyBufferRegion(RHIBuffer* pDstBuffer, uint64_t dstOffset, RHIBuffer* pSrcBuffer, uint64_t srcOffset, uint64_t numBytes) override;

        // Resource barrier
        void ResourceBarrier(uint32_t numBarriers, const BarrierDesc* pBarriers) override;

        // Root signature setting
        void SetGraphicsRootSignature(RHIRootSignature* pRootSignature) override;
        void SetComputeRootSignature(RHIRootSignature* pRootSignature) override;
        void SetDescriptorHeaps(uint32_t numHeaps, RHIDescriptorHeap* const* ppHeaps) override;

        // Pipeline state setting
        void SetPipelineState(RHIPipelineState* pPipelineState, PipelineStateType stateType) override;

        // Graphics pipeline binding
        void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, RHIDescriptorHeap* pDescriptorHeap, uint32_t offsetInDescriptorsFromTableStart) override;
        void SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetGraphicsRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues) override;
        void SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues) override;

        // Compute pipeline binding
        void SetComputeRootDescriptorTable(uint32_t rootParameterIndex, RHIDescriptorHeap* pDescriptorHeap, uint32_t offsetInDescriptorsFromTableStart) override;
        void SetComputeRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetComputeRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetComputeRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues) override;
        void SetComputeRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues) override;

        ID3D12GraphicsCommandList* GetCommandList() const { return m_pCommandList.Get(); }

    private:
        // Here, use this function to quickly get the allocator
        CommandAllocatorD3D12* GetAllocator() { 
            if(m_pAllocator == nullptr){
#ifdef RHI_ENABLE_RESOURCE_DEBUG_INFO
                ThrowErrorMessage("CommandAllocatorD3D12 is nullptr");
#endif
                return nullptr;
            }
            return SafeCast<CommandAllocatorD3D12>(m_pAllocator);
        }

        ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
    };

    using GraphicsCommandListD3D12 = CommandListD3D12;
    using ComputeCommandListD3D12 = CommandListD3D12;
    using CopyCommandListD3D12 = CommandListD3D12;

    class CommandQueueD3D12 : public RHICommandQueue
    {
    public:
        CommandQueueD3D12(RHICmdType InType, ID3D12CommandQueue* pQueue, ID3D12Device* InDevice)
            : RHICommandQueue(InType)
            , m_pCommandQueue(pQueue)
            , m_Device(InDevice) {
                ThrowIfFailed(GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
                m_fenceEvent = CreateEvent(nullptr,FALSE,FALSE,nullptr);
                if (!m_fenceEvent) {
                    ThrowErrorMessage("Failed to create fence event");
                }
            }
        ~CommandQueueD3D12() override {
            CloseHandle(m_fenceEvent);
        };

        void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) override;
        void BeginFrame() override;
        void EndFrame() override;
        void WaitForGPU() override;

        // Synchronization operation
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
        UINT64 m_fenceValues[RHI_MULTI_BUFFERING] = {0}; // Multi-buffering fence values
        UINT64 m_nextFenceValue = 1;
        UINT m_currentFrame = 0;  

        ID3D12Device* m_Device;
    };

    using GraphicsCommandQueueD3D12 = CommandQueueD3D12;
    using ComputeCommandQueueD3D12 = CommandQueueD3D12;
    using CopyCommandQueueD3D12 = CommandQueueD3D12;
}
