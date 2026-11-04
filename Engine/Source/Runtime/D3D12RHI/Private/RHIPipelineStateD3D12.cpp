
#include "RHID3D12.h"
#include "RHIResourceD3D12.h"
#include "RHIPipelineStateD3D12.h"
#include "RHIRootSignatureD3D12.h"
#include "RHIFormatD3D12.h"

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

        D3D12_INPUT_ELEMENT_DESC ConvertInputLayout(const InputElementDesc& desc)
        {
            D3D12_INPUT_ELEMENT_DESC d3dDesc = {};
            d3dDesc.SemanticName = desc.SemanticName;
            d3dDesc.SemanticIndex = desc.SemanticIndex;
            d3dDesc.Format = ToDXGIFormat(desc.Format);
            d3dDesc.InputSlot = desc.InputSlot;
            d3dDesc.AlignedByteOffset = desc.AlignedByteOffset;
            d3dDesc.InputSlotClass = ConvertInputClassification(desc.InputSlotClass);
            d3dDesc.InstanceDataStepRate = desc.InstanceDataStepRate;
            return d3dDesc;
        }
    }

    RHIPipelineStateD3D12::RHIPipelineStateD3D12()
    {
    }

    RHIPipelineStateD3D12::~RHIPipelineStateD3D12()
    {
        Shutdown();
    }

    bool RHIPipelineStateD3D12::Initialize(Device* device, const GraphicsPipelineStateDesc& desc)
    {
        // check params
        if (!device || !desc.pRootSignature || !desc.pVertexShader || !desc.pPixelShader)
        {
            ThrowErrorMessage("RHIPipelineStateD3D12::Initialize: Device, RootSignature, VertexShader, or PixelShader is null");
            return false;
        }

        // check d3d12 device
        auto dx12Device = SafeCast<DeviceD3D12>(device);
        auto d3dDevice = dx12Device->GetDevice();
        if (!d3dDevice)
        {
            ThrowErrorMessage("RHIPipelineStateD3D12::Initialize: Device is null");
            return false;
        }

        // check root signature
        auto rootSig = SafeCast<RHIRootSignatureD3D12>(desc.pRootSignature);
        if (!rootSig->GetRootSignature())
        {
            ThrowErrorMessage("RHIPipelineStateD3D12::Initialize: RootSignature is not a RHIRootSignatureD3D12");
            return false;
        }

        // check vertex and pixel shader
        auto vsShader = SafeCast<VertexShaderD3D12>(desc.pVertexShader);
        auto psShader = SafeCast<PixelShaderD3D12>(desc.pPixelShader);
        if (!vsShader || !psShader)
        {
            ThrowErrorMessage("RHIPipelineStateD3D12::Initialize: VertexShader or PixelShader is not a VertexShaderD3D12 or PixelShaderD3D12");
            return false;
        }
        
        // convert input layout
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
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsShader->GetBytecode().data(), static_cast<UINT>(vsShader->GetBytecode().size()));
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(psShader->GetBytecode().data(), static_cast<UINT>(psShader->GetBytecode().size()));

        if (desc.pGeometryShader)
        {
            auto gsShader = SafeCast<GeometryShaderD3D12>(desc.pGeometryShader);
            psoDesc.GS = CD3DX12_SHADER_BYTECODE(gsShader->GetBytecode().data(), static_cast<UINT>(gsShader->GetBytecode().size()));
        }

        if (desc.pHullShader)
        {
            auto hsShader = SafeCast<HullShaderD3D12>(desc.pHullShader);
            psoDesc.HS = CD3DX12_SHADER_BYTECODE(hsShader->GetBytecode().data(), static_cast<UINT>(hsShader->GetBytecode().size()));
        }

        if (desc.pDomainShader)
        {
            auto dsShader = SafeCast<DomainShaderD3D12>(desc.pDomainShader);
            psoDesc.DS = CD3DX12_SHADER_BYTECODE(dsShader->GetBytecode().data(), static_cast<UINT>(dsShader->GetBytecode().size()));
        }

        psoDesc.BlendState = ConvertBlendState(desc.pBlendState);
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.RasterizerState = ConvertRasterizerState(desc.pRasterizerState);
        psoDesc.DepthStencilState = ConvertDepthStencilState(desc.pDepthStencilState);

        if (desc.DepthStencilFormat != RHITextureFormat::Unknown && !IsDepthFormat(desc.DepthStencilFormat))
            ThrowErrorMessage("DepthStencil format is not depth format");
        psoDesc.DSVFormat = ToDXGIFormat(desc.DepthStencilFormat);

        psoDesc.NumRenderTargets = desc.NumRenderTargets;
        for (UINT i = 0; i < desc.NumRenderTargets; ++i)
        {
            psoDesc.RTVFormats[i] = ToDXGIFormat(desc.RenderTargetFormats[i]);
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

    bool RHIPipelineStateD3D12::Initialize(Device* device, const ComputePipelineStateDesc& desc)
    {
        // check params
        if (!device || !desc.pRootSignature || !desc.pComputeShader)
            return false;

        // check d3d12 device
        auto dx12Device = SafeCast<DeviceD3D12>(device);
        auto d3dDevice = dx12Device->GetDevice();
        if (!d3dDevice)
        {
            ThrowErrorMessage("RHIPipelineStateD3D12::Initialize: Device is null");
            return false;
        }

        // check root signature
        auto rootSig = SafeCast<RHIRootSignatureD3D12>(desc.pRootSignature);
        if (!rootSig->GetRootSignature())
        {
            ThrowErrorMessage("RHIPipelineStateD3D12::Initialize: RootSignature is not a RHIRootSignatureD3D12");
            return false;
        }

        // check compute shader
        auto csShader = SafeCast<ComputeShaderD3D12>(desc.pComputeShader);
        if (!csShader)
        {
            ThrowErrorMessage("RHIPipelineStateD3D12::Initialize: ComputeShader is not a ComputeShaderD3D12");
            return false;
        }

        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = rootSig->GetRootSignature();
        psoDesc.CS.pShaderBytecode = csShader->GetBytecode().data();
        psoDesc.CS.BytecodeLength = static_cast<UINT>(csShader->GetBytecode().size());
        psoDesc.NodeMask = 0;

        ThrowIfFailed(d3dDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState)));

        Type = PipelineStateType::Compute;
        return true;
    }

    void RHIPipelineStateD3D12::Shutdown()
    {
        m_pPipelineState.Reset();
    }

    bool RHIPipelineStateD3D12::IsValid() const
    {
        return m_pPipelineState != nullptr;
    }

    PipelineStateType RHIPipelineStateD3D12::GetType() const
    {
        return Type;
    }

    // ============== DeviceD3D12 ==============
    std::shared_ptr<RHIPipelineState> DeviceD3D12::CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
    {
        auto pipelineState = std::make_shared<RHIPipelineStateD3D12>();
        if (pipelineState->Initialize(this, desc))
        {
            return pipelineState;
        }
        return nullptr;
    }

    std::shared_ptr<RHIPipelineState> DeviceD3D12::CreateComputePipelineState(const ComputePipelineStateDesc& desc)
    {
        auto pipelineState = std::make_shared<RHIPipelineStateD3D12>();
        if (pipelineState->Initialize(this, desc))
        {
            return pipelineState;
        }
        return nullptr;
    }

    void DeviceD3D12::DeletePipelineState(std::shared_ptr<RHIPipelineState>& pipelineState)
    {
        if (pipelineState)
        {
            auto dx12PipelineState = SafeCast<RHIPipelineStateD3D12>(pipelineState.get());
            if (dx12PipelineState)
            {
                dx12PipelineState->Shutdown();
            }
            pipelineState.reset();
        }
    }
}
