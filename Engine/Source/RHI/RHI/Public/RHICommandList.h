#pragma once

#include "Common/RHI_API.h"
#include "RHIResource.h"

#include <vector>
#include <memory>

namespace RHI
{
    class RHI_API RHICommandList : public RHIResource
    {
    public:
        RHICommandList(RHICmdListType InType)
            : RHIResource(RRT_None), CmdListType(InType) {}
        virtual ~RHICommandList() = default;

        RHICmdListType GetCmdListType() const { return CmdListType; }

        virtual void BeginRecording() = 0;
        virtual void EndRecording() = 0;

        // 输入装配器
        virtual void IASetPrimitiveTopology(RHIPrimitiveTopology topology, uint32_t controlPointCount = 1) = 0;
        virtual void IASetVertexBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers, const uint64_t* pOffsets = nullptr) = 0;
        virtual void IASetIndexBuffer(RHIBuffer* pIndexBuffer, RHIIndexFormat format, uint64_t offset = 0) = 0;

        // 着色器
        virtual void VSSetShader(class RHIVertexShader* pShader) = 0;
        virtual void PSSetShader(class RHIPixelShader* pShader) = 0;
        virtual void GSSetShader(class RHIGeometryShader* pShader) = 0;
        virtual void HSSetShader(class RHIHullShader* pShader) = 0;
        virtual void DSSetShader(class RHIDomainShader* pShader) = 0;
        virtual void CSSetShader(class RHIComputeShader* pShader) = 0;

        // 着色器资源
        virtual void VSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) = 0;
        virtual void PSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) = 0;
        virtual void GSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) = 0;
        virtual void HSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) = 0;
        virtual void DSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) = 0;
        virtual void CSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) = 0;

        virtual void VSSetShaderResources(uint32_t startSlot, uint32_t numViews, class RHIShaderResourceView* const* ppViews) = 0;
        virtual void PSSetShaderResources(uint32_t startSlot, uint32_t numViews, class RHIShaderResourceView* const* ppViews) = 0;
        virtual void GSSetShaderResources(uint32_t startSlot, uint32_t numViews, class RHIShaderResourceView* const* ppViews) = 0;
        virtual void HSSetShaderResources(uint32_t startSlot, uint32_t numViews, class RHIShaderResourceView* const* ppViews) = 0;
        virtual void DSSetShaderResources(uint32_t startSlot, uint32_t numViews, class RHIShaderResourceView* const* ppViews) = 0;
        virtual void CSSetShaderResources(uint32_t startSlot, uint32_t numViews, class RHIShaderResourceView* const* ppViews) = 0;

        virtual void VSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) = 0;
        virtual void PSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) = 0;
        virtual void GSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) = 0;
        virtual void HSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) = 0;
        virtual void DSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) = 0;
        virtual void CSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) = 0;

        // 光栅器
        virtual void RSSetViewports(uint32_t numViewports, const RHIViewport* pViewports) = 0;
        virtual void RSSetScissorRects(uint32_t numRects, const RHIRect* pRects) = 0;
        virtual void RSSetState(class RHIRasterizerState* pState) = 0;

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

    protected:
        RHICmdListType CmdListType;
    };

    class RHI_API RHICommandQueue : public RHIResource
    {
    public:
        RHICommandQueue(RHICmdListType InType)
            : RHIResource(RRT_None), QueueType(InType) {}
        virtual ~RHICommandQueue() = default;

        RHICmdListType GetQueueType() const { return QueueType; }

        virtual void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) = 0;
        virtual void WaitForIdle() = 0;

        // 同步操作
        virtual uint64_t Signal() = 0;
        virtual bool GetTimestampFrequency(uint64_t* frequency) = 0;
        virtual bool SetEventOnCompletion(uint64_t fenceValue, void* hEvent) = 0;
        virtual uint64_t GetCompletedValue() const = 0;

    protected:
        RHICmdListType QueueType;
    };
} // namespace RHI
