#pragma once
#include <RHIPipelineState.h>

namespace RHI
{
    class RHIPipelineStateDirectX11 : public RHIPipelineState
    {
    public:
        RHIPipelineStateDirectX11();
        ~RHIPipelineStateDirectX11() override;

        bool Initialize(Device* device, const GraphicsPipelineStateDesc& desc) override;
        bool Initialize(Device* device, const ComputePipelineStateDesc& desc) override;
        void Shutdown() override;

        bool IsValid() const override;
        PipelineStateType GetType() const override;

    private:
        PipelineStateType Type = PipelineStateType::Unknown;
        // TODO: Pipeline state data
    };
}
