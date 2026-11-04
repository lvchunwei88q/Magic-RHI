#include "RHICommandListVulKan.h"

namespace RHI
{
    void CommandListVulKan::RSSetViewports(uint32_t numViewports, const RHIViewport* pViewports)
    {
        ThrowIf(numViewports == 0 || pViewports == nullptr, "RSSetViewports: empty viewport array");

        std::vector<VkViewport> viewports(numViewports);
        for (uint32_t i = 0; i < numViewports; ++i)
        {
            // D3D's NDC y goes up, Vulkan's goes down: moving the origin down by a height and giving it a negative height is equivalent to flipping the y-axis.
            viewports[i].x        = pViewports[i].topLeftX;
            viewports[i].y        = pViewports[i].topLeftY + pViewports[i].height;
            viewports[i].width    = pViewports[i].width;
            viewports[i].height   = -pViewports[i].height;
            viewports[i].minDepth = pViewports[i].minDepth;
            viewports[i].maxDepth = pViewports[i].maxDepth;
        }
        vkCmdSetViewport(m_CommandBuffer, 0, numViewports, viewports.data());
    }

    void CommandListVulKan::RSSetScissorRects(uint32_t numRects, const RHIRect* pRects)
    {
        ThrowIf(numRects == 0 || pRects == nullptr, "RSSetScissorRects: empty rect array");

        std::vector<VkRect2D> rects(numRects);
        for (uint32_t i = 0; i < numRects; ++i)
        {
            const int32_t w = static_cast<int32_t>(pRects[i].right - pRects[i].left);
            const int32_t h = static_cast<int32_t>(pRects[i].bottom - pRects[i].top);
            rects[i].offset = {static_cast<int32_t>(pRects[i].left), static_cast<int32_t>(pRects[i].top)};
            rects[i].extent = {w > 0 ? static_cast<uint32_t>(w) : 0u,
                               h > 0 ? static_cast<uint32_t>(h) : 0u};
        }
        vkCmdSetScissor(m_CommandBuffer, 0, numRects, rects.data());
    }
}
