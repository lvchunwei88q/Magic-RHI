#pragma once
#include <CoreMinimal.h>

#include <vulkan.h>
#include <Common/Check.h>
#include <Common/RHIConfig.h>
#include <RHICommandList.h>
#include "RHIResourceVulKan.h"
#include "RHIRootSignatureVulKan.h"

namespace RHI
{
    class CommandAllocatorVulKan : public RHICommandAllocator
    {
    public:
        CommandAllocatorVulKan(RHICmdType type, VkCommandPool commandPool)
            : RHICommandAllocator(type)
            , m_CommandPool(commandPool) {}
        virtual ~CommandAllocatorVulKan() = default;

        VkCommandPool GetCommandPool() const { return m_CommandPool; }

    private:
        VkCommandPool m_CommandPool;
    };

    class CommandListVulKan : public RHICommandList
    {
    public:
        CommandListVulKan(RHICommandAllocator* pCmdAllocator, VkCommandBuffer commandBuffer)
            : RHICommandList(pCmdAllocator)
            , m_CommandBuffer(commandBuffer) {}
        ~CommandListVulKan() override = default;

        void BeginRecording() override;
        void EndRecording() override;

        // Input assembler
        virtual void IASetPrimitiveTopology(RHIPrimitiveTopology topology, uint32_t controlPointCount = 1) override;
        virtual void IASetVertexBuffers(uint32_t startSlot, uint32_t numBuffers, RHIVertexBuffer* const* ppBuffers, const uint64_t* pOffsets = nullptr) override;
        virtual void IASetIndexBuffer(RHIIndexBuffer* pIndexBuffer, RHIIndexFormat format, uint64_t offset = 0) override;

        // Rasterizer
        virtual void RSSetViewports(uint32_t numViewports, const RHIViewport* pViewports) override;
        virtual void RSSetScissorRects(uint32_t numRects, const RHIRect* pRects) override;
        
        // Output merger state
        virtual void OMSetRenderTargets(uint32_t numRenderTargets, class RHIRenderTargetView* const* ppViews, bool RTsSingleHandleToDescriptorRange, class RHIDepthStencilView* pDepthStencilView = nullptr) override;
        virtual void OMSetBlendState(class RHIBlendState* pState, const float* blendFactor = nullptr, uint32_t sampleMask = 0xFFFFFFFF) override;
        virtual void OMSetDepthStencilState(class RHIDepthStencilState* pState, uint32_t stencilRef = 0) override;

