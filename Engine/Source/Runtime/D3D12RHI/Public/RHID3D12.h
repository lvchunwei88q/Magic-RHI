#pragma once

#include "Common/D3D12RHI_API.h"
#include <RHIInterface.h>
#include <d3d12.h>
#include <dxgi1_6.h>

// DescriptorHeap
#include "DescriptorHeapD3D12.h"

#include <vector>
#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    // CommandQueue Forward Declaration
    class CommandQueueD3D12;
    using GraphicsCommandQueueD3D12 = CommandQueueD3D12;
    using ComputeCommandQueueD3D12 = CommandQueueD3D12;
    using CopyCommandQueueD3D12 = CommandQueueD3D12;

    class D3D12RHI_API DeviceD3D12 : public Device
    {
    public:
        DeviceD3D12();
        ~DeviceD3D12() override;

        bool Initialize() override;
        void Shutdown() override;
        bool IsValid() const override;
        
        RHIType GetType() const override { return RHIType::D3D12; }
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
        [[nodiscard]] RHICommandQueue* GetCommandQueue(RHICmdType Type) const override;

        void CreateQueues();

        [[nodiscard]] std::shared_ptr<RHIRootSignature> CreateRootSignature(const RootSignatureDesc& desc) override;
        void DeleteRootSignature(std::shared_ptr<RHI::RHIRootSignature>& rootSignature) override;

        [[nodiscard]] std::shared_ptr<RHIPipelineState> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIPipelineState> CreateComputePipelineState(const ComputePipelineStateDesc& desc) override;
        void DeletePipelineState(std::shared_ptr<RHIPipelineState>& pipelineState) override;

        [[nodiscard]] RHIDescriptorHeap* GetDescriptorHeap(RHIDescriptorHeapType type) const override;
        
        FeatureLevel GetFeatureLevel() const override;
        ID3D12Device* GetDevice() const { return m_pDevice.Get(); }
        IDXGIAdapter1* GetAdapter() const { return m_pAdapter.Get(); }

    private:
        ComPtr<ID3D12Device> m_pDevice;
        std::wstring m_AdapterName;
        D3D_FEATURE_LEVEL m_FeatureLevel;
        
        ComPtr<IDXGIAdapter1> m_pAdapter; // GPU

        // CommandQueue
        std::unique_ptr<GraphicsCommandQueueD3D12> m_GraphicsQueue;
        std::unique_ptr<ComputeCommandQueueD3D12> m_ComputeQueue;
        std::unique_ptr<CopyCommandQueueD3D12> m_CopyQueue;

        std::unique_ptr<DescriptorHeapD3D12> m_pStandardHeap;
        std::unique_ptr<DescriptorHeapD3D12> m_pSamplerHeap;
        std::unique_ptr<DescriptorHeapD3D12> m_pRTVHeap;
        std::unique_ptr<DescriptorHeapD3D12> m_pDSVHeap;
    };

    class D3D12RHI_API SwapChainD3D12 : public SwapChain
    {
    public:
        SwapChainD3D12();
        ~SwapChainD3D12() override;

        bool Initialize(Device* device, const SwapChainDesc& desc) override;
        void Shutdown() override;
        bool IsValid() const override;

        void Present(uint32_t syncInterval, uint32_t presentFlags) override;
        void Resize(uint32_t width, uint32_t height) override;
        uint32_t GetFrameIndex() const override { return m_pSwapChain3->GetCurrentBackBufferIndex(); }// 获取当前帧索引

        uint32_t GetWidth() const override { return m_desc.Width; }
        uint32_t GetHeight() const override { return m_desc.Height; }
        // Get render target view
        RHIRenderTargetView* GetRenderTargetView(uint32_t index) const override;
        // Get back buffer
        RHITexture* GetBackBuffer(uint32_t index) const override;

        IDXGISwapChain3* GetSwapChain3() const { return m_pSwapChain3.Get(); }
        IDXGISwapChain1* GetSwapChain() const { return m_pSwapChain1.Get(); }

    private:
        DeviceD3D12* m_pRHI;
        
        void CreateRTVs();

        ComPtr<IDXGISwapChain3> m_pSwapChain3;
        ComPtr<IDXGISwapChain1> m_pSwapChain1;
        ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
        std::unique_ptr<RHITexture> m_pBackBuffers[RHI_MULTI_BUFFERING];
        std::unique_ptr<RHIRenderTargetView> m_pRenderTargetViews[RHI_MULTI_BUFFERING];

        SwapChainDesc m_desc;
    };

    class D3D12RHI_API CreateShaderD3D12 : public CreateShader
    {
    public:
        CreateShaderD3D12() {};
        ~CreateShaderD3D12() override { Shutdown();  };
        bool Initialize(Device* device) override;
        void Shutdown() override;
        bool IsValid() const override;
        
        [[nodiscard]] std::unique_ptr<RHIVertexShader> CreateVertexShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIPixelShader> CreatePixelShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIGeometryShader> CreateGeometryShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIHullShader> CreateHullShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIDomainShader> CreateDomainShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIComputeShader> CreateComputeShader(const CreateShaderDesc& desc) override;
        ShaderModelVersion GetShaderModelVersion() const override;

    private:
        DeviceD3D12* m_pRHI;
    };
    
}
