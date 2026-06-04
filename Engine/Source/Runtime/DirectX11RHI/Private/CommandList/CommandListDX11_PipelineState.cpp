#include "RHICommandListDirectX11.h"
#include "RHIPipelineStateDirectX11.h"

namespace RHI
{
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
            

        auto pso = static_cast<RHIPipelineStateDirectX11*>(pPipelineState);
        if (!pso->IsValid())
        {
#if RHI_ENABLE_RESOURCE_DEBUG_INFO
            ThrowErrorMessage("This PSO cannot be converted");
#endif
            return;           
        }

        // TODO Set Pipeline State
    }
}
