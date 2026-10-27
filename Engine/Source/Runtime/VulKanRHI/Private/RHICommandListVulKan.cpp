#include "RHICommandListVulKan.h"
#include "CoreMinimal.h"
#include "RHIVulKan.h"

#define EXTRACT_VK_COMMAND_BUFFERS(outBuffers, inCmdLists)                                          \
    do {                                                                                            \
        outBuffers.clear();                                                                         \
        outBuffers.reserve(inCmdLists.size());                                                      \
        for (const auto& _cmdList : inCmdLists)                                                     \
        {                                                                                           \
            CommandListVulKan* _vulkanCmdList = SafeCast<CommandListVulKan>(_cmdList.get());        \
            if (_vulkanCmdList)                                                                     \
            {                                                                                       \
                outBuffers.push_back(_vulkanCmdList->GetCommandBuffer());                           \
            }                                                                                       \
        }                                                                                           \
    } while (0)

#define MAKE_COMPUTE_COPY_SUBMIT_INFO(submitInfo, commandBuffers)                           \
    do {                                                                                    \
        submitInfo = {};                                                                    \
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;                                   \
        submitInfo.waitSemaphoreCount = 0;                                                  \
        submitInfo.pWaitSemaphores = nullptr;                                               \
        submitInfo.pWaitDstStageMask = nullptr;                                             \
        submitInfo.commandBufferCount = static_cast<uint32_t>((commandBuffers).size());     \
        submitInfo.pCommandBuffers = (commandBuffers).data();                               \
        submitInfo.signalSemaphoreCount = 0;                                                \
        submitInfo.pSignalSemaphores = nullptr;                                             \
    } while (0)

