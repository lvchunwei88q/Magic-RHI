#pragma once

#include <vulkan.h>
#include <Common/Check.h>
#include <Common/RHIConfig.h>
#include <RHICommandList.h>

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

        VkCommandBuffer GetCommandBuffer() const { return m_CommandBuffer; }

    private:
        VkCommandBuffer m_CommandBuffer;
    };

    class CommandQueueVulKan : public RHICommandQueue
    {
    public:
        CommandQueueVulKan(RHICmdType InType, VkQueue queue, uint32_t queueFamilyIndex, VkDevice device)
            : RHICommandQueue(InType)
            , m_Queue(queue)
            , m_QueueFamilyIndex(queueFamilyIndex)
            , m_Device(device) {}
        ~CommandQueueVulKan() override = default;

        void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) override;
        void BeginFrame() override;
        void EndFrame() override;
        void WaitForGPU() override;

        VkQueue GetQueue() const { return m_Queue; }
        uint32_t GetQueueFamilyIndex() const { return m_QueueFamilyIndex; }

    private:
        VkQueue m_Queue;
        uint32_t m_QueueFamilyIndex;
        VkDevice m_Device;

        VkFence m_Fence = nullptr;
        uint64_t m_FenceValues[RHI_MULTI_BUFFERING] = {0};
        uint64_t m_NextFenceValue = 1;
        uint32_t m_CurrentFrame = 0;
    };
}
