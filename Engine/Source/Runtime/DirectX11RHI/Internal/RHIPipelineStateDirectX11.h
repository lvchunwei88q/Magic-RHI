#pragma once
#include <RHIPipelineState.h>
#include <d3d11.h>
#include <Common/RHIException.h>

namespace RHI
{
    struct GPSDDirectX11
    {        
        InputElementDesc* pInputElementDesc = nullptr;
        uint32_t NumInputElements = 0; 
        
        RHIVertexShader* pVertexShader = nullptr;
        RHIPixelShader* pPixelShader = nullptr;
        RHIGeometryShader* pGeometryShader = nullptr;
        RHIHullShader* pHullShader = nullptr;
        RHIDomainShader* pDomainShader = nullptr;

        RHIRasterizerState* pRasterizerState = nullptr;
        RHIBlendState* pBlendState = nullptr;
        RHIDepthStencilState* pDepthStencilState = nullptr;

        uint32_t NumRenderTargets = 0;
        DXGI_FORMAT RenderTargetFormats[8] = {};
        DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_UNKNOWN;
    };

    struct CPSDDirectX11
    {
        RHIComputeShader* pComputeShader = nullptr;
    };

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
        // 这里存放的是DirectX11的管线状态数据
        GPSDDirectX11 GraphicsDesc;
        CPSDDirectX11 ComputeDesc;
    };
}
