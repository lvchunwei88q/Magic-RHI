#include "RHICommandListDirectX11.h"

namespace RHI
{
    void CommandListDirectX11::OMSetRenderTargets(uint32_t numRenderTargets, RHIRenderTargetView* const* ppViews, bool RTsSingleHandleToDescriptorRange, RHIDepthStencilView* pDepthStencilView)
    {
        std::vector<ID3D11RenderTargetView*> pRTViews(numRenderTargets);
        for (uint32_t i = 0; i < numRenderTargets && i < 8; i++)
        {
            if (ppViews[i])
            {
                auto* pRTV11 = static_cast<RenderTargetViewDirectX11*>(ppViews[i]);
                pRTViews[i] = pRTV11->GetRTV();
            }
        }

        ID3D11DepthStencilView* pDSV = nullptr;
        if (pDepthStencilView)
        {
            auto* pDSV11 = static_cast<DepthStencilViewDirectX11*>(pDepthStencilView);
            pDSV = pDSV11->GetDSV();
        }

        // DX 11 不支持 RTsSingleHandleToDescriptorRange
        m_pDeviceContext->OMSetRenderTargets(numRenderTargets, pRTViews.data(), pDSV);
    }

    void CommandListDirectX11::OMSetBlendState(RHIBlendState* pState, const float* blendFactor, uint32_t sampleMask)
    {
        // TODO: 实现混合状态设置
    }

    void CommandListDirectX11::OMSetDepthStencilState(RHIDepthStencilState* pState, uint32_t stencilRef)
    {
        // TODO: 实现深度模板状态设置
    }
}
