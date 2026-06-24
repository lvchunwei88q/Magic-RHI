#include "RHICommandListD3D11.h"

namespace RHI
{
    void CommandListD3D11::ClearRenderTargetView(RHIRenderTargetView* pView, const float* colorRGBA)
    {
        // Get the command allocator
        CommandAllocatorD3D11* dx11CmdAllocator = GetAllocator();
        RenderTargetViewD3D11* pRTView = SafeCast<RenderTargetViewD3D11>(pView);
        ThrowIf(pRTView == nullptr, "Render target view is null");
        
        ID3D11RenderTargetView* pRTV = pRTView->GetRTV();
        dx11CmdAllocator->GetDeviceContext()->ClearRenderTargetView(pRTV, colorRGBA);
    }

    void CommandListD3D11::ClearDepthStencilView(RHIDepthStencilView* pView, RHIClearFlags clearFlags, float depth, uint8_t stencil)
    {
        // TODO: 实现深度模板清除
    }
}
