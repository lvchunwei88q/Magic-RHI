#include "RHICommandListD3D11.h"

namespace RHI
{
    void CommandListD3D11::OMSetRenderTargets(uint32_t numRenderTargets, RHIRenderTargetView* const* ppViews, bool RTsSingleHandleToDescriptorRange, RHIDepthStencilView* pDepthStencilView)
    {
        #define MAX_RENDER_TARGETS 8 // 最大渲染目标数量
        ThrowIf(numRenderTargets > MAX_RENDER_TARGETS, "numRenderTargets must be less than or equal to 8");
        
        std::vector<ID3D11RenderTargetView*> pRTViews(numRenderTargets);
        for (uint32_t i = 0; i < numRenderTargets; i++)
        {
            if (ppViews[i])
            {
                auto* pRTV11 = static_cast<RenderTargetViewD3D11*>(ppViews[i]);
                pRTViews[i] = pRTV11->GetRTV();
            }
        }

        ID3D11DepthStencilView* pDSV = nullptr;
        if (pDepthStencilView)
        {
            auto* pDSV11 = static_cast<DepthStencilViewD3D11*>(pDepthStencilView);
            pDSV = pDSV11->GetDSV();
        }

        // DX 11 不支持 RTsSingleHandleToDescriptorRange
        m_pDeviceContext->OMSetRenderTargets(numRenderTargets, pRTViews.data(), pDSV);
    }

    void CommandListD3D11::OMSetBlendState(RHIBlendState* pState, const float* blendFactor, uint32_t sampleMask)
    {
        // TODO: 实现混合状态设置
    }

    void CommandListD3D11::OMSetDepthStencilState(RHIDepthStencilState* pState, uint32_t stencilRef)
    {
        // TODO: 实现深度模板状态设置
    }
}
