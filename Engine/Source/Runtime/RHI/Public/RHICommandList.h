#pragma once

#include "Common/RHI_API.h"
#include "RHIResource.h"
#include "Common/RHIDesc.h"
#include "RHIRootSignature.h" // RootSignatureDesc
#include "RHIPipelineState.h" // GraphicsPipelineStateDesc, ComputePipelineStateDesc

#include <vector>
#include <memory>

namespace RHI
{
    class RHI_API RHICommandAllocator
    {
        public:
            RHICommandAllocator(RHICmdType InType);
            virtual ~RHICommandAllocator();

            RHICmdType GetCmdType() const { return CmdType; }
        private:
            RHICmdType CmdType;
    };

    class RHI_API RHICommandList
    {
    public:
        RHICommandList(RHICommandAllocator* pCmdAllocator);
        virtual ~RHICommandList();

        virtual void BeginRecording() = 0;
        virtual void EndRecording() = 0;

        // 输入装配器
        virtual void IASetPrimitiveTopology(RHIPrimitiveTopology topology, uint32_t controlPointCount = 1) = 0;
        virtual void IASetVertexBuffers(uint32_t startSlot, uint32_t numBuffers, RHIVertexBuffer* const* ppBuffers, const uint64_t* pOffsets = nullptr) = 0;
        virtual void IASetIndexBuffer(RHIIndexBuffer* pIndexBuffer, RHIIndexFormat format, uint64_t offset = 0) = 0;

        // 光栅器
        virtual void RSSetViewports(uint32_t numViewports, const RHIViewport* pViewports) = 0;
        virtual void RSSetScissorRects(uint32_t numRects, const RHIRect* pRects) = 0;

        // 输出合并器
        virtual void OMSetRenderTargets(uint32_t numRenderTargets, class RHIRenderTargetView* const* ppViews, class RHIDepthStencilView* pDepthStencilView = nullptr) = 0;
        virtual void OMSetBlendState(class RHIBlendState* pState, const float* blendFactor = nullptr, uint32_t sampleMask = 0xFFFFFFFF) = 0;
        virtual void OMSetDepthStencilState(class RHIDepthStencilState* pState, uint32_t stencilRef = 0) = 0;

        // 绘制
        virtual void Draw(uint32_t vertexCount, uint32_t startVertexLocation) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation) = 0;
        virtual void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) = 0;
        virtual void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) = 0;
        virtual void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) = 0;

        // 清除
        virtual void ClearRenderTargetView(class RHIRenderTargetView* pView, const float* colorRGBA) = 0;
        virtual void ClearDepthStencilView(class RHIDepthStencilView* pView, RHIClearFlags clearFlags, float depth, uint8_t stencil) = 0;

        // 资源操作
        virtual void CopyResource(RHIResource* pDstResource, RHIResource* pSrcResource) = 0;
        virtual void CopyBufferRegion(RHIBuffer* pDstBuffer, uint64_t dstOffset, RHIBuffer* pSrcBuffer, uint64_t srcOffset, uint64_t numBytes) = 0;

        // 屏障
        virtual void ResourceBarrier(uint32_t numBarriers, const BarrierDesc* pBarriers) = 0;

        // 根签名设置
        virtual void SetGraphicsRootSignature(RHIRootSignature* pRootSignature) = 0;
        virtual void SetComputeRootSignature(RHIRootSignature* pRootSignature) = 0;
        virtual void SetDescriptorHeaps(uint32_t numHeaps, RHIDescriptorHeap* const* ppHeaps) = 0;

        // PSO设置
        virtual void SetPipelineState(RHIPipelineState* pPipelineState, PipelineStateType stateType) = 0;

        // 图形管线绑定
        virtual void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, RHIDescriptorHeap* pDescriptorHeap, uint32_t offsetInDescriptorsFromTableStart) = 0;
        virtual void SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) = 0;
        virtual void SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) = 0;
        virtual void SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) = 0;
        virtual void SetGraphicsRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues) = 0;
        virtual void SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues) = 0;

        // 计算管线绑定
        virtual void SetComputeRootDescriptorTable(uint32_t rootParameterIndex, RHIDescriptorHeap* pDescriptorHeap, uint32_t offsetInDescriptorsFromTableStart) = 0;
        virtual void SetComputeRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) = 0;
        virtual void SetComputeRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) = 0;
        virtual void SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) = 0;
        virtual void SetComputeRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues) = 0;
        virtual void SetComputeRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues) = 0;

    protected:
        RHICommandAllocator* m_pAllocator; // 命令分配器 占时引用
    };

    class RHI_API RHICommandQueue
    {
    public:
        RHICommandQueue(RHICmdType InType);
        virtual ~RHICommandQueue();

        RHICmdType GetQueueType() const { return QueueType; }

        virtual void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) = 0;
        virtual void WaitForGPU() = 0;

        // 同步操作
        virtual void Signal(uint64_t fenceValue) = 0;
        virtual bool GetTimestampFrequency(uint64_t* frequency) = 0;
        virtual bool SetEventOnCompletion(uint64_t fenceValue, void* hEvent) = 0;
        virtual uint64_t GetFrameIndex() const = 0;

    protected:
        RHICmdType QueueType;
    };
} // namespace RHI
