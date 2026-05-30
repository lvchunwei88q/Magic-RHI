#pragma once
#include "Common/RHI_API.h"
#include <cstdint>

namespace RHI
{
    class Device; // forward declaration

    // for forward declaration of Resources
    class RHIRootSignature;
    class RHIComputeShader;

    class RHIRasterizerState;
    class RHIBlendState;
    class RHIDepthStencilState;
    
    class RHIDomainShader;
    class RHIHullShader;
    class RHIVertexShader;
    class RHIPixelShader;
    class RHIGeometryShader;


    struct GraphicsPipelineStateDesc
    {
        RHIRootSignature* pRootSignature = nullptr;
        
        RHIVertexShader* pVertexShader = nullptr;
        RHIPixelShader* pPixelShader = nullptr;
        RHIGeometryShader* pGeometryShader = nullptr;
        RHIHullShader* pHullShader = nullptr;
        RHIDomainShader* pDomainShader = nullptr;

        RHIRasterizerState* pRasterizerState = nullptr;
        RHIBlendState* pBlendState = nullptr;
        RHIDepthStencilState* pDepthStencilState = nullptr;

        uint32_t NumRenderTargets = 0;
        uint32_t RenderTargetFormats[8] = {0};
        uint32_t DepthStencilFormat = 0;
    };

    struct ComputePipelineStateDesc
    {
        RHIRootSignature* pRootSignature = nullptr;
        RHIComputeShader* pComputeShader = nullptr;
    };

    enum class PipelineStateType
    {
        Graphics,
        Compute,

        Unknown,
    };

    class RHI_API RHIPipelineState
    {
    public:
        RHIPipelineState();
        virtual ~RHIPipelineState();
        virtual bool Initialize(Device* device, const GraphicsPipelineStateDesc& desc) = 0;
        virtual bool Initialize(Device* device, const ComputePipelineStateDesc& desc) = 0;
        virtual void Shutdown() = 0;

        virtual bool IsValid() const = 0;
        virtual PipelineStateType GetType() const = 0;
    };
}
