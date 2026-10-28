#pragma once

#include "Common/RHI_API.h"
#include "RHIResource.h"
#include "Common/RHIDefinitions.h"
#include "RHIRootSignature.h" // RootSignatureDesc
#include "RHIPipelineState.h" // GraphicsPipelineStateDesc, ComputePipelineStateDesc

#include <vector>
#include <memory>
#include <atomic>

namespace RHI
{
    class RHI_API RHICommandPool 
    {
        public:
            /**
             * @brief
             * @param InType
             */
            RHICommandPool (RHICmdType InType);
            /**
             * @brief
             */
            virtual ~RHICommandPool();

            /**
             * @brief
             * @return
             */
            RHICmdType GetCmdType() const { return CmdType; }
            /**
             * @brief reset command pool memory
             */
            virtual void Reset() const = 0;
            /**
             * @brief get is recording
             * @return
             */
            bool IsRecording() const { return m_IsRecording.load(std::memory_order_relaxed); }
            /**
             * @brief set is recording
             * @param isRecording
             */
            void SetIsRecording(bool isRecording) { m_IsRecording.store(isRecording, std::memory_order_relaxed); }
        private:
            RHICmdType CmdType;
            // This is mainly to prevent users from recording with multiple threads.
            std::atomic<bool> m_IsRecording = false;
    };

    // By default, we use non-constant pointers, but that doesn't mean the actual subclass being pointed to is constant.
    using RHICommandPoolPtr = RHICommandPool*;

    class RHI_API RHICommandList
    {
    public:
        /**
         * @brief
         * @param pCommandPool
         */
        RHICommandList(RHICommandPoolPtr pCommandPool);
        /**
         * @brief
         */
        virtual ~RHICommandList();

        /**
         * @brief
         */
        virtual void BeginRecording() = 0;
        /**
         * @brief
         */
        virtual void EndRecording() = 0;

        // Input assembler
        /**
         * @brief
         * @param topology
         * @param controlPointCount
         */
        virtual void IASetPrimitiveTopology(RHIPrimitiveTopology topology, uint32_t controlPointCount = 1) = 0;
        /**
         * @brief
         * @param startSlot
         * @param numBuffers
         * @param ppBuffers
         * @param pOffsets
         */
        virtual void IASetVertexBuffers(uint32_t startSlot, uint32_t numBuffers, RHIVertexBuffer* const* ppBuffers, const uint64_t* pOffsets = nullptr) = 0;
        /**
         * @brief
         * @param pIndexBuffer
         * @param format
         * @param offset
         */
        virtual void IASetIndexBuffer(RHIIndexBuffer* pIndexBuffer, RHIIndexFormat format, uint64_t offset = 0) = 0;

        // Rasterizer
        /**
         * @brief
         * @param numViewports
         * @param pViewports
         */
        virtual void RSSetViewports(uint32_t numViewports, const RHIViewport* pViewports) = 0;
        /**
         * @brief
         * @param numRects
         * @param pRects
         */
        virtual void RSSetScissorRects(uint32_t numRects, const RHIRect* pRects) = 0;
        
        // Output merger state
        /**
         * @brief
         * @param numRenderTargets
         * @param ppViews
         * @param RTsSingleHandleToDescriptorRange
         * @param pDepthStencilView
         */
        virtual void OMSetRenderTargets(uint32_t numRenderTargets, class RHIRenderTargetView* const* ppViews, bool RTsSingleHandleToDescriptorRange, class RHIDepthStencilView* pDepthStencilView = nullptr) = 0;
        /**
         * @brief
         * @param pState
         * @param blendFactor
         * @param sampleMask
         */
        virtual void OMSetBlendState(class RHIBlendState* pState, const float* blendFactor = nullptr, uint32_t sampleMask = 0xFFFFFFFF) = 0;
        /**
         * @brief
         * @param pState
         * @param stencilRef
         */
        virtual void OMSetDepthStencilState(class RHIDepthStencilState* pState, uint32_t stencilRef = 0) = 0;

