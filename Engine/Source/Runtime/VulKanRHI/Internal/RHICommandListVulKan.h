#pragma once
#include <CoreMinimal.h>

#include <vulkan.h>
#include <Common/Check.h>
#include <Common/RHIConfig.h>
#include <RHICommandList.h>
#include "RHIResourceVulKan.h"
#include "RHIRootSignatureVulKan.h"
#include "RHIVulKan.h"

#define DEFINE_QUEUE_DEVICE_GETTERS(Device,m_PhysicalDevice,m_Queue)    \
    const VkDevice GetDevice() const {                                  \
        if (!m_Device) {                                                \
            ThrowErrorMessage("Queue: Device is null");                 \
            return VK_NULL_HANDLE;                                      \
        }                                                               \
        return *m_Device;                                               \
    }                                                                   \
    const VkPhysicalDevice GetPhysicalDevice() const {                  \
        if (!m_PhysicalDevice) {                                        \
            ThrowErrorMessage("Queue: PhysicalDevice is null");         \
            return VK_NULL_HANDLE;                                      \
        }                                                               \
        return *m_PhysicalDevice;                                       \
    }                                                                   \
    const VkQueue GetQueue() const { return m_Queue; }

namespace RHI
{
    class CommandPoolVulKan : public RHICommandPool
    {
    public:
        CommandPoolVulKan(RHICmdType type, VkCommandPool commandPool, const VkDevice* device)
            : RHICommandPool(type)
            , m_Device(device)
            , m_CommandPool(commandPool) {}
        virtual ~CommandPoolVulKan();

        VkCommandPool GetCommandPool() const { return m_CommandPool; }
        // reset command pool memory
        virtual void Reset() const override;

    private:
        VkCommandPool m_CommandPool;
        const VkDevice* m_Device = nullptr;
    };

    class CommandListVulKan : public RHICommandList
    {
    public:
        CommandListVulKan(RHICommandPoolPtr pCommandPool, VkCommandBuffer commandBuffer)
            : RHICommandList(pCommandPool)
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

    class GraphicsCommandQueueVulKan : public RHIFrameCommandQueue
    {
    public:
        GraphicsCommandQueueVulKan(RHICmdType InType, VkQueue queue, uint32_t queueFamilyIndex, const DeviceVulKan* device, const VkPhysicalDevice* physicalDevice)
            : RHIFrameCommandQueue(InType)
            , m_Queue(queue)
            , m_QueueFamilyIndex(queueFamilyIndex)
            , m_VkDevice(device)
            , m_PhysicalDevice(physicalDevice){
                // Fence creation info
                VkFenceCreateInfo fenceInfo{};
                fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

                VkResult result = vkCreateFence (GetDevice(), &fenceInfo, nullptr, &m_inFlightFence);
                if (result != VK_SUCCESS)
                {
                    ThrowErrorMessage("Failed to create a usable image fence");
                }

                // We choose to create dynamically
                // m_RenderFinishedSemaphores.resize(m_pBackBuffers.size());
                // for (size_t i = 0; i < m_pBackBuffers.size(); i++)
                // {
                //     result = vkCreateSemaphore(GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]);
                //     if (result != VK_SUCCESS) {
                //         ThrowErrorMessage("Failed to create synchronization objects for an image!");
                //     }
                // }
            }
        ~GraphicsCommandQueueVulKan() override {
            WaitForGPU(); // Wait for GPU to finish

            for (auto renderFinishedSemaphore : m_RenderFinishedSemaphores)
            {
                vkDestroySemaphore(GetDevice(), renderFinishedSemaphore, nullptr);
            }

            vkDestroyFence (GetDevice(), m_inFlightFence, nullptr);
        };
        // No copying
        GraphicsCommandQueueVulKan(const GraphicsCommandQueueVulKan&) = delete;
        GraphicsCommandQueueVulKan& operator=(const GraphicsCommandQueueVulKan&) = delete;

        void BeginFrame() override;
        void EndFrame() override;
        void Present() override;
        void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) override;
        void WaitForGPU() override;

        void SetGraphicsQueueSwapChainInfo(GraphicsQueueSwapChainInfo Info) override;
        void ResetCommandPoolMemory(const RHICommandPool* cmdPool) override;
        bool GetTimestampFrequency(uint64_t* frequency) override;
        uint64_t GetFrameIndex() const override;

