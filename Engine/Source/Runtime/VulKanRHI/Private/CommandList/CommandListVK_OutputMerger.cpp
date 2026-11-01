#include "CoreMinimal.h"
#include "RHICommandListVulKan.h"

namespace RHI
{
    void CommandListVulKan::OMSetRenderTargets(uint32_t numRenderTargets, RHIRenderTargetView* const* ppViews, bool RTsSingleHandleToDescriptorRange, RHIDepthStencilView* pDepthStencilView)
    {
        ThrowIf(numRenderTargets > DRAW_MAX_RENDER_TARGETS, "numRenderTargets must be less than or equal to DRAW_MAX_RENDER_TARGETS");
        ThrowIf(numRenderTargets > 0 && ppViews == nullptr, "Render target views are null");

        // If dynamic rendering is already active, end it before starting a new one
        if(IsDynamicRendering()){
            Core::ErrorCapture::Capture("OMSetRenderTargets is being called right now, but dynamic rendering is already active. Please fix any possible issues in the list.");
            EndDynamicRendering();
        }

        std::vector<VkRenderingAttachmentInfoKHR> colorAttachments(numRenderTargets);
        uint32_t renderWidth = 0;
        uint32_t renderHeight = 0;

        for (uint32_t i = 0; i < numRenderTargets; ++i)
        {
            auto* renderTarget = SafeCast<RenderTargetViewVulKan>(ppViews[i]);
            ThrowIf(renderTarget == nullptr, "Render target view is not a Vulkan render target view");
            ThrowIf(renderTarget->GetImageView() == VK_NULL_HANDLE, "Render target image view is null");

            if (i == 0)
            {
                renderWidth = renderTarget->GetWidth();
                renderHeight = renderTarget->GetHeight();
            }
            else
            {
                ThrowIf(renderWidth != renderTarget->GetWidth() || renderHeight != renderTarget->GetHeight(),
                    "All Vulkan render targets must have the same dimensions");
            }

            VkRenderingAttachmentInfoKHR& attachment = colorAttachments[i];
            attachment = {};
            attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
            attachment.imageView = renderTarget->GetImageView();
            attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            m_CurrentRenderTargets.push_back(renderTarget);
        }

        VkRenderingAttachmentInfoKHR depthAttachment{};
        if (pDepthStencilView != nullptr)
        {
            auto* depthStencilView = SafeCast<DepthStencilViewVulKan>(pDepthStencilView);
            ThrowIf(depthStencilView == nullptr, "Depth stencil view is not a Vulkan depth stencil view");
            ThrowIf(depthStencilView->GetImageView() == VK_NULL_HANDLE, "Depth stencil image view is null");

            if (renderWidth == 0 || renderHeight == 0)
            {
                renderWidth = depthStencilView->GetWidth();
                renderHeight = depthStencilView->GetHeight();
            }
            else
            {
                ThrowIf(renderWidth != depthStencilView->GetWidth() || renderHeight != depthStencilView->GetHeight(),
                    "Depth stencil dimensions must match the render targets");
            }

            depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
            depthAttachment.imageView = depthStencilView->GetImageView();
            depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        }

        if (numRenderTargets == 0 && pDepthStencilView == nullptr)
        {
            return;
        }

        ThrowIf(renderWidth == 0 || renderHeight == 0, "Dynamic rendering attachments have an empty extent");
        VkRenderingInfoKHR renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        renderingInfo.renderArea.offset = { 0, 0 };
        renderingInfo.renderArea.extent = { renderWidth, renderHeight };
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
        renderingInfo.pColorAttachments = colorAttachments.data();
        if (pDepthStencilView != nullptr)
        {
            renderingInfo.pDepthAttachment = &depthAttachment;
        }

        m_CurrentRenderArea = renderingInfo.renderArea;
        BeginDynamicRendering(renderingInfo);
    }

    void CommandListVulKan::BeginDynamicRendering(const VkRenderingInfoKHR& renderingInfo)
    {
        ThrowIf(m_IsDynamicRendering, "Dynamic rendering is already active");
        ThrowIf(m_pfnCmdBeginRenderingKHR == nullptr, "vkCmdBeginRenderingKHR was not loaded");
        // Start dynamic rendering
        m_pfnCmdBeginRenderingKHR(m_CommandBuffer, &renderingInfo);
        m_IsDynamicRendering = true;
    }

    void CommandListVulKan::EndDynamicRendering()
    {
        ThrowIf(!m_IsDynamicRendering, "Dynamic rendering isn't activated, so it can't finish.");
        ThrowIf(m_pfnCmdEndRenderingKHR == nullptr, "vkCmdEndRenderingKHR was not loaded");
        // Stop dynamic rendering (if in active mode)
        m_pfnCmdEndRenderingKHR(m_CommandBuffer);
        m_IsDynamicRendering = false;
    }

    void CommandListVulKan::OMSetBlendState(RHIBlendState* pState, const float* blendFactor, uint32_t sampleMask)
    {
        // TODO: 实现混合状态设置
    }

    void CommandListVulKan::OMSetDepthStencilState(RHIDepthStencilState* pState, uint32_t stencilRef)
    {
        // TODO: 实现深度模板状态设置
    }
}
