#include "RHICommandListDirectX12.h"
#include "SwapChainDirectX12.h"

namespace RHI
{
    void CommandListDirectX12::OMSetRenderTargets(uint32_t numRenderTargets, RHIRenderTargetView* const* ppViews, bool RTsSingleHandleToDescriptorRange, RHIDepthStencilView* pDepthStencilView)
    {
        #define MAX_RENDER_TARGETS 8 // 最大渲染目标数量
        ThrowIf(numRenderTargets > MAX_RENDER_TARGETS, "numRenderTargets must be less than or equal to 8");
        
        std::vector<RenderTargetViewDirectX12*> pRTViews(numRenderTargets);
        for (uint32_t i = 0; i < numRenderTargets; ++i)
        {
            RenderTargetViewDirectX12* pRTView = SafeCast<RenderTargetViewDirectX12>(ppViews[i]);
            pRTViews[i] = pRTView;
        }

        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> pRTHandles(numRenderTargets);
        for (uint32_t i = 0; i < numRenderTargets; ++i)
        {
            pRTHandles[i] = *pRTViews[i]->GetCPUDescriptorHandle();
        }

        DepthStencilViewDirectX12* pDepthStencilViewDx12 = SafeCast<DepthStencilViewDirectX12>(pDepthStencilView);
        const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle = nullptr;
        if(pDepthStencilViewDx12!=nullptr)
            pDSVHandle = pDepthStencilViewDx12->GetCPUDescriptorHandle(); // 获取深度模板视图的 CPU 描述符句柄
        
        m_pCommandList->OMSetRenderTargets(numRenderTargets, pRTHandles.data(), RTsSingleHandleToDescriptorRange, pDSVHandle);
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
