#include "RHICommandListDirectX11.h"
#include "RHIPipelineStateDirectX11.h"

namespace RHI
{
    namespace
    {
        void SetGraphicsPipelineState(ID3D11DeviceContext* pDeviceContext,const GPSDDirectX11& desc)
        {
            // TODO Set Graphics Pipeline State
            pDeviceContext->IASetInputLayout(desc.pInputLayout.Get());
            pDeviceContext->VSSetShader(desc.pVertexShader.Get(),0,0);
            if(desc.pPixelShader)
                pDeviceContext->PSSetShader(desc.pPixelShader.Get(),0,0);
            if(desc.pGeometryShader)
                pDeviceContext->GSSetShader(desc.pGeometryShader.Get(),0,0);
            if(desc.pHullShader)
                pDeviceContext->HSSetShader(desc.pHullShader.Get(),0,0);
            if(desc.pDomainShader)
                pDeviceContext->DSSetShader(desc.pDomainShader.Get(),0,0);
            pDeviceContext->RSSetState(desc.pRasterizerState.Get());
            pDeviceContext->OMSetBlendState(desc.pBlendState.Get(),nullptr,0xFFFFFFFF);
            pDeviceContext->OMSetDepthStencilState(desc.pDepthStencilState.Get(),0); // TODO Set StencilRef
        }
        void SetComputePipelineState(ID3D11DeviceContext* pDeviceContext,const CPSDDirectX11& desc)
        {
            // TODO Set Compute Pipeline State
            pDeviceContext->CSSetShader(desc.pComputeShader.Get(),0,0);
        }
    }
    void CommandListDirectX11::SetPipelineState(RHIPipelineState* pPipelineState,PipelineStateType stateType)
    {
        if (!pPipelineState)
        {
#if RHI_ENABLE_RESOURCE_DEBUG_INFO
            ThrowErrorMessage("This PSO is empty");
#endif
            return;
        }

        if(stateType != pPipelineState->GetType()){
#if RHI_ENABLE_RESOURCE_DEBUG_INFO
            ThrowErrorMessage("This PSO is not as expected");
#endif
            return;
        }
            

        auto pso = SafeCast<RHIPipelineStateDirectX11>(pPipelineState);
        if (!pso->IsValid())
        {
#if RHI_ENABLE_RESOURCE_DEBUG_INFO
            ThrowErrorMessage("This PSO cannot be converted");
#endif
            return;           
        }

        // TODO Set Pipeline State
        if(stateType == PipelineStateType::Graphics){
            SetGraphicsPipelineState(m_pDeviceContext,pso->GetGraphicsDesc());
        }
        else if(stateType == PipelineStateType::Compute){
            SetComputePipelineState(m_pDeviceContext,pso->GetComputeDesc());
        }
    }
}
