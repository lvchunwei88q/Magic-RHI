
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

        DXGI_FORMAT ToDXGIFormat(RHITextureFormat Format)
        {
            switch (Format)
            {
            // Unknown
            case RHITextureFormat::Unknown:
                return DXGI_FORMAT_UNKNOWN;
                
            // 8-bit single component
            case RHITextureFormat::R8_UNORM:    return DXGI_FORMAT_R8_UNORM;
            case RHITextureFormat::R8_SNORM:    return DXGI_FORMAT_R8_SNORM;
            case RHITextureFormat::R8_UINT:     return DXGI_FORMAT_R8_UINT;
            case RHITextureFormat::R8_SINT:     return DXGI_FORMAT_R8_SINT;
            case RHITextureFormat::R8_SRGB:     return DXGI_FORMAT_R8_UNORM; // 注意：R8 没有 SRGB 变体
                
            // 16-bit single component
            case RHITextureFormat::R16_UNORM:   return DXGI_FORMAT_R16_UNORM;
            case RHITextureFormat::R16_SNORM:   return DXGI_FORMAT_R16_SNORM;
            case RHITextureFormat::R16_UINT:    return DXGI_FORMAT_R16_UINT;
            case RHITextureFormat::R16_SINT:    return DXGI_FORMAT_R16_SINT;
            case RHITextureFormat::R16_FLOAT:   return DXGI_FORMAT_R16_FLOAT;
                
            // 8-bit 2 components
            case RHITextureFormat::R8G8_UNORM:  return DXGI_FORMAT_R8G8_UNORM;
            case RHITextureFormat::R8G8_SNORM:  return DXGI_FORMAT_R8G8_SNORM;
            case RHITextureFormat::R8G8_UINT:   return DXGI_FORMAT_R8G8_UINT;
            case RHITextureFormat::R8G8_SINT:   return DXGI_FORMAT_R8G8_SINT;
            case RHITextureFormat::R8G8_SRGB:   return DXGI_FORMAT_R8G8_UNORM;
                
            // 16-bit 2 components
            case RHITextureFormat::R16G16_UNORM:    return DXGI_FORMAT_R16G16_UNORM;
            case RHITextureFormat::R16G16_SNORM:    return DXGI_FORMAT_R16G16_SNORM;
            case RHITextureFormat::R16G16_UINT:     return DXGI_FORMAT_R16G16_UINT;
            case RHITextureFormat::R16G16_SINT:     return DXGI_FORMAT_R16G16_SINT;
            case RHITextureFormat::R16G16_FLOAT:    return DXGI_FORMAT_R16G16_FLOAT;
                
            // 32-bit 2 components
            case RHITextureFormat::R32G32_UINT:     return DXGI_FORMAT_R32G32_UINT;
            case RHITextureFormat::R32G32_SINT:     return DXGI_FORMAT_R32G32_SINT;
            case RHITextureFormat::R32G32_FLOAT:    return DXGI_FORMAT_R32G32_FLOAT;
                
            // 8-bit 4 components
            case RHITextureFormat::R8G8B8A8_UNORM:  return DXGI_FORMAT_R8G8B8A8_UNORM;
            case RHITextureFormat::R8G8B8A8_SNORM:  return DXGI_FORMAT_R8G8B8A8_SNORM;
            case RHITextureFormat::R8G8B8A8_UINT:   return DXGI_FORMAT_R8G8B8A8_UINT;
            case RHITextureFormat::R8G8B8A8_SINT:   return DXGI_FORMAT_R8G8B8A8_SINT;
            case RHITextureFormat::R8G8B8A8_SRGB:   return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                
            // 16-bit 4 components
            case RHITextureFormat::R16G16B16A16_UNORM:  return DXGI_FORMAT_R16G16B16A16_UNORM;
            case RHITextureFormat::R16G16B16A16_SNORM:  return DXGI_FORMAT_R16G16B16A16_SNORM;
            case RHITextureFormat::R16G16B16A16_UINT:   return DXGI_FORMAT_R16G16B16A16_UINT;
            case RHITextureFormat::R16G16B16A16_SINT:   return DXGI_FORMAT_R16G16B16A16_SINT;
            case RHITextureFormat::R16G16B16A16_FLOAT:  return DXGI_FORMAT_R16G16B16A16_FLOAT;
                
            // 32-bit 4 components
            case RHITextureFormat::R32G32B32A32_UINT:   return DXGI_FORMAT_R32G32B32A32_UINT;
            case RHITextureFormat::R32G32B32A32_SINT:   return DXGI_FORMAT_R32G32B32A32_SINT;
            case RHITextureFormat::R32G32B32A32_FLOAT:  return DXGI_FORMAT_R32G32B32A32_FLOAT;
                
            // Packed formats
            case RHITextureFormat::R10G10B10A2_UNORM:   return DXGI_FORMAT_R10G10B10A2_UNORM;
            case RHITextureFormat::R10G10B10A2_UINT:    return DXGI_FORMAT_R10G10B10A2_UINT;
            case RHITextureFormat::R11G11B10_FLOAT:     return DXGI_FORMAT_R11G11B10_FLOAT;
            case RHITextureFormat::B5G6R5_UNORM:        return DXGI_FORMAT_B5G6R5_UNORM;
            case RHITextureFormat::B5G5R5A1_UNORM:      return DXGI_FORMAT_B5G5R5A1_UNORM;
            case RHITextureFormat::B4G4R4A4_UNORM:      return DXGI_FORMAT_B4G4R4A4_UNORM;
                
            // BGRA formats
            case RHITextureFormat::B8G8R8A8_UNORM:      return DXGI_FORMAT_B8G8R8A8_UNORM;
            case RHITextureFormat::B8G8R8A8_SRGB:       return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            case RHITextureFormat::B8G8R8X8_UNORM:      return DXGI_FORMAT_B8G8R8X8_UNORM;
            case RHITextureFormat::B8G8R8X8_SRGB:       return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
                
            // Depth formats
            case RHITextureFormat::D16_UNORM:           return DXGI_FORMAT_D16_UNORM;
            case RHITextureFormat::D24_UNORM_S8_UINT:   return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case RHITextureFormat::D32_FLOAT:           return DXGI_FORMAT_D32_FLOAT;
            case RHITextureFormat::D32_FLOAT_S8X24_UINT:return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
                
            // BC compressed formats
            case RHITextureFormat::BC1_UNORM:           return DXGI_FORMAT_BC1_UNORM;
            case RHITextureFormat::BC1_UNORM_SRGB:      return DXGI_FORMAT_BC1_UNORM_SRGB;
            case RHITextureFormat::BC2_UNORM:           return DXGI_FORMAT_BC2_UNORM;
            case RHITextureFormat::BC2_UNORM_SRGB:      return DXGI_FORMAT_BC2_UNORM_SRGB;
            case RHITextureFormat::BC3_UNORM:           return DXGI_FORMAT_BC3_UNORM;
            case RHITextureFormat::BC3_UNORM_SRGB:      return DXGI_FORMAT_BC3_UNORM_SRGB;
            case RHITextureFormat::BC4_UNORM:           return DXGI_FORMAT_BC4_UNORM;
            case RHITextureFormat::BC4_SNORM:           return DXGI_FORMAT_BC4_SNORM;
            case RHITextureFormat::BC5_UNORM:           return DXGI_FORMAT_BC5_UNORM;
            case RHITextureFormat::BC5_SNORM:           return DXGI_FORMAT_BC5_SNORM;
            case RHITextureFormat::BC6H_UF16:           return DXGI_FORMAT_BC6H_UF16;
            case RHITextureFormat::BC6H_SF16:           return DXGI_FORMAT_BC6H_SF16;
            case RHITextureFormat::BC7_UNORM:           return DXGI_FORMAT_BC7_UNORM;
            case RHITextureFormat::BC7_UNORM_SRGB:      return DXGI_FORMAT_BC7_UNORM_SRGB;
                
            // Typeless formats
            case RHITextureFormat::R8G8B8A8_TYPELESS:       return DXGI_FORMAT_R8G8B8A8_TYPELESS;
            case RHITextureFormat::R16G16B16A16_TYPELESS:   return DXGI_FORMAT_R16G16B16A16_TYPELESS;
            case RHITextureFormat::R32G32B32A32_TYPELESS:   return DXGI_FORMAT_R32G32B32A32_TYPELESS;
            case RHITextureFormat::R32_TYPELESS:            return DXGI_FORMAT_R32_TYPELESS;
                
            default:
                return DXGI_FORMAT_UNKNOWN;
            }
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