        // Draw
        virtual void Draw(uint32_t vertexCount, uint32_t startVertexLocation) override;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation) override;
        virtual void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) override;
        virtual void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) override;
        virtual void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;

        // Clear
        virtual void ClearRenderTargetView(class RHIRenderTargetView* pView, const float* colorRGBA) override;
        virtual void ClearDepthStencilView(class RHIDepthStencilView* pView, RHIClearFlags clearFlags, float depth, uint8_t stencil) override;

        // Resource operation
        virtual void CopyResource(RHIResource* pDstResource, RHIResource* pSrcResource) override;
        virtual void CopyBufferRegion(RHIBuffer* pDstBuffer, uint64_t dstOffset, RHIBuffer* pSrcBuffer, uint64_t srcOffset, uint64_t numBytes) override;

        // Resource barrier
        virtual void ResourceBarrier(uint32_t numBarriers, const BarrierDesc* pBarriers) override;

        // Root signature set
        virtual void SetGraphicsRootSignature(RHIRootSignature* pRootSignature) override;
        virtual void SetComputeRootSignature(RHIRootSignature* pRootSignature) override;
        virtual void SetDescriptorHeaps(uint32_t numHeaps, RHIDescriptorHeap* const* ppHeaps) override;

        // Pipeline state set
        virtual void SetPipelineState(RHIPipelineState* pPipelineState, PipelineStateType stateType) override;

        // Graphics pipeline binding
        virtual void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, RHIDescriptorHeap* pDescriptorHeap, uint32_t offsetInDescriptorsFromTableStart) override;
        virtual void SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        virtual void SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        virtual void SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        // The last parameter is to set the offset of your data in the target buffer
        virtual void SetGraphicsRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues) override;
        virtual void SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues) override;

        // Compute pipeline binding
        virtual void SetComputeRootDescriptorTable(uint32_t rootParameterIndex, RHIDescriptorHeap* pDescriptorHeap, uint32_t offsetInDescriptorsFromTableStart) override;
        virtual void SetComputeRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        virtual void SetComputeRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        virtual void SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress) override;
        // The last parameter is to set the offset of your data in the target buffer
        virtual void SetComputeRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues) override;
        virtual void SetComputeRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues) override;

        VkCommandBuffer GetCommandBuffer() const { return m_CommandBuffer; }

    private:
        VkCommandBuffer m_CommandBuffer;

        // Cached currently-bound root signatures (for vkCmdBindDescriptorSets / vkCmdPushConstants access)
        RHIRootSignatureVulKan* m_pCurrentGraphicsRS = nullptr;
        RHIRootSignatureVulKan* m_pCurrentComputeRS = nullptr;
    };

    using GraphicsCommandListVulKan = CommandListVulKan;
    using ComputeCommandListVulKan = CommandListVulKan;
    using CopyCommandListVulKan = CommandListVulKan;

    class CommandQueueVulKan : public RHICommandQueue
    {
    public:
        CommandQueueVulKan(RHICmdType InType, VkQueue queue, uint32_t queueFamilyIndex, const VkDevice* device, const VkPhysicalDevice* physicalDevice)
            : RHICommandQueue(InType)
            , m_Queue(queue)
            , m_QueueFamilyIndex(queueFamilyIndex)
            , m_Device(device)
            , m_PhysicalDevice(physicalDevice) {
                // Create Timeline Semaphore
                VkSemaphoreTypeCreateInfo timelineInfo{};
                timelineInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
                timelineInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
                timelineInfo.initialValue = 0;

                VkSemaphoreCreateInfo semaphoreInfo{};
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                semaphoreInfo.pNext = &timelineInfo;

                VkResult result = vkCreateSemaphore(GetDevice(), &semaphoreInfo, nullptr,
                 &m_TimelineSemaphore);
                if (result != VK_SUCCESS) 
                    Core::ErrorCapture::Capture("Failed to create timeline semaphore");
            }
        ~CommandQueueVulKan() override {
            WaitForGPU(); // Wait for GPU to finish
            // Destroy timeline semaphore
            if (m_TimelineSemaphore != VK_NULL_HANDLE) {
                vkDestroySemaphore(GetDevice(), m_TimelineSemaphore, nullptr);
                m_TimelineSemaphore = VK_NULL_HANDLE;
            }
        };
        // No copying
        CommandQueueVulKan(const CommandQueueVulKan&) = delete;
        CommandQueueVulKan& operator=(const CommandQueueVulKan&) = delete;

        void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) override;
        void BeginFrame() override;
        void EndFrame() override;
        void WaitForGPU() override;

         // Synchronous operation
        void Signal(uint64_t fenceValue) override;
        bool GetTimestampFrequency(uint64_t* frequency) override;
        uint64_t GetFrameIndex() const override;

        const VkDevice GetDevice() const;
        const VkPhysicalDevice GetPhysicalDevice() const;
        const VkQueue GetQueue() const { return m_Queue; }
        uint32_t GetQueueFamilyIndex() const { return m_QueueFamilyIndex; }
    private:
        const VkQueue m_Queue;
        const uint32_t m_QueueFamilyIndex;
        // Here we just store device handle references
        const VkDevice* m_Device;
        const VkPhysicalDevice* m_PhysicalDevice;

        // use Timeline semaphore
        VkSemaphore m_TimelineSemaphore = VK_NULL_HANDLE;
        uint64_t m_FenceValues[RHI_MULTI_BUFFERING] = {0};
        uint64_t m_NextFenceValue = 1;
        uint32_t m_CurrentFrame = 0;
        uint64_t m_LastSubmittedFenceValue = 0;
    };

    using GraphicsCommandQueueVulKan = CommandQueueVulKan;
    using ComputeCommandQueueVulKan = CommandQueueVulKan;
    using CopyCommandQueueVulKan = CommandQueueVulKan;
}
