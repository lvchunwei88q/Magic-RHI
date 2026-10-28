#pragma once
#include "Common/RHI_API.h"
#include "Common/RHIDefinitions.h"
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

    /**
     * @brief
     */
    struct GraphicsPipelineStateDesc
    {
        RHIRootSignature* pRootSignature = nullptr;
        
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
        RHITextureFormat RenderTargetFormats[8] = {};
        RHITextureFormat DepthStencilFormat = RHITextureFormat::Unknown;
    };

    /**
     * @brief
     */
    struct ComputePipelineStateDesc
    {
        RHIRootSignature* pRootSignature = nullptr;
        RHIComputeShader* pComputeShader = nullptr;
    };

    /**
     * @brief
     */
    enum class PipelineStateType
    {
        Graphics,
        Compute,

        Unknown,
    };

    class RHI_API RHIPipelineState
    {
    public:
        /**
         * @brief
         */
        RHIPipelineState();
        /**
         * @brief
         */
        virtual ~RHIPipelineState();
        /**
         * @brief
         * @param device
         * @param desc
         * @return
         */
        virtual bool Initialize(Device* device, const GraphicsPipelineStateDesc& desc) = 0;
        /**
         * @brief
         * @param device
         * @param desc
         * @return
         */
        virtual bool Initialize(Device* device, const ComputePipelineStateDesc& desc) = 0;
        /**
         * @brief
         */
        virtual void Shutdown() = 0;

        /**
         * @brief
         * @return
         */
        virtual bool IsValid() const = 0;
        /**
         * @brief
         * @return
         */
        virtual PipelineStateType GetType() const = 0;
    };
}
