#include "RHICommandListDirectX12.h"

namespace RHI
{
    void CommandListDirectX12::OMSetRenderTargets(uint32_t numRenderTargets, RHIRenderTargetView* const* ppViews, RHIDepthStencilView* pDepthStencilView)
    {
        // TODO: 实现渲染目标设置
    }

    void CommandListDirectX12::OMSetBlendState(RHIBlendState* pState, const float* blendFactor, uint32_t sampleMask)
    {
        // TODO: 实现混合状态设置
    }

    void CommandListDirectX12::OMSetDepthStencilState(RHIDepthStencilState* pState, uint32_t stencilRef)
    {
        // TODO: 实现深度模板状态设置
    }
}
