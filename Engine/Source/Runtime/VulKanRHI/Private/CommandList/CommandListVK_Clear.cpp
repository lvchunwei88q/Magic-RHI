#include "RHICommandListVulKan.h"

namespace RHI
{
    void CommandListVulKan::ClearRenderTargetView(RHIRenderTargetView* pView, const float* colorRGBA)
    {
        ThrowIf(pView == nullptr, "Render target view is null");
        ThrowIf(colorRGBA == nullptr, "Clear color is null");
        ThrowIf(!m_IsDynamicRendering, "ClearRenderTargetView requires active dynamic rendering");

        auto* renderTarget = SafeCast<RenderTargetViewVulKan>(pView);
        ThrowIf(renderTarget == nullptr, "Render target view is not a Vulkan render target view");

        // Find the index of the render target in the current dynamic rendering scope
        uint32_t attachmentIndex = 0;
        while (attachmentIndex < m_CurrentRenderTargets.size() && m_CurrentRenderTargets[attachmentIndex] != renderTarget)  ++attachmentIndex;
        // If the render target is not found, throw an error
        ThrowIf(attachmentIndex == m_CurrentRenderTargets.size(),
            "Render target view is not bound to the current dynamic rendering scope");

        VkClearAttachment attachment{};
        attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        attachment.colorAttachment = attachmentIndex;
        attachment.clearValue.color.float32[0] = colorRGBA[0];
        attachment.clearValue.color.float32[1] = colorRGBA[1];
        attachment.clearValue.color.float32[2] = colorRGBA[2];
        attachment.clearValue.color.float32[3] = colorRGBA[3];

        VkClearRect clearRect{};
        clearRect.rect = m_CurrentRenderArea;
        clearRect.baseArrayLayer = 0;
        clearRect.layerCount = 1;
        vkCmdClearAttachments(m_CommandBuffer, 1, &attachment, 1, &clearRect);
    }

    void CommandListVulKan::ClearDepthStencilView(RHIDepthStencilView* pView, RHIClearFlags clearFlags, float depth, uint8_t stencil)
    {
    }
}
