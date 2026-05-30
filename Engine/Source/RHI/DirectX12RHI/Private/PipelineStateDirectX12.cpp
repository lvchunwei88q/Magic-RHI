
#include "RHIDirectX12.h"
#include "RHIPipelineStateDirectX12.h"

namespace RHI
{
    RHIPipelineStateDirectX12::RHIPipelineStateDirectX12()
    {
    }

    RHIPipelineStateDirectX12::~RHIPipelineStateDirectX12()
    {
        Shutdown();
    }

    bool RHIPipelineStateDirectX12::Initialize(Device* /*device*/, const GraphicsPipelineStateDesc& /*desc*/)
    {
        Type = PipelineStateType::Graphics;
        // TODO: Pipeline state data initialization
        return true;
    }

    bool RHIPipelineStateDirectX12::Initialize(Device* /*device*/, const ComputePipelineStateDesc& /*desc*/)
    {
        Type = PipelineStateType::Compute;
        // TODO: Pipeline state data initialization
        return true;
    }

    void RHIPipelineStateDirectX12::Shutdown()
    {
        m_pPipelineState.Reset();
    }

    bool RHIPipelineStateDirectX12::IsValid() const
    {
        return m_pPipelineState != nullptr;
    }

    PipelineStateType RHIPipelineStateDirectX12::GetType() const
    {
        return Type;
    }

    std::shared_ptr<RHIPipelineState> RHIDirectX12::CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
    {
        auto pipelineState = std::make_shared<RHIPipelineStateDirectX12>();
        if (pipelineState->Initialize(this, desc))
        {
            return pipelineState;
        }
        return nullptr;
    }

    std::shared_ptr<RHIPipelineState> RHIDirectX12::CreateComputePipelineState(const ComputePipelineStateDesc& desc)
    {
        auto pipelineState = std::make_shared<RHIPipelineStateDirectX12>();
        if (pipelineState->Initialize(this, desc))
        {
            return pipelineState;
        }
        return nullptr;
    }

    void RHIDirectX12::DeletePipelineState(std::shared_ptr<RHIPipelineState>& pipelineState)
    {
        if (pipelineState)
        {
            auto dx12PipelineState = static_cast<RHIPipelineStateDirectX12*>(pipelineState.get());
            if (dx12PipelineState)
            {
                dx12PipelineState->Shutdown();
            }
            pipelineState.reset();
        }
    }
}
