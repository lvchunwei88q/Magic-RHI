#include "RHIPipelineStateVulkan.h"
#include "RHIVulKan.h"

namespace RHI
{
    namespace
    {

    }

    RHIPipelineStateVulkan::RHIPipelineStateVulkan()
    {
    }
    RHIPipelineStateVulkan::~RHIPipelineStateVulkan()
    {
        Shutdown();
    }

    bool RHIPipelineStateVulkan::Initialize(Device* device, const GraphicsPipelineStateDesc& desc)
    {
        Type = PipelineStateType::Graphics;
        return false;
    }
    bool RHIPipelineStateVulkan::Initialize(Device* device, const ComputePipelineStateDesc& desc)
    {
        Type = PipelineStateType::Compute;
        return false;
    }

    void RHIPipelineStateVulkan::Shutdown()
    {
    }

    bool RHIPipelineStateVulkan::IsValid() const
    {
        return false;
    }

    PipelineStateType RHIPipelineStateVulkan::GetType() const
    {
        return Type;
    }

    // ============== DeviceVulKan ==============
    std::shared_ptr<RHIPipelineState> DeviceVulKan::CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
    {
        auto pipelineState = std::make_shared<RHIPipelineStateVulkan>();
        if (pipelineState->Initialize(this, desc))
        {
            return pipelineState;
        }
        return nullptr;
    }

    std::shared_ptr<RHIPipelineState> DeviceVulKan::CreateComputePipelineState(const ComputePipelineStateDesc& desc)
    {
        auto pipelineState = std::make_shared<RHIPipelineStateVulkan>();
        if (pipelineState->Initialize(this, desc))
        {
            return pipelineState;
        }
        return nullptr;
    }

    void DeviceVulKan::DeletePipelineState(std::shared_ptr<RHIPipelineState>& pipelineState)
    {
        if (pipelineState)
        {
            auto vulkanPipelineState = SafeCast<RHIPipelineStateVulkan>(pipelineState.get());
            if (vulkanPipelineState)
            {
                vulkanPipelineState->Shutdown();
            }
            pipelineState.reset();
        }
    }

} // namespace RHI
