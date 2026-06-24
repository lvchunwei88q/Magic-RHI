#pragma once
#include <RHIPipelineState.h>
#include <d3d11.h>
#include <Common/Check.h>

#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    /*
    * @brief 图形管线状态描述
    * 由于DX11不支持所谓PSO，所以这里直接将管线状态描述存储在这里结构体中，在CommandListDX11_PipelineState.cpp中直接
    * 调用D3D11DeviceContext::SetPipelineState()设置管线状态。
    * 初始化时根据描述创建对应的DX11管线状态对象。
    */
    struct GPSDD3D11
    {        
        ComPtr<ID3D11InputLayout> pInputLayout;
        
        ComPtr<ID3D11VertexShader> pVertexShader;
        ComPtr<ID3D11PixelShader> pPixelShader;
        ComPtr<ID3D11GeometryShader> pGeometryShader;
        ComPtr<ID3D11HullShader> pHullShader;
        ComPtr<ID3D11DomainShader> pDomainShader;

        ComPtr<ID3D11RasterizerState> pRasterizerState;
        ComPtr<ID3D11BlendState> pBlendState;
        ComPtr<ID3D11DepthStencilState> pDepthStencilState;

        uint32_t NumRenderTargets = 0;
        DXGI_FORMAT RenderTargetFormats[8] = {};
        DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_UNKNOWN;
    };

    struct CPSDD3D11
    {
        ComPtr<ID3D11ComputeShader> pComputeShader;
    };

    class RHIPipelineStateD3D11 : public RHIPipelineState
    {
    public:
        RHIPipelineStateD3D11();
        ~RHIPipelineStateD3D11() override;

        bool Initialize(Device* device, const GraphicsPipelineStateDesc& desc) override;
        bool Initialize(Device* device, const ComputePipelineStateDesc& desc) override;
        void Shutdown() override;

        bool IsValid() const override;
        PipelineStateType GetType() const override;

        const GPSDD3D11& GetGraphicsDesc() const;
        const CPSDD3D11& GetComputeDesc() const;
    private:
        PipelineStateType Type = PipelineStateType::Unknown;
        // 这里存放的是D3D11的管线状态数据
        GPSDD3D11 GraphicsDesc;
        CPSDD3D11 ComputeDesc;
    };
}
