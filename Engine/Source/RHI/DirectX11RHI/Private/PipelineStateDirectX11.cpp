
#include "RHIDirectX11.h"
#include "RHIPipelineStateDirectX11.h"

namespace RHI
{
    namespace
    {
        D3D11_INPUT_CLASSIFICATION ConvertInputClassification(InputClassification classification)
        {
            switch (classification)
            {
            case InputClassification::PerVertexData:
                return D3D11_INPUT_PER_VERTEX_DATA;
            case InputClassification::PerInstanceData:
                return D3D11_INPUT_PER_INSTANCE_DATA;
            default:
                return D3D11_INPUT_PER_VERTEX_DATA;
            }
        }
        
        D3D11_BLEND_DESC ConvertBlendState(RHIBlendState* blendState)
        {
            D3D11_BLEND_DESC desc = {};
            if (blendState)
            {
                // TODO: Blend state data initialization
            }
            return desc;
        }

        D3D11_RASTERIZER_DESC ConvertRasterizerState(RHIRasterizerState* rasterizerState)
        {
            D3D11_RASTERIZER_DESC desc = {};
            if (rasterizerState)
            {
                // TODO: Rasterizer state data initialization
            }
            return desc;
        }

        D3D11_DEPTH_STENCIL_DESC ConvertDepthStencilState(RHIDepthStencilState* depthStencilState)
        {
            D3D11_DEPTH_STENCIL_DESC desc ={};
            if (depthStencilState)
            {
                // TODO: Depth stencil state data initialization
            }
            return desc;
        }

        DXGI_FORMAT ConvertFormat(uint32_t format)
        {
            if (format == 0)
                return DXGI_FORMAT_UNKNOWN;

            // TODO: Format data initialization
            return static_cast<DXGI_FORMAT>(format);
        }
        
        D3D11_INPUT_ELEMENT_DESC ConvertInputLayout(const InputElementDesc& desc)
        {
            D3D11_INPUT_ELEMENT_DESC d3dDesc = {};
            d3dDesc.SemanticName = desc.SemanticName;
            d3dDesc.SemanticIndex = desc.SemanticIndex;
            d3dDesc.Format = ConvertFormat(desc.Format);
            d3dDesc.InputSlot = desc.InputSlot;
            d3dDesc.AlignedByteOffset = desc.AlignedByteOffset;
            d3dDesc.InputSlotClass = ConvertInputClassification(desc.InputSlotClass);
            d3dDesc.InstanceDataStepRate = desc.InstanceDataStepRate;
            return d3dDesc;
        }
    }
    
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
