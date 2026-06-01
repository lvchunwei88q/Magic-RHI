
#include "RHIDirectX12.h"
#include "RHIResourceDirectX12.h"
#include "RHIPipelineStateDirectX12.h"
#include "RHIRootSignatureDirectX12.h"

namespace RHI
{
    namespace
    {
        D3D12_INPUT_CLASSIFICATION ConvertInputClassification(InputClassification classification)
        {
            switch (classification)
            {
            case InputClassification::PerVertexData:
                return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            case InputClassification::PerInstanceData:
                return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
            default:
                return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            }
        }
        
        D3D12_BLEND_DESC ConvertBlendState(RHIBlendState* blendState)
        {
            D3D12_BLEND_DESC desc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            if (blendState)
            {
                // TODO: Blend state data initialization
            }
            return desc;
        }

        D3D12_RASTERIZER_DESC ConvertRasterizerState(RHIRasterizerState* rasterizerState)
        {
            D3D12_RASTERIZER_DESC desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            if (rasterizerState)
            {
                // TODO: Rasterizer state data initialization
            }
            return desc;
        }

        D3D12_DEPTH_STENCIL_DESC ConvertDepthStencilState(RHIDepthStencilState* depthStencilState)
        {
            D3D12_DEPTH_STENCIL_DESC desc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
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
        
        D3D12_INPUT_ELEMENT_DESC ConvertInputLayout(const InputElementDesc& desc)
        {
            D3D12_INPUT_ELEMENT_DESC d3dDesc = {};
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

    RHIPipelineStateDirectX12::RHIPipelineStateDirectX12()
    {
    }

    RHIPipelineStateDirectX12::~RHIPipelineStateDirectX12()
    {
        Shutdown();
    }

    bool RHIPipelineStateDirectX12::Initialize(Device* device, const GraphicsPipelineStateDesc& desc)
    {
        if (!device || !desc.pRootSignature || !desc.pVertexShader || !desc.pPixelShader)
            return false;

        auto dx12Device = static_cast<RHIDirectX12*>(device);
        auto d3dDevice = dx12Device->GetDevice();
        if (!d3dDevice)
            return false;

        auto rootSig = static_cast<RHIRootSignatureDirectX12*>(desc.pRootSignature);
        if (!rootSig->GetRootSignature())
            return false;

        auto vsShader = static_cast<VertexShaderDirectX12*>(desc.pVertexShader);
        auto psShader = static_cast<PixelShaderDirectX12*>(desc.pPixelShader);
        
        std::vector<D3D12_INPUT_ELEMENT_DESC> d3d12Elements;
        d3d12Elements.reserve(desc.NumInputElements); 
        for (int i = 0; i < desc.NumInputElements; i++)
        {
            auto& inputElement = desc.pInputElementDesc[i];
            d3d12Elements.push_back(RHI::ConvertInputLayout(inputElement ));
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = rootSig->GetRootSignature();
        psoDesc.InputLayout = {d3d12Elements.data(),static_cast<UINT>(d3d12Elements.size()) };
        psoDesc.VS.pShaderBytecode = vsShader->GetBytecode().data();
        psoDesc.VS.BytecodeLength = static_cast<UINT>(vsShader->GetBytecode().size());
        psoDesc.PS.pShaderBytecode = psShader->GetBytecode().data();
        psoDesc.PS.BytecodeLength = static_cast<UINT>(psShader->GetBytecode().size());

        if (desc.pGeometryShader)
        {
            auto gsShader = static_cast<GeometryShaderDirectX12*>(desc.pGeometryShader);
            psoDesc.GS.pShaderBytecode = gsShader->GetBytecode().data();
            psoDesc.GS.BytecodeLength = static_cast<UINT>(gsShader->GetBytecode().size());
        }

        if (desc.pHullShader)
        {
            auto hsShader = static_cast<HullShaderDirectX12*>(desc.pHullShader);
            psoDesc.HS.pShaderBytecode = hsShader->GetBytecode().data();
            psoDesc.HS.BytecodeLength = static_cast<UINT>(hsShader->GetBytecode().size());
        }

        if (desc.pDomainShader)
        {
            auto dsShader = static_cast<DomainShaderDirectX12*>(desc.pDomainShader);
            psoDesc.DS.pShaderBytecode = dsShader->GetBytecode().data();
            psoDesc.DS.BytecodeLength = static_cast<UINT>(dsShader->GetBytecode().size());
        }

        psoDesc.BlendState = ConvertBlendState(desc.pBlendState);
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.RasterizerState = ConvertRasterizerState(desc.pRasterizerState);
        psoDesc.DepthStencilState = ConvertDepthStencilState(desc.pDepthStencilState);
        psoDesc.DSVFormat = ConvertFormat(desc.DepthStencilFormat);

        psoDesc.NumRenderTargets = desc.NumRenderTargets;
        for (UINT i = 0; i < desc.NumRenderTargets; ++i)
        {
            psoDesc.RTVFormats[i] = ConvertFormat(desc.RenderTargetFormats[i]);
        }

        // TODO: Primitive topology type data initialization
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;
        psoDesc.NodeMask = 0;

        HRESULT hr = d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState));
        if (FAILED(hr))
            return false;

        Type = PipelineStateType::Graphics;
        return true;
    }

    bool RHIPipelineStateDirectX12::Initialize(Device* device, const ComputePipelineStateDesc& desc)
    {
        if (!device || !desc.pRootSignature || !desc.pComputeShader)
            return false;

        auto dx12Device = static_cast<RHIDirectX12*>(device);
        auto d3dDevice = dx12Device->GetDevice();
        if (!d3dDevice)
            return false;

        auto rootSig = static_cast<RHIRootSignatureDirectX12*>(desc.pRootSignature);
        if (!rootSig->GetRootSignature())
            return false;

        auto csShader = static_cast<ComputeShaderDirectX12*>(desc.pComputeShader);

        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = rootSig->GetRootSignature();
        psoDesc.CS.pShaderBytecode = csShader->GetBytecode().data();
        psoDesc.CS.BytecodeLength = static_cast<UINT>(csShader->GetBytecode().size());
        psoDesc.NodeMask = 0;

        ThrowIfFailed(d3dDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState)));

        Type = PipelineStateType::Compute;
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
