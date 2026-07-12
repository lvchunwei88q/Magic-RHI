#include "RHICommandListVulKan.h"
#include "RHIVulKan.h"

namespace RHI
{
    namespace
    {
        uint32_t GetQueueFamilyIndex(const DeviceVulKan* device, RHICmdType type)
        {
            switch (type)
            {
            case RHICmdType::Graphics:  //Graphics queue family index
                return device->GetGraphicsQueueFamilyIndex();
            case RHICmdType::Compute:   //Compute queue family index
                return device->GetComputeQueueFamilyIndex();
            case RHICmdType::Copy:      //Copy queue family index
                return device->GetCopyQueueFamilyIndex();
            default:
#ifdef RHI_ENABLE_DEBUG_INFO
                // Throw an exception if in debug mode
                ThrowErrorMessage("Invalid RHICmdType");
#endif
                return 0;
            }
        }
    }
    // ===========================================================================
    // Create command dispatcher
    std::shared_ptr<RHICommandAllocator> DeviceVulKan::CreateCommandAllocator(RHICmdType type)
    {
        // Fill create command pool info
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = GetQueueFamilyIndex(this, type);
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        // Create command pool
        VkCommandPool commandPool;
        VkResult result = vkCreateCommandPool(m_Device, &poolInfo, nullptr, &commandPool);
        if (result != VK_SUCCESS)
            ThrowErrorMessage("Failed to create command pool");
        
        return std::make_shared<CommandAllocatorVulKan>(type, commandPool);
    }

    std::shared_ptr<RHICommandList> DeviceVulKan::CreateCommandList(std::shared_ptr<RHICommandAllocator>& allocator)
    {
        CommandAllocatorVulKan* pAllocator = SafeCast<CommandAllocatorVulKan>(allocator.get());
        if (pAllocator == nullptr) {
            ThrowErrorMessage("CommandAllocatorVulKan is nullptr");
            return nullptr;
        }
        RHICmdType type = pAllocator->GetCmdType();

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = pAllocator->GetCommandPool();
        // Note that here we've made a trade-off: we force all Cmds to use the main buffer, and only one can be created at a time.
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        VkResult result = vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);
        if (result != VK_SUCCESS)
            ThrowErrorMessage("Failed to allocate command buffer");
        
        return std::make_shared<CommandListVulKan>(pAllocator, commandBuffer);
    }
    // ===========================================================================

    void CommandListVulKan::BeginRecording()
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VkResult result = vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);
        if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("Failed to begin recording command buffer");
        }
    }

    void CommandListVulKan::EndRecording()
    {
        VkResult result = vkEndCommandBuffer(m_CommandBuffer);
        if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("Failed to end recording command buffer");
        }
    }

    const VkDevice CommandQueueVulKan::GetDevice() const
    {
        if (!m_Device)
        {
            ThrowErrorMessage("CommandQueueVulKan: Device is null");
            return VK_NULL_HANDLE;
        }
        return *m_Device;
    }

    const VkPhysicalDevice CommandQueueVulKan::GetPhysicalDevice() const
    {
        if (!m_PhysicalDevice)
        {
            ThrowErrorMessage("CommandQueueVulKan: PhysicalDevice is null");
            return VK_NULL_HANDLE;
        }
        return *m_PhysicalDevice;
    }

    void CommandQueueVulKan::ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists)
    {
        if (cmdLists.empty())
            return;

        std::vector<VkCommandBuffer> commandBuffers;
        commandBuffers.reserve(cmdLists.size());

        for (const auto& cmdList : cmdLists)
        {
            CommandListVulKan* vulkanCmdList = SafeCast<CommandListVulKan>(cmdList.get());
            if (vulkanCmdList)
            {
                commandBuffers.push_back(vulkanCmdList->GetCommandBuffer());
            }
        }

        // Create submit info
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
        submitInfo.pCommandBuffers = commandBuffers.data();

        // Submit command buffers
        VkResult result = vkQueueSubmit(m_Queue, 1, &submitInfo, VK_NULL_HANDLE);
        if (result != VK_SUCCESS)
            ThrowErrorMessage("Failed to submit command buffer");
        
    }

    void CommandQueueVulKan::BeginFrame()
    {
        // NOT IMPLEMENTED
    }

    void CommandQueueVulKan::EndFrame()
    {
        m_FenceValues[m_CurrentFrame] = m_NextFenceValue++;
        Signal(m_FenceValues[m_CurrentFrame]);
        // Just jot down the value of the ending frame
        m_LastSubmittedFenceValue = m_FenceValues[m_CurrentFrame];
        m_CurrentFrame = (m_CurrentFrame + 1) % RHI_MULTI_BUFFERING;
    }

    void CommandQueueVulKan::WaitForGPU()
    {
        uint64_t fenceValue = m_LastSubmittedFenceValue;
        if (fenceValue == 0) return;

        VkSemaphoreWaitInfo waitInfo{};
        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = &m_TimelineSemaphore;
        waitInfo.pValues = &fenceValue;

        vkWaitSemaphores(GetDevice(), &waitInfo, UINT64_MAX);
    }

    void CommandQueueVulKan::Signal(uint64_t fenceValue)
    {
        // Configure Timeline Semaphore submit info
        VkTimelineSemaphoreSubmitInfo timelineInfo{};
        timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        timelineInfo.signalSemaphoreValueCount = 1;
        timelineInfo.pSignalSemaphoreValues = &fenceValue;

        // Configure submit info
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = &timelineInfo;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_TimelineSemaphore;

        // Submit submit info
        VkResult result = vkQueueSubmit(m_Queue, 1, &submitInfo, VK_NULL_HANDLE);
        if (result != VK_SUCCESS) {
            // Error handling
            Core::ErrorCapture::Capture("Failed to signal timeline semaphore");
        }
    }

    bool CommandQueueVulKan::GetTimestampFrequency(uint64_t* frequency)
    {
        if (!frequency)
            return false;

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);
        
        // The unit of timestampPeriod is nanoseconds
        // Frequency = 1,000,000,000 / timestampPeriod
        *frequency = static_cast<uint64_t>(1e9 / properties.limits.timestampPeriod);
        return true;
    }

    uint64_t CommandQueueVulKan::GetFrameIndex() const
    {
        return m_CurrentFrame;
    }
}