        // Self Tools Function
        const VkDevice GetDevice() const {
            if (!m_VkDevice) {
                ThrowErrorMessage("Queue: Device is null");
                return nullptr;
            }
            const VkDevice* device = m_VkDevice->GetDevice();
            return *device;
        }
        const VkPhysicalDevice GetPhysicalDevice() const {
            if (!m_PhysicalDevice) {
                ThrowErrorMessage("Queue: PhysicalDevice is null");
                return nullptr;
            }
            return *m_PhysicalDevice;
        }
        const VkQueue GetQueue() const { return m_Queue; }
        uint32_t GetQueueFamilyIndex() const { return m_QueueFamilyIndex; }
    private:
        const VkQueue m_Queue;
        const uint32_t m_QueueFamilyIndex;
        // Here we just store device handle references
        const DeviceVulKan* m_VkDevice;
        const VkPhysicalDevice* m_PhysicalDevice;

        // Set up semaphores and fences These will be used for frame sync in Vulkan because the GPU is always asynchronous
        std::vector<VkSemaphore> m_RenderFinishedSemaphores; // TODO: 实现创建功能
        VkFence m_inFlightFence = VK_NULL_HANDLE;
        // We can record the commands needed for the next frame in advance while the GPU is working asynchronously,
        // so the GPU can start rendering the next frame right away without waiting for the CPU to finish.
        uint32_t m_CurrentFrameIndex = 0;

        // SwapChain Info 
        GraphicsQueueSwapChainInfo m_SwapChainInfo;
    };

    class ComputeCommandQueueVulKan : public RHICommandQueue
    {
    public:
        ComputeCommandQueueVulKan(RHICmdType InType, VkQueue queue, uint32_t queueFamilyIndex, const VkDevice* device, const VkPhysicalDevice* physicalDevice)
            : RHICommandQueue(InType)
            , m_Queue(queue)
            , m_QueueFamilyIndex(queueFamilyIndex)
            , m_Device(device)
            , m_PhysicalDevice(physicalDevice){
                // Fence creation info
                VkFenceCreateInfo fenceInfo{};
                fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

                VkResult result = vkCreateFence (GetDevice(), &fenceInfo, nullptr, &m_ComputeFence);
            }
        ~ComputeCommandQueueVulKan() override{
            WaitForGPU(); // Wait for GPU to finish
            vkDestroyFence (GetDevice(), m_ComputeFence, nullptr);
        };

        void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) override;
        void WaitForGPU() override;
        bool GetTimestampFrequency(uint64_t* frequency) override;

        // Self Tools Function
        DEFINE_QUEUE_DEVICE_GETTERS(m_Device,m_PhysicalDevice,m_Queue)
    private:
        const VkQueue m_Queue;
        const uint32_t m_QueueFamilyIndex;
        // Here we just store device handle references
        const VkDevice* m_Device;
        const VkPhysicalDevice* m_PhysicalDevice;

        // Set fences These will be used for frame sync in Vulkan because the GPU is always asynchronous
        VkFence m_ComputeFence = VK_NULL_HANDLE;
    };

    class CopyCommandQueueVulKan : public RHICommandQueue
    {  
    public:
        CopyCommandQueueVulKan(RHICmdType InType, VkQueue queue, uint32_t queueFamilyIndex, const VkDevice* device, const VkPhysicalDevice* physicalDevice)
            : RHICommandQueue(InType)
            , m_Queue(queue)
            , m_QueueFamilyIndex(queueFamilyIndex)
            , m_Device(device)
            , m_PhysicalDevice(physicalDevice){
                // Fence creation info
                VkFenceCreateInfo fenceInfo{};
                fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

                VkResult result = vkCreateFence (GetDevice(), &fenceInfo, nullptr, &m_CopyFence);
            }
        ~CopyCommandQueueVulKan() override{
            WaitForGPU(); // Wait for GPU to finish
            vkDestroyFence (GetDevice(), m_CopyFence, nullptr);
        };

        void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) override;
        void WaitForGPU() override;
        bool GetTimestampFrequency(uint64_t* frequency) override;

        // Self Tools Function
        DEFINE_QUEUE_DEVICE_GETTERS(m_Device,m_PhysicalDevice,m_Queue)
    private:
        const VkQueue m_Queue;
        const uint32_t m_QueueFamilyIndex;
        // Here we just store device handle references
        const VkDevice* m_Device;
        const VkPhysicalDevice* m_PhysicalDevice;

        // Set fences These will be used for frame sync in Vulkan because the GPU is always asynchronous
        VkFence m_CopyFence = VK_NULL_HANDLE;
    };
}
