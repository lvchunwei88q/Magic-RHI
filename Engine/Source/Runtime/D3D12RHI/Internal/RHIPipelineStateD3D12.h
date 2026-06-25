#pragma once

#include <RHIPipelineState.h>
#include <d3d12.h>
#include <d3dx12.h>

#include <wrl.h>
using Microsoft::WRL::ComPtr;

namespace RHI
{
    class RHIPipelineStateD3D12 : public RHIPipelineState
    {
    public:
        RHIPipelineStateD3D12();
        ~RHIPipelineStateD3D12() override;

        bool Initialize(Device* device, const GraphicsPipelineStateDesc& desc) override;
        bool Initialize(Device* device, const ComputePipelineStateDesc& desc) override;
        void Shutdown() override;

        bool IsValid() const override;
        PipelineStateType GetType() const override;

        ID3D12PipelineState* GetPipelineState() const { return m_pPipelineState.Get(); }

    private:
        PipelineStateType Type = PipelineStateType::Unknown;
        ComPtr<ID3D12PipelineState> m_pPipelineState;
    };
}
