
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
        
        D3D11_INPUT_ELEMENT_DESC ConvertInputLayout(const InputElementDesc& desc)
        {
            D3D11_INPUT_ELEMENT_DESC d3dDesc = {};
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
    
    RHIPipelineStateDirectX11::RHIPipelineStateDirectX11()
    {
    }

    RHIPipelineStateDirectX11::~RHIPipelineStateDirectX11()
    {
        Shutdown();
    }

    bool RHIPipelineStateDirectX11::Initialize(Device* /*device*/, const GraphicsPipelineStateDesc& desc)
    {
        Type = PipelineStateType::Graphics;
        // TODO: Pipeline state data initialization
        //GraphicsDesc.pRootSignature = desc.pRootSignature;
        GraphicsDesc.pInputElementDesc = desc.pInputElementDesc;
        GraphicsDesc.NumInputElements = desc.NumInputElements;
        GraphicsDesc.pVertexShader = desc.pVertexShader;
        GraphicsDesc.pPixelShader = desc.pPixelShader;
        GraphicsDesc.pGeometryShader = desc.pGeometryShader;
        GraphicsDesc.pHullShader = desc.pHullShader;
        GraphicsDesc.pDomainShader = desc.pDomainShader;
        GraphicsDesc.pRasterizerState = desc.pRasterizerState;
        GraphicsDesc.pBlendState = desc.pBlendState;
        GraphicsDesc.pDepthStencilState = desc.pDepthStencilState;
        GraphicsDesc.NumRenderTargets = desc.NumRenderTargets;
        // 复制数组
        for (uint32_t i = 0; i < 8; ++i)
        {
            GraphicsDesc.RenderTargetFormats[i] = ToDXGIFormat(desc.RenderTargetFormats[i]);
        }
        GraphicsDesc.DepthStencilFormat = ToDXGIFormat(desc.DepthStencilFormat);
        return true;
    }

    bool RHIPipelineStateDirectX11::Initialize(Device* /*device*/, const ComputePipelineStateDesc& desc)
    {
        Type = PipelineStateType::Compute;
        // TODO: Pipeline state data initialization
        //ComputeDesc.pRootSignature = desc.pRootSignature;
        ComputeDesc.pComputeShader = desc.pComputeShader;
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