#define GET_TIMESTAMP_FREQUENCY(freqPtr, device)                                    \
    do {                                                                            \
        if (!(freqPtr)) return false;                                               \
        VkPhysicalDeviceProperties _props;                                          \
        vkGetPhysicalDeviceProperties((device), &_props);                           \
        if (_props.limits.timestampPeriod == 0.0f) return false;                    \
                                                                                    \
        /* Frequency = 1,000,000,000 / timestampPeriod */                           \
        *(freqPtr) = static_cast<uint64_t>(1e9 / _props.limits.timestampPeriod);    \
        return true;                                                                \
    } while (0)

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
    std::shared_ptr<RHICommandPool> DeviceVulKan::CreateCommandPool(RHICmdType type)
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
        
        return std::make_shared<CommandPoolVulKan>(type, commandPool, &m_Device);
    }

    std::shared_ptr<RHICommandList> DeviceVulKan::CreateCommandList(std::shared_ptr<RHICommandPool>& pool)
    {
        CommandPoolVulKan* pPool = SafeCast<CommandPoolVulKan>(pool.get());
        if (pPool == nullptr) {
            ThrowErrorMessage("CommandPoolVulKan is nullptr");
            return nullptr;
        }
        RHICmdType type = pPool->GetCmdType();

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = pPool->GetCommandPool();
        // Note, here we made a trade-off: we force all Cmds to use the main buffer, and only create one at a time.
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        VkResult result = vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);
        if (result != VK_SUCCESS)
            ThrowErrorMessage("Failed to allocate command buffer");
        
        return std::make_shared<CommandListVulKan>(pPool, commandBuffer);
    }
    // ===========================================================================

    // reset command pool memory
    void CommandPoolVulKan::Reset() const
    {
        if (!m_Device)
        {
            ThrowErrorMessage("CommandPoolVulKan: Device is null");return;
        }
        VkResult result = vkResetCommandPool(*m_Device, m_CommandPool, 0);
        
        if (result != VK_SUCCESS)
            ThrowErrorMessage("Failed to reset command pool");
        
    }

    void CommandListVulKan::BeginRecording()
    {
        // Check if recording is already in progress
        if (m_pCommandPool->IsRecording()) 
            ThrowErrorMessage("CommandListVulKan: BeginRecording() was called multiple times, be careful not to use multithreading");
        // Set is recording to true
        m_pCommandPool->SetIsRecording(true);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VkResult result = vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);
        if (result != VK_SUCCESS)
            ThrowErrorMessage("Failed to begin recording command buffer");
    }

    void CommandListVulKan::EndRecording()
    {
        VkResult result = vkEndCommandBuffer(m_CommandBuffer);
        if (result != VK_SUCCESS)
            ThrowErrorMessage("Failed to end recording command buffer");
        // Set is recording to false
        m_pCommandPool->SetIsRecording(false);
    }

    // ============================================================ GraphicsCommandQueueVulKan

    void GraphicsCommandQueueVulKan::ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists)
    {
        if (cmdLists.empty())
            return;

        // Reset the render-complete semaphore from the previous frame
        vkResetFences(GetDevice(), 1, &m_inFlightFence);
        
        // Adjust the number of render-complete semaphores
        uint32_t SwapChainBackBufferCount = m_SwapChainInfo.SwapChainBackBufferCount_Vk;
        if (m_RenderFinishedSemaphores.size() != SwapChainBackBufferCount)
        {
            m_RenderFinishedSemaphores.resize(SwapChainBackBufferCount);
            for (size_t i = 0; i < SwapChainBackBufferCount; ++i)
            {
                VkSemaphoreCreateInfo semaphoreInfo = {};
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                VkResult result = vkCreateSemaphore(GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]);
                if (result != VK_SUCCESS)
                {
                    ThrowErrorMessage("Failed to create render finished semaphore");
                }
            }
        }

        std::vector<VkCommandBuffer> commandBuffers;
        EXTRACT_VK_COMMAND_BUFFERS(commandBuffers,cmdLists);

        // Create submit info
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // Set wait semaphore
        VkSemaphore vkImageAvailableSemaphore = reinterpret_cast<VkSemaphore>(m_SwapChainInfo.Semaphore_Vk);
        VkSemaphore waitSemaphores[] = { vkImageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
		// Submit command buffer
        submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
        submitInfo.pCommandBuffers = commandBuffers.data();

		// Set render completion semaphore
        VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_SwapChainInfo.imageIndex] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // Submit submit info
        VkResult result = vkQueueSubmit(m_Queue, 1, &submitInfo, m_inFlightFence);
        if (result != VK_SUCCESS) {
            // Error handling
            Core::ErrorCapture::Capture("Failed to signal timeline semaphore");
        }
        
    }

    void GraphicsCommandQueueVulKan::BeginFrame()
    {
        // Waiting for the previous frame to finish rendering
        vkWaitForFences (GetDevice(), 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    }

    void GraphicsCommandQueueVulKan::EndFrame()
    {
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % RHI_MULTI_BUFFERING;
    }

    void GraphicsCommandQueueVulKan::Present()
    {
        // Semaphore for getting the current SwapChain image index
        VkSemaphore RenderFinishedSemaphore[] = { m_RenderFinishedSemaphores[m_SwapChainInfo.imageIndex] };
        VkSwapchainKHR vkSwapChain = reinterpret_cast<VkSwapchainKHR>(m_SwapChainInfo.SwapChain);

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        // Set wait semaphore
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = RenderFinishedSemaphore;
        // Set up the swap chain, image index
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &vkSwapChain;
        presentInfo.pImageIndices = &m_SwapChainInfo.imageIndex;

        VkResult result = vkQueuePresentKHR(
            m_Queue,
            &presentInfo
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            Core::WarningCapture::Capture("This swap chain is outdated");
        }
        else if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("Failed to present swap chain image");
        }
    }

    void GraphicsCommandQueueVulKan::WaitForGPU()
    {
        vkQueueWaitIdle(GetQueue());
    }

    void GraphicsCommandQueueVulKan::SetGraphicsQueueSwapChainInfo(GraphicsQueueSwapChainInfo Info) 
    {
        m_SwapChainInfo = Info;
    }

    void GraphicsCommandQueueVulKan::ResetCommandPoolMemory(const RHICommandPool* cmdPool)
    {
        if (!cmdPool)
        {
            ThrowErrorMessage("CommandQueueVulKan: cmdPool is null");return;
        }
        cmdPool->Reset();
    }

    bool GraphicsCommandQueueVulKan::GetTimestampFrequency(uint64_t* frequency)
    {
        GET_TIMESTAMP_FREQUENCY(frequency, GetPhysicalDevice());
    }

    uint64_t GraphicsCommandQueueVulKan::GetFrameIndex() const
    {
        return m_CurrentFrameIndex;
    }

    // ============================================================ ComputeCommandQueueVulKan

    void ComputeCommandQueueVulKan::WaitForGPU()
    {
        vkWaitForFences(GetDevice(), 1, &m_ComputeFence, VK_TRUE, UINT64_MAX);
    }

    void ComputeCommandQueueVulKan::ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists)
    {
        if (cmdLists.empty())
            return;

        std::vector<VkCommandBuffer> commandBuffers;
        EXTRACT_VK_COMMAND_BUFFERS(commandBuffers,cmdLists);

        // Reset semaphore
        vkResetFences(GetDevice(), 1, &m_ComputeFence);

        // Create submit info
        VkSubmitInfo submitInfo{};
        MAKE_COMPUTE_COPY_SUBMIT_INFO(submitInfo,commandBuffers);

        // Submit submit info
        VkResult result = vkQueueSubmit(m_Queue, 1, &submitInfo, m_ComputeFence);
        if (result != VK_SUCCESS) {
            // Error handling
            Core::ErrorCapture::Capture("Failed to signal timeline semaphore");
        }
    }

    bool ComputeCommandQueueVulKan::GetTimestampFrequency(uint64_t* frequency)
    {
        GET_TIMESTAMP_FREQUENCY(frequency, GetPhysicalDevice());
    }

    // ============================================================ CopyCommandQueueVulKan
    
    void CopyCommandQueueVulKan::WaitForGPU()
    {
        vkWaitForFences(GetDevice(), 1, &m_CopyFence, VK_TRUE, UINT64_MAX);
    }

    void CopyCommandQueueVulKan::ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists)
    {
        if (cmdLists.empty())
            return;

        std::vector<VkCommandBuffer> commandBuffers;
        EXTRACT_VK_COMMAND_BUFFERS(commandBuffers,cmdLists);

        // Reset semaphore
        vkResetFences(GetDevice(), 1, &m_CopyFence);

        // Create submit info
        VkSubmitInfo submitInfo{};
        MAKE_COMPUTE_COPY_SUBMIT_INFO(submitInfo,commandBuffers);

        // Submit submit info
        VkResult result = vkQueueSubmit(m_Queue, 1, &submitInfo, m_CopyFence);
        if (result != VK_SUCCESS) {
            // Error handling
            Core::ErrorCapture::Capture("Failed to signal timeline semaphore");
        }
    }

    bool CopyCommandQueueVulKan::GetTimestampFrequency(uint64_t* frequency)
    {
        GET_TIMESTAMP_FREQUENCY(frequency, GetPhysicalDevice());
    }
}
