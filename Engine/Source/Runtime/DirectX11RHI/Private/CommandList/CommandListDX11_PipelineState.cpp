#include "RHICommandListD3D11.h"
#include "RHIPipelineStateD3D11.h"

namespace RHI
{
    // We directly use a static function to set up the pipeline state -> pso
    namespace
    {
        // Set graphics pipeline state
        void SetGraphicsPipelineState(ID3D11DeviceContext* pDeviceContext,const GPSDD3D11& desc)
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

        // Set compute pipeline state
        void SetComputePipelineState(ID3D11DeviceContext* pDeviceContext,const CPSDD3D11& desc)
        {
            // TODO Set Compute Pipeline State
            pDeviceContext->CSSetShader(desc.pComputeShader.Get(),0,0);
        }
    }
    void CommandListD3D11::SetPipelineState(RHIPipelineState* pPipelineState,PipelineStateType stateType)
    {
        // Check pipeline state
        if (!pPipelineState)
        {
#if RHI_ENABLE_RESOURCE_DEBUG_INFO
            ThrowErrorMessage("This PSO is empty");
#endif
            return;
        }

        // Check pipeline state type
        if(stateType != pPipelineState->GetType()){
#if RHI_ENABLE_RESOURCE_DEBUG_INFO
            ThrowErrorMessage("This PSO is not as expected");
#endif
            return;
        }
            
        // Check pipeline state validity
        auto pso = SafeCast<RHIPipelineStateD3D11>(pPipelineState);
        if (!pso->IsValid())
        {
#if RHI_ENABLE_RESOURCE_DEBUG_INFO
            ThrowErrorMessage("This PSO cannot be converted");
#endif
            return;           
        }

        // Get device context
        CommandAllocatorD3D11* dx11CmdAllocator = GetAllocator();
        ID3D11DeviceContext* pDeviceContext = dx11CmdAllocator->GetDeviceContext();

        // Set Pipeline state
        if(stateType == PipelineStateType::Graphics){
            SetGraphicsPipelineState(pDeviceContext,pso->GetGraphicsDesc());
        }
        else if(stateType == PipelineStateType::Compute){
            SetComputePipelineState(pDeviceContext,pso->GetComputeDesc());
        }
    }
}
