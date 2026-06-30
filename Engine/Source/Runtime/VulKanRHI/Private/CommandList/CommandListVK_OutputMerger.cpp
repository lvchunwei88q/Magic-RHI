#include "RHICommandListVulKan.h"

namespace RHI
{
    void CommandListVulKan::OMSetRenderTargets(uint32_t numRenderTargets, RHIRenderTargetView* const* ppViews, bool RTsSingleHandleToDescriptorRange, RHIDepthStencilView* pDepthStencilView)
    {
        ThrowIf(numRenderTargets > DRAW_MAX_RENDER_TARGETS, "numRenderTargets must be less than or equal to DRAW_MAX_RENDER_TARGETS");
        // TODO: 实现渲染目标设置
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
