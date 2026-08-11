#pragma once

#include <RHIPipelineState.h>
#include <vulkan.h>

namespace RHI
{
    class RHIPipelineStateVulkan : public RHIPipelineState
    {
    public:
        RHIPipelineStateVulkan();
        ~RHIPipelineStateVulkan();

        bool Initialize(Device* device, const GraphicsPipelineStateDesc& desc) override;
        bool Initialize(Device* device, const ComputePipelineStateDesc& desc) override;
        void Shutdown() override;

        bool IsValid() const override;
        PipelineStateType GetType() const override;

    private:
        PipelineStateType Type = PipelineStateType::Unknown;
        VkPipeline m_Pipeline;
    };

} // namespace RHI
