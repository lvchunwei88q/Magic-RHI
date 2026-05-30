
#include "RHIDirectX11.h"
#include "RHIPipelineStateDirectX11.h"

namespace RHI
{
    RHIPipelineStateDirectX11::RHIPipelineStateDirectX11()
    {
    }

    RHIPipelineStateDirectX11::~RHIPipelineStateDirectX11()
    {
        Shutdown();
    }

    bool RHIPipelineStateDirectX11::Initialize(Device* /*device*/, const GraphicsPipelineStateDesc& /*desc*/)
    {
        Type = PipelineStateType::Graphics;
        // TODO: Pipeline state data initialization
        return true;
    }

    bool RHIPipelineStateDirectX11::Initialize(Device* /*device*/, const ComputePipelineStateDesc& /*desc*/)
    {
        Type = PipelineStateType::Compute;
        // TODO: Pipeline state data initialization
        return true;
    }

    void RHIPipelineStateDirectX11::Shutdown()
    {
    }

    bool RHIPipelineStateDirectX11::IsValid() const
    {
        return true;
    }

    PipelineStateType RHIPipelineStateDirectX11::GetType() const
    {
        return Type;
    }

    std::shared_ptr<RHIPipelineState> RHIDirectX11::CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
    {
        auto pipelineState = std::make_shared<RHIPipelineStateDirectX11>();
        if (pipelineState->Initialize(this, desc))
        {
            return pipelineState;
        }
        return nullptr;
    }

    std::shared_ptr<RHIPipelineState> RHIDirectX11::CreateComputePipelineState(const ComputePipelineStateDesc& desc)
    {
        auto pipelineState = std::make_shared<RHIPipelineStateDirectX11>();
        if (pipelineState->Initialize(this, desc))
        {
            return pipelineState;
        }
        return nullptr;
    }

    void RHIDirectX11::DeletePipelineState(std::shared_ptr<RHIPipelineState>& pipelineState)
    {
        if (pipelineState)
        {
            auto dx11PipelineState = static_cast<RHIPipelineStateDirectX11*>(pipelineState.get());
            if (dx11PipelineState)
            {
                dx11PipelineState->Shutdown();
            }
            pipelineState.reset();
        }
    }
}
