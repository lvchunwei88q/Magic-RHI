#include "RHICommandListDirectX12.h"

namespace RHI
{
    void CommandListDirectX12::ClearRenderTargetView(RHIRenderTargetView* pView, const float* colorRGBA)
    {
        RenderTargetViewDirectX12* pRTView = SafeCast<RenderTargetViewDirectX12>(pView);
        ThrowIf(pRTView == nullptr, "Render target view is null");
    
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = *pRTView->GetCPUDescriptorHandle();
        m_pCommandList->ClearRenderTargetView(rtvHandle, colorRGBA, 0, nullptr);
    }

    void CommandListDirectX12::ClearDepthStencilView(RHIDepthStencilView* pView, RHIClearFlags clearFlags, float depth, uint8_t stencil)
    {
        // TODO: 实现深度模板清除
    }
}
