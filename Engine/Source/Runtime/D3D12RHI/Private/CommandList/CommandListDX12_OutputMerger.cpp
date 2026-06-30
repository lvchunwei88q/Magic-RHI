#include "DirectXConfig.h"
#include "RHICommandListD3D12.h"

namespace RHI
{
    void CommandListD3D12::OMSetRenderTargets(uint32_t numRenderTargets, RHIRenderTargetView* const* ppViews, bool RTsSingleHandleToDescriptorRange, RHIDepthStencilView* pDepthStencilView)
    {
        ThrowIf(numRenderTargets > DRAW_MAX_RENDER_TARGETS, "numRenderTargets must be less than or equal to DRAW_MAX_RENDER_TARGETS");
        
        std::vector<RenderTargetViewD3D12*> pRTViews(numRenderTargets);
        for (uint32_t i = 0; i < numRenderTargets; ++i)
        {
            RenderTargetViewD3D12* pRTView = SafeCast<RenderTargetViewD3D12>(ppViews[i]);
            pRTViews[i] = pRTView;
        }

        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> pRTHandles(numRenderTargets);
        for (uint32_t i = 0; i < numRenderTargets; ++i)
        {
            pRTHandles[i] = *pRTViews[i]->GetCPUDescriptorHandle();
        }

        DepthStencilViewD3D12* pDepthStencilViewDx12 = SafeCast<DepthStencilViewD3D12>(pDepthStencilView);
        const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle = nullptr;
        if(pDepthStencilViewDx12!=nullptr)
            // Get the CPU descriptor handle for the depth stencil view.
            pDSVHandle = pDepthStencilViewDx12->GetCPUDescriptorHandle(); 
        
        m_pCommandList->OMSetRenderTargets(numRenderTargets, pRTHandles.data(), RTsSingleHandleToDescriptorRange, pDSVHandle);
    }

    void CommandListD3D12::OMSetBlendState(RHIBlendState* pState, const float* blendFactor, uint32_t sampleMask)
    {
        // TODO: 实现混合状态设置
    }

    void CommandListD3D12::OMSetDepthStencilState(RHIDepthStencilState* pState, uint32_t stencilRef)
    {
        // TODO: 实现深度模板状态设置
    }
}
