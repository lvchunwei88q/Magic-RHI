
#include <Common/Check.h>
#include "RHIDirectX11.h"
#include "RHIPipelineStateDirectX11.h"
#include "RHIResourceDirectX11.h"

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
            // 先给一个默认值，后续再修改
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            if (blendState)
            {
                // TODO: Blend state data initialization
            }
            return desc;
        }

        D3D11_RASTERIZER_DESC ConvertRasterizerState(RHIRasterizerState* rasterizerState)
        {
            D3D11_RASTERIZER_DESC desc = {};
            // 先给一个默认值，后续再修改
            desc.FillMode = D3D11_FILL_SOLID;
            desc.CullMode = D3D11_CULL_BACK;
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

            // 32-bit 3 components
            case RHITextureFormat::R32G32B32_UINT:     return DXGI_FORMAT_R32G32B32_UINT;
            case RHITextureFormat::R32G32B32_SINT:     return DXGI_FORMAT_R32G32B32_SINT;
            case RHITextureFormat::R32G32B32_FLOAT:    return DXGI_FORMAT_R32G32B32_FLOAT;
                
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

    bool RHIPipelineStateDirectX11::Initialize(Device* device, const GraphicsPipelineStateDesc& desc)
    {
        Type = PipelineStateType::Graphics;
        RHIDirectX11* dx11direct = SafeCast<RHIDirectX11>(device);
        // TODO: Pipeline state data initialization
        //GraphicsDesc.pRootSignature = desc.pRootSignature;

        VertexShaderDirectX11* pVShader = SafeCast<VertexShaderDirectX11>(desc.pVertexShader);
        ComPtr<ID3D11VertexShader> VShader = pVShader->GetShader();
        ComPtr<ID3DBlob> pVSBlob = pVShader->GetVSBlob();

        ComPtr<ID3D11PixelShader> PShader;
        if(desc.pPixelShader != nullptr)
        {
            PixelShaderDirectX11* pPShader = SafeCast<PixelShaderDirectX11>(desc.pPixelShader);
            PShader = pPShader->GetShader();
        }

        ComPtr<ID3D11GeometryShader> GShader;
        if(desc.pGeometryShader != nullptr)
        {
            GeometryShaderDirectX11* pGShader = SafeCast<GeometryShaderDirectX11>(desc.pGeometryShader);
            GShader = pGShader->GetShader();
        }

        ComPtr<ID3D11HullShader> HShader;
        if(desc.pHullShader != nullptr)
        {
            HullShaderDirectX11* pHShader = SafeCast<HullShaderDirectX11>(desc.pHullShader);
            HShader = pHShader->GetShader();
        }

        ComPtr<ID3D11DomainShader> DShader;
        if(desc.pDomainShader != nullptr)
        {
            DomainShaderDirectX11* pDShader = SafeCast<DomainShaderDirectX11>(desc.pDomainShader);
            DShader = pDShader->GetShader();
        }

        std::vector<D3D11_INPUT_ELEMENT_DESC> d3d11Elements;
        d3d11Elements.reserve(desc.NumInputElements);
        for (uint32_t i = 0; i < desc.NumInputElements; i++)
        {
            auto& inputElement = desc.pInputElementDesc[i];
            d3d11Elements.push_back(ConvertInputLayout(inputElement));
        }

        ThrowIfFailed(dx11direct->GetDevice()->CreateInputLayout(
            d3d11Elements.data(),
            static_cast<UINT>(d3d11Elements.size()),
            pVSBlob->GetBufferPointer(),   // VS 字节码（用于验证）
            pVSBlob->GetBufferSize(),
            GraphicsDesc.pInputLayout.GetAddressOf()
        ));


        ComPtr<ID3D11RasterizerState> pRasterizerState;
        D3D11_RASTERIZER_DESC rasterizerDesc = ConvertRasterizerState(desc.pRasterizerState);
        ThrowIfFailed(dx11direct->GetDevice()->CreateRasterizerState(&rasterizerDesc, pRasterizerState.GetAddressOf()));

        ComPtr<ID3D11BlendState> pBlendState;
        D3D11_BLEND_DESC blendDesc = ConvertBlendState(desc.pBlendState);
        ThrowIfFailed(dx11direct->GetDevice()->CreateBlendState(&blendDesc, pBlendState.GetAddressOf()));

        ComPtr<ID3D11DepthStencilState> pDepthStencilState;
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = ConvertDepthStencilState(desc.pDepthStencilState);
        ThrowIfFailed(dx11direct->GetDevice()->CreateDepthStencilState(&depthStencilDesc, pDepthStencilState.GetAddressOf()));

        GraphicsDesc.pVertexShader = VShader;
        GraphicsDesc.pPixelShader = PShader;
        GraphicsDesc.pGeometryShader = GShader;
        GraphicsDesc.pHullShader = HShader;
        GraphicsDesc.pDomainShader = DShader;
        GraphicsDesc.pRasterizerState = pRasterizerState;
        GraphicsDesc.pBlendState = pBlendState;
        GraphicsDesc.pDepthStencilState = pDepthStencilState;
        GraphicsDesc.NumRenderTargets = desc.NumRenderTargets;
        // 复制数组
        for (uint32_t i = 0; i < 8; ++i)
        {
            GraphicsDesc.RenderTargetFormats[i] = ToDXGIFormat(desc.RenderTargetFormats[i]);
        }
        GraphicsDesc.DepthStencilFormat = ToDXGIFormat(desc.DepthStencilFormat);
        return true;
    }

    bool RHIPipelineStateDirectX11::Initialize(Device* device, const ComputePipelineStateDesc& desc)
    {
        Type = PipelineStateType::Compute;
        RHIDirectX11* dx11direct = SafeCast<RHIDirectX11>(device);
        // TODO: Pipeline state data initialization
        //ComputeDesc.pRootSignature = desc.pRootSignature;
        
        ComputeShaderDirectX11* pCShader = SafeCast<ComputeShaderDirectX11>(desc.pComputeShader);
        ComPtr<ID3D11ComputeShader> CShader = pCShader->GetShader();

        ComputeDesc.pComputeShader = CShader;
        return true;
    }

    void RHIPipelineStateDirectX11::Shutdown()
    {
    }

    bool RHIPipelineStateDirectX11::IsValid() const
    {
        return Type != PipelineStateType::Unknown;
    }

    PipelineStateType RHIPipelineStateDirectX11::GetType() const
    {
        return Type;
    }

    const GPSDDirectX11& RHIPipelineStateDirectX11::GetGraphicsDesc() const
    {
        return GraphicsDesc;
    }

    const CPSDDirectX11& RHIPipelineStateDirectX11::GetComputeDesc() const
    {
        return ComputeDesc;
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
