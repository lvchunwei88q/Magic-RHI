#pragma once

#include <cstddef>
#include <cstdint>
#include <d3d11.h>
#include <Common/Check.h>
#include <Tools/Singleton.h>
#include <RHICommandList.h>

#include "RHIResourceD3D11.h"
#include "DescriptorHeapD3D11.h"
#include "RHIRootSignatureD3D11.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace RHI
{
    // Command draw callback.
    // Since we need to simulate the behavior of signing,
    // we need a callback function before Draw that allows us to submit our data before submission.
    using CommandDrawCallback = void (*)(RHIRootSignatureD3D11* pRootSignature, ID3D11DeviceContext* pDeviceContext);

    class CommandDrawCallbackD3D11 : public Singleton<CommandDrawCallbackD3D11>
    {
    public:
        CommandDrawCallbackD3D11() = default;
        ~CommandDrawCallbackD3D11() = default;

        void SetCallback(CommandDrawCallback callback) { m_Callback = callback; }
        void ClearCallback() { m_Callback = nullptr; }

        void Execute(RHIRootSignatureD3D11* pRootSignature, ID3D11DeviceContext* pDeviceContext) const
        {
            if(m_Callback){
                m_Callback(pRootSignature, pDeviceContext);
            }
        }
    private:
        CommandDrawCallback m_Callback = nullptr;
    };

    // For DX11, we don't really need an allocator and this concept doesn't even exist,
    //  but we can mimic the design of modern APIs to carry out some tasks that require 'allocation'.
    class CommandAllocatorD3D11 : public RHICommandAllocator
    {
        public:
            CommandAllocatorD3D11(RHICmdType type, ID3D11DeviceContext* pContext)
                : RHICommandAllocator(type)
                , m_pDeviceContext(pContext) {}
            virtual ~CommandAllocatorD3D11() = default;

            ID3D11DeviceContext* GetDeviceContext() const { return m_pDeviceContext; }
        private:
            ID3D11DeviceContext* m_pDeviceContext;
    };

    // Temporary binding allocation, because in the command list we only handle the current state,
    // while they are determined by SetRootSignature and SetDescriptorHeaps.
    struct TempBindingAssignmentD3D11 {
        TempBindingAssignmentD3D11() = default;
        TempBindingAssignmentD3D11(RHIRootSignatureD3D11* pRootSignature, std::vector<DescriptorHeapD3D11*> ppHeaps)
            : m_pRootSignature(pRootSignature), m_ppHeaps(ppHeaps) {}
        RHIRootSignatureD3D11* m_pRootSignature = nullptr;
        // Descriptor heaps.
        std::vector<DescriptorHeapD3D11*> m_ppHeaps;

        // Clear the binding assignment.
        void clear() { m_pRootSignature = nullptr; m_ppHeaps.clear(); }
        // Check if the binding assignment is valid.
        bool IsValid() const { return m_pRootSignature != nullptr && !m_ppHeaps.empty(); }
    };

    class CommandListD3D11 : public RHICommandList
    {
    public:
        CommandListD3D11(RHICommandAllocator* pCmdAllocator)
            : RHICommandList(pCmdAllocator) {}
        ~CommandListD3D11() override = default;

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

        // Root signature
        void SetGraphicsRootSignature(RHIRootSignature* pRootSignature) override;
        void SetComputeRootSignature(RHIRootSignature* pRootSignature) override;
        void SetDescriptorHeaps(uint32_t numHeaps, RHIDescriptorHeap* const* ppHeaps) override;

        // Pipeline state set
        void SetPipelineState(RHIPipelineState* pPipelineState, PipelineStateType stateType) override;

        // Graphics Pipeline binding
        void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, RHIDescriptorHeap* pDescriptorHeap, uint32_t offsetInDescriptorsFromTableStart) override;
        void SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetGraphicsRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues) override;
        void SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues) override;

        // Compute Pipeline binding
        void SetComputeRootDescriptorTable(uint32_t rootParameterIndex, RHIDescriptorHeap* pDescriptorHeap, uint32_t offsetInDescriptorsFromTableStart) override;
        void SetComputeRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetComputeRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        void SetComputeRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues) override;
        void SetComputeRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues) override;

    private:
        // We use allocators to bind various resources
        //ID3D11DeviceContext* m_pDeviceContext;
        // Here, use this function to quickly get the allocator
        CommandAllocatorD3D11* GetAllocator() { 
            if(m_pAllocator == nullptr){
#if RHI_ENABLE_DEBUG_INFO
                ThrowErrorMessage("CommandAllocatorD3D11 is nullptr");
#endif
                return nullptr;
            }
            return SafeCast<CommandAllocatorD3D11>(m_pAllocator);
        }

        // This is current binding assignment
        TempBindingAssignmentD3D11 m_tempBindingAssignment;
    };

    class CommandQueueD3D11 : public RHICommandQueue
    {
    public:
        CommandQueueD3D11(RHICmdType InType, ID3D11DeviceContext* pContext, ID3D11Device* pDevice)
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
        ~CommandQueueD3D11() override = default;

        void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) override;
        void BeginFrame() override;
        void EndFrame() override;
        void WaitForGPU() override;

        // Synchronous operation
        void Signal(uint64_t fenceValue) override;
        bool GetTimestampFrequency(uint64_t* frequency) override;
        bool SetEventOnCompletion(uint64_t fenceValue, void* hEvent);
        uint64_t GetFrameIndex() const override;

    private:
        ID3D11DeviceContext* m_pDeviceContext;
        ID3D11Device* m_pDevice;

        // GPU 测量
        ComPtr<ID3D11Query> m_pTimestampStart;
        ComPtr<ID3D11Query> m_pTimestampEnd;
        ComPtr<ID3D11Query> m_pDisjoint;
    };
}
