#include "RHICommandListDirectX11.h"

namespace RHI
{
    void CommandListDirectX11::SetPipelineState(RHIPipelineState* pPipelineState,PipelineStateType stateType)
    {
        if(stateType != pPipelineState->GetType()){
#if RHI_ENABLE_RESOURCE_DEBUG_INFO
            ThrowErrorMessage("This PSO is not as expected");
#endif
            return;
        }
    }
}
