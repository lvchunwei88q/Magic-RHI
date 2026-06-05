#include "RHICommandListDirectX11.h"

namespace RHI
{
    void CommandListDirectX11::ClearRenderTargetView(RHIRenderTargetView* pView, const float* colorRGBA)
    {
        RenderTargetViewDirectX11* pRTView = SafeCast<RenderTargetViewDirectX11>(pView);
        ThrowIf(pRTView == nullptr, "Render target view is null");
        
        ID3D11RenderTargetView* pRTV = pRTView->GetRTV();
        m_pDeviceContext->ClearRenderTargetView(pRTV, colorRGBA);
    }

    void CommandListDirectX11::ClearDepthStencilView(RHIDepthStencilView* pView, RHIClearFlags clearFlags, float depth, uint8_t stencil)
    {
        // TODO: 实现深度模板清除
    }
}
