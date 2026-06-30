#include "RHICommandListVulKan.h"
#include "RHIVulKan.h"

namespace RHI
{
    // ===========================================================================
    // Create command dispatcher
    std::shared_ptr<RHICommandAllocator> DeviceVulKan::CreateCommandAllocator(RHICmdType type)
    {
        return nullptr;
    }

    std::shared_ptr<RHICommandList> DeviceVulKan::CreateCommandList(std::shared_ptr<RHICommandAllocator>& allocator)
    {
        return nullptr;
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

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
        submitInfo.pCommandBuffers = commandBuffers.data();

        if (m_Fence == nullptr)
        {
            VkFenceCreateInfo fenceInfo = {};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            vkCreateFence(m_Device, &fenceInfo, nullptr, &m_Fence);
        }

        VkResult result = vkQueueSubmit(m_Queue, 1, &submitInfo, m_Fence);
        if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("Failed to submit command buffer");
        }
    }

    void CommandQueueVulKan::BeginFrame()
    {
        m_CurrentFrame = (m_CurrentFrame + 1) % RHI_MULTI_BUFFERING;

        if (m_FenceValues[m_CurrentFrame] != 0)
        {
            vkWaitForFences(m_Device, 1, &m_Fence, VK_TRUE, UINT64_MAX);
            vkResetFences(m_Device, 1, &m_Fence);
        }
    }

    void CommandQueueVulKan::EndFrame()
    {
        m_FenceValues[m_CurrentFrame] = m_NextFenceValue++;
    }

    void CommandQueueVulKan::WaitForGPU()
    {
        if (m_Fence == nullptr)
        {
            VkFenceCreateInfo fenceInfo = {};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            vkCreateFence(m_Device, &fenceInfo, nullptr, &m_Fence);
        }

        vkQueueSubmit(m_Queue, 0, nullptr, m_Fence);
        vkWaitForFences(m_Device, 1, &m_Fence, VK_TRUE, UINT64_MAX);
    }
}
