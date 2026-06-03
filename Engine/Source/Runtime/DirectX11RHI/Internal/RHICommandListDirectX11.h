#pragma once

#include <cstdint>
#include <d3d11.h>
#include "DirectXHelper.h"
#include <RHICommandList.h>
#include "RHIResourceDirectX11.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace RHI
{
    class CommandAllocatorDirectX11 : public RHICommandAllocator
    {
        public:
            CommandAllocatorDirectX11(RHICmdType type, ID3D11DeviceContext* pContext)
                : RHICommandAllocator(type)
                , m_pDeviceContext(pContext) {}
            virtual ~CommandAllocatorDirectX11() = default;

            ID3D11DeviceContext* GetDeviceContext() const { return m_pDeviceContext; }
        private:
            ID3D11DeviceContext* m_pDeviceContext;
    };


    class CommandListDirectX11 : public RHICommandList
    {
    public:
        CommandListDirectX11(RHICommandAllocator* pCmdAllocator)
            : RHICommandList(pCmdAllocator) {}
        ~CommandListDirectX11() override = default;

        /* DX 11 不支持命令列表记录 */ 
        void BeginRecording() override {}
        void EndRecording() override {}

        // 输入装配器
        void IASetPrimitiveTopology(RHIPrimitiveTopology topology, uint32_t controlPointCount = 1) override;
        void IASetVertexBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers, const uint64_t* pOffsets = nullptr) override;
        void IASetIndexBuffer(RHIBuffer* pIndexBuffer, RHIIndexFormat format, uint64_t offset = 0) override;

        // 光栅器
        void RSSetViewports(uint32_t numViewports, const RHIViewport* pViewports) override;
        void RSSetScissorRects(uint32_t numRects, const RHIRect* pRects) override;

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

        // 屏障
        void ResourceBarrier(uint32_t numBarriers, const BarrierDesc* pBarriers) override;

        // 根签名设置
        void SetGraphicsRootSignature(RHIRootSignature* pRootSignature) override;
        void SetComputeRootSignature(RHIRootSignature* pRootSignature) override;
        void SetDescriptorHeaps(uint32_t numHeaps, RHIDescriptorHeap* const* ppHeaps) override;

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

    };

    class CommandQueueDirectX11 : public RHICommandQueue
    {
    public:
        CommandQueueDirectX11(RHICmdType InType, ID3D11DeviceContext* pContext, ID3D11Device* pDevice)
            : RHICommandQueue(InType)
            , m_pDeviceContext(pContext)
            , m_pDevice(pDevice) {
                D3D11_QUERY_DESC desc = {};
                desc.Query = D3D11_QUERY_TIMESTAMP;
                m_pDevice->CreateQuery(&desc, &m_pTimestampStart);
                m_pDevice->CreateQuery(&desc, &m_pTimestampEnd);
                desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
                m_pDevice->CreateQuery(&desc, &m_pDisjoint);
            }
        ~CommandQueueDirectX11() override = default;

        void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) override;
        void WaitForGPU() override {}

        // 同步操作 但是 DX 11 不支持
        void Signal(uint64_t fenceValue) override {}
        bool GetTimestampFrequency(uint64_t* frequency) override;
        bool SetEventOnCompletion(uint64_t fenceValue, void* hEvent) override { return false; }
        uint64_t GetFrameIndex() const override { return 0; }

    private:
        ID3D11DeviceContext* m_pDeviceContext;
        ID3D11Device* m_pDevice;

        // GPU 测量
        ComPtr<ID3D11Query> m_pTimestampStart;
        ComPtr<ID3D11Query> m_pTimestampEnd;
        ComPtr<ID3D11Query> m_pDisjoint;
    };
}
