#include "RHICommandListDirectX12.h"
#include "RHIPipelineStateDirectX12.h"

namespace RHI
{
    void CommandListDirectX12::SetPipelineState(RHIPipelineState* pPipelineState,PipelineStateType stateType)
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
            

        auto pso = static_cast<RHIPipelineStateDirectX12*>(pPipelineState);
        if (!pso->IsValid())
        {
#if RHI_ENABLE_RESOURCE_DEBUG_INFO
            ThrowErrorMessage("This PSO cannot be converted");
#endif
            return;           
        }

        m_pCommandList->SetPipelineState(pso->GetPipelineState());
    }
}
