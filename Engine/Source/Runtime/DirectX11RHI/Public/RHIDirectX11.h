#pragma once

#include "Common/DIRECTX11RHI_API.h"
#include <RHI.h>
#include <d3d11.h>
#include <dxgi.h>

// DescriptorHeap
#include "DescriptorHeapDirectX11.h"

#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    // CommandQueue Forward Declaration
    class CommandQueueDirectX11;

    class DIRECTX11RHI_API RHIDirectX11 : public Device
    {
    public:
        RHIDirectX11();
        ~RHIDirectX11() override;

        bool Initialize() override;
        void Shutdown() override;

        bool IsValid() const override;
        RHIType GetType() const override { return RHIType::DirectX11; }
        const std::wstring& GetAdapterName() const override { return m_AdapterName; }

        [[nodiscard]] std::shared_ptr<RHISamplerState> CreateSamplerState(const SamplerStateDesc& desc) override;
        void DeleteSamplerState(std::shared_ptr<RHI::RHISamplerState>& samplerState) override;

        [[nodiscard]] std::shared_ptr<RHIBuffer> CreateBuffer(BufferDesc& desc) override;
        void DeleteBuffer(std::shared_ptr<RHI::RHIBuffer>& buffer) override;
        
        RHIDescriptorHandle CreateStandardHeapDescriptorView(RHIBuffer* Buffer,DescriptorRangeType Type) override;
        RHIDescriptorHandle CreateStandardHeapDescriptorView(RHITexture* Texture,DescriptorRangeType Type) override;
        RHIDescriptorHandle CreateSamplerHeapDescriptorView(const SamplerStateDesc& desc) override;
        RHIDescriptorHandle CreateRTVHeapDescriptorView(RHIRenderTargetView* InView) override;
        RHIDescriptorHandle CreateDSVHeapDescriptorView(RHIDepthStencilView* InView) override;

        [[nodiscard]] std::shared_ptr<RHICommandAllocator> CreateCommandAllocator(RHICmdType type) override;
        [[nodiscard]] std::shared_ptr<RHICommandList> CreateCommandList(std::shared_ptr<RHICommandAllocator>& allocator) override;
        [[nodiscard]] std::shared_ptr<RHICommandQueue> GetCommandQueue(RHICmdType Type) const override;

        [[nodiscard]] std::shared_ptr<RHIVertexShader> CompileVertexShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIPixelShader> CompilePixelShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIGeometryShader> CompileGeometryShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIHullShader> CompileHullShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIDomainShader> CompileDomainShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIComputeShader> CompileComputeShader(const ShaderCompileDesc& desc) override;
        
        [[nodiscard]] std::shared_ptr<RHIRootSignature> CreateRootSignature(const RootSignatureDesc& desc) override;
        void DeleteRootSignature(std::shared_ptr<RHI::RHIRootSignature>& rootSignature) override;

        [[nodiscard]] std::shared_ptr<RHIPipelineState> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIPipelineState> CreateComputePipelineState(const ComputePipelineStateDesc& desc) override;
        void DeletePipelineState(std::shared_ptr<RHIPipelineState>& pipelineState) override;

        [[nodiscard]] std::shared_ptr<RHICommandQueue> GetGraphicsQueue() const override;
        [[nodiscard]] std::shared_ptr<RHICommandQueue> GetComputeQueue() const override;
        [[nodiscard]] std::shared_ptr<RHICommandQueue> GetCopyQueue() const override;

        FeatureLevel GetFeatureLevel() const override;
        ID3D11Device* GetDevice() const { return m_pDevice.Get(); }
        ID3D11DeviceContext* GetDeviceContext() const { return m_pDeviceContext.Get(); }

    private:
        ComPtr<ID3D11Device> m_pDevice;
        ComPtr<ID3D11DeviceContext> m_pDeviceContext;
        std::wstring m_AdapterName;
        D3D_FEATURE_LEVEL m_FeatureLevel;
        // CommandQueue DirectX11
        std::shared_ptr<CommandQueueDirectX11> m_CommandQueue;

        /*
         * DescriptorHeap DirectX11 模拟
         * 包含标准堆、采样器堆、RTV堆、DSV堆
        */
        std::unique_ptr<DescriptorHeapDirectX11> m_pStandardHeap;
        std::unique_ptr<DescriptorHeapDirectX11> m_pSamplerHeap;
        std::unique_ptr<DescriptorHeapDirectX11> m_pRTVHeap;
        std::unique_ptr<DescriptorHeapDirectX11> m_pDSVHeap;
    };
}