        // Draw
        /**
         * @brief
         * @param vertexCount
         * @param startVertexLocation
         */
        virtual void Draw(uint32_t vertexCount, uint32_t startVertexLocation) = 0;
        /**
         * @brief
         * @param indexCount
         * @param startIndexLocation
         * @param baseVertexLocation
         */
        virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation) = 0;
        /**
         * @brief
         * @param vertexCountPerInstance
         * @param instanceCount
         * @param startVertexLocation
         * @param startInstanceLocation
         */
        virtual void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) = 0;
        /**
         * @brief
         * @param indexCountPerInstance
         * @param instanceCount
         * @param startIndexLocation
         * @param baseVertexLocation
         * @param startInstanceLocation
         */
        virtual void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) = 0;
        /**
         * @brief
         * @param threadGroupCountX
         * @param threadGroupCountY
         * @param threadGroupCountZ
         */
        virtual void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) = 0;

        // Clear
        /**
         * @brief
         * @param pView
         * @param colorRGBA
         */
        virtual void ClearRenderTargetView(class RHIRenderTargetView* pView, const float* colorRGBA) = 0;
        /**
         * @brief
         * @param pView
         * @param clearFlags
         * @param depth
         * @param stencil
         */
        virtual void ClearDepthStencilView(class RHIDepthStencilView* pView, RHIClearFlags clearFlags, float depth, uint8_t stencil) = 0;

        // Resource operation
        /**
         * @brief
         * @param pDstResource
         * @param pSrcResource
         */
        virtual void CopyResource(RHIResource* pDstResource, RHIResource* pSrcResource) = 0;
        /**
         * @brief
         * @param pDstBuffer
         * @param dstOffset
         * @param pSrcBuffer
         * @param srcOffset
         * @param numBytes
         */
        virtual void CopyBufferRegion(RHIBuffer* pDstBuffer, uint64_t dstOffset, RHIBuffer* pSrcBuffer, uint64_t srcOffset, uint64_t numBytes) = 0;

        // Resource barrier
        /**
         * @brief
         * @param numBarriers
         * @param pBarriers
         */
        virtual void ResourceBarrier(uint32_t numBarriers, const BarrierDesc* pBarriers) = 0;

        // Root signature set
        /**
         * @brief
         * @param pRootSignature
         */
        virtual void SetGraphicsRootSignature(RHIRootSignature* pRootSignature) = 0;
        /**
         * @brief
         * @param pRootSignature
         */
        virtual void SetComputeRootSignature(RHIRootSignature* pRootSignature) = 0;
        /**
         * @brief
         * @param numHeaps
         * @param ppHeaps
         */
        virtual void SetDescriptorHeaps(uint32_t numHeaps, RHIDescriptorHeap* const* ppHeaps) = 0;

        // Pipeline state set
        /**
         * @brief
         * @param pPipelineState
         * @param stateType
         */
        virtual void SetPipelineState(RHIPipelineState* pPipelineState, PipelineStateType stateType) = 0;

        // Graphics pipeline binding
        /**
         * @brief
         * @param rootParameterIndex
         * @param pDescriptorHeap
         * @param offsetInDescriptorsFromTableStart
         */
        virtual void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, RHIDescriptorHeap* pDescriptorHeap, uint32_t offsetInDescriptorsFromTableStart) = 0;
        /**
         * @brief
         * @param rootParameterIndex
         * @param gpuVirtualAddress
         */
        virtual void SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) = 0;
        /**
         * @brief
         * @param rootParameterIndex
         * @param gpuVirtualAddress
         */
        virtual void SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) = 0;
        /**
         * @brief
         * @param rootParameterIndex
         * @param gpuVirtualAddress
         */
        virtual void SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) = 0;
        /**
         * @brief The last parameter is to set the offset of your data in the target buffer
         * @param rootParameterIndex
         * @param value
         * @param destOffsetIn32BitValues
         */
        virtual void SetGraphicsRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues) = 0;
        /**
         * @brief
         * @param rootParameterIndex
         * @param num32BitValues
         * @param pSrcData
         * @param destOffsetIn32BitValues
         */
        virtual void SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues) = 0;

        // Compute pipeline binding
        /**
         * @brief
         * @param rootParameterIndex
         * @param pDescriptorHeap
         * @param offsetInDescriptorsFromTableStart
         */
        virtual void SetComputeRootDescriptorTable(uint32_t rootParameterIndex, RHIDescriptorHeap* pDescriptorHeap, uint32_t offsetInDescriptorsFromTableStart) = 0;
        /**
         * @brief
         * @param rootParameterIndex
         * @param gpuVirtualAddress
         */
        virtual void SetComputeRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) = 0;
        /**
         * @brief
         * @param rootParameterIndex
         * @param gpuVirtualAddress
         */
        virtual void SetComputeRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) = 0;
        /**
         * @brief
         * @param rootParameterIndex
         * @param gpuVirtualAddress
         */
        virtual void SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) = 0;
        /**
         * @brief The last parameter is to set the offset of your data in the target buffer
         * @param rootParameterIndex
         * @param value
         * @param destOffsetIn32BitValues
         */
        virtual void SetComputeRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues) = 0;
        /**
         * @brief
         * @param rootParameterIndex
         * @param num32BitValues
         * @param pSrcData
         * @param destOffsetIn32BitValues
         */
        virtual void SetComputeRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues) = 0;

    protected:
        // Command pool pointer
        RHICommandPoolPtr m_pCommandPool;
    };

    // Basic queue
    class RHI_API RHICommandQueue
    {
    public:
        /**
         * @brief
         * @param InType
         */
        RHICommandQueue(RHICmdType InType);
        /**
         * @brief
         */
        virtual ~RHICommandQueue();

        /**
         * @brief
         * @return
         */
        RHICmdType GetQueueType() const { return QueueType; }

        /**
         * @brief All queues must be able to execute commands
         * @param cmdLists
         */
        virtual void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) = 0;
        /**
         * @brief All queues must be able to block-wait (for exit or resource synchronization)
         */
        virtual void WaitForGPU() = 0;
        /**
         * @brief All queues can query timestamps (for performance analysis)
         * @param frequency
         * @return
         */
        virtual bool GetTimestampFrequency(uint64_t* frequency) = 0;

    protected:
        RHICmdType QueueType;
    };

    // Frame queue
    class RHI_API RHIFrameCommandQueue : public RHICommandQueue
    {
    public:
        /**
         * @brief
         * @param InType
         */
        RHIFrameCommandQueue(RHICmdType InType);
        /**
         * @brief
         */
        virtual ~RHIFrameCommandQueue();

        /**
         * @brief
         */
        virtual void BeginFrame() = 0;
        /**
         * @brief
         */
        virtual void EndFrame() = 0;
        /**
         * @brief
         */
        virtual void Present() = 0;
        /**
         * @brief
         * @param Info
         */
        virtual void SetGraphicsQueueSwapChainInfo(GraphicsQueueSwapChainInfo Info) = 0;
        /**
         * @brief
         * @param cmdPool
         */
        virtual void ResetCommandPoolMemory(const RHICommandPool* cmdPool) = 0;
        /**
         * @brief
         * @return
         */
        virtual uint64_t GetFrameIndex() const = 0;
    };
} // namespace RHI
