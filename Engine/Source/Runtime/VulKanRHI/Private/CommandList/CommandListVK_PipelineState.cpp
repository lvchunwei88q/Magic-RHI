#include "RHICommandListVulKan.h"

namespace RHI
{
    void CommandListVulKan::SetPipelineState(RHIPipelineState* pPipelineState,PipelineStateType stateType)
    {
        if (!pPipelineState)
        {
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("This PSO is empty");
#endif
            return;
        }

        if(stateType != pPipelineState->GetType()){
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("This PSO is not as expected");
#endif
            return;
        }
        
        RHIPipelineStateVulkan* pso = SafeCast<RHIPipelineStateVulkan>(pPipelineState);
        if (!pso || !pso->IsValid())
        {
            ThrowErrorMessage("SetPipelineState: PSO is not a valid RHIPipelineStateVulkan");
            return;
        }

        if (stateType == PipelineStateType::Graphics)
        {
            vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pso->GetPipeline());
            m_pCurrentGraphicsPSO = pso;
        }
        else if (stateType == PipelineStateType::Compute)
        {
            vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pso->GetPipeline());
            m_pCurrentComputePSO = pso;
        }
    }
}
