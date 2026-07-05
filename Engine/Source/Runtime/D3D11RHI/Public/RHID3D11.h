#pragma once

#include "Common/D3D11RHI_API.h"
#include <RHIInterface.h>
#include <d3d11.h>
#include <dxgi.h>

// DescriptorHeap
#include "DescriptorHeapD3D11.h"

#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    // CommandQueue Forward Declaration
    class CommandQueueD3D11;

    class D3D11RHI_API DeviceD3D11 : public Device
    {
    public:
        DeviceD3D11();
        ~DeviceD3D11() override;

        bool Initialize() override;
        void Shutdown() override;
        bool IsValid() const override;

        RHIType GetType() const override { return RHIType::D3D11; }
        const std::wstring& GetAdapterName() const override { return m_AdapterName; }

        [[nodiscard]] std::shared_ptr<RHISamplerState> CreateSamplerState(const SamplerStateDesc& desc) override;
        void DeleteSamplerState(std::shared_ptr<RHI::RHISamplerState>& samplerState) override;

        [[nodiscard]] std::shared_ptr<RHIBuffer> CreateBuffer(BufferDesc& desc) override;
        void DeleteBuffer(std::shared_ptr<RHI::RHIBuffer>& buffer) override;
        
        RHIDescriptorHandle CreateStandardHeapDescriptorView(RHIBuffer* Buffer,DescriptorRangeType Type) override;
        RHIDescriptorHandle CreateStandardHeapDescriptorView(RHITexture* Texture,DescriptorRangeType Type) override;
        RHIDescriptorHandle CreateSamplerHeapDescriptorView(const SamplerStateDesc& desc) override;
        RHIDescriptorHandle CreateRTVHeapDescriptorView(RHITexture* Texture) override;
        RHIDescriptorHandle CreateDSVHeapDescriptorView(RHITexture* Texture) override;

        [[nodiscard]] std::unique_ptr<RHIVertexShader> CreateVertexShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIPixelShader> CreatePixelShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIGeometryShader> CreateGeometryShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIHullShader> CreateHullShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIDomainShader> CreateDomainShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIComputeShader> CreateComputeShader(const CreateShaderDesc& desc) override;
        ShaderModelVersion GetShaderModelVersion() const override;

        [[nodiscard]] std::shared_ptr<RHICommandAllocator> CreateCommandAllocator(RHICmdType type) override;
        [[nodiscard]] std::shared_ptr<RHICommandList> CreateCommandList(std::shared_ptr<RHICommandAllocator>& allocator) override;
        [[nodiscard]] RHICommandQueue* GetCommandQueue(RHICmdType Type) const override;
        
        [[nodiscard]] std::shared_ptr<RHIRootSignature> CreateRootSignature(const RootSignatureDesc& desc) override;
        void DeleteRootSignature(std::shared_ptr<RHI::RHIRootSignature>& rootSignature) override;

        [[nodiscard]] std::shared_ptr<RHIPipelineState> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIPipelineState> CreateComputePipelineState(const ComputePipelineStateDesc& desc) override;
        void DeletePipelineState(std::shared_ptr<RHIPipelineState>& pipelineState) override;

        [[nodiscard]] RHIDescriptorHeap* GetDescriptorHeap(RHIDescriptorHeapType type) const override;

        FeatureLevel GetFeatureLevel() const override;
        ID3D11Device* GetDevice() const { return m_pDevice.Get(); }
        ID3D11DeviceContext* GetDeviceContext() const { return m_pDeviceContext.Get(); }

    private:
        ComPtr<ID3D11Device> m_pDevice;
        ComPtr<ID3D11DeviceContext> m_pDeviceContext;
        std::wstring m_AdapterName;
        D3D_FEATURE_LEVEL m_FeatureLevel;
        // CommandQueue D3D11
        std::unique_ptr<CommandQueueD3D11> m_CommandQueue;

        /*
         * DescriptorHeap D3D11 模拟
         * 包含标准堆、采样器堆、RTV堆、DSV堆
        */
        std::unique_ptr<DescriptorHeapD3D11> m_pStandardHeap;
        std::unique_ptr<DescriptorHeapD3D11> m_pSamplerHeap;
        std::unique_ptr<DescriptorHeapD3D11> m_pRTVHeap;
        std::unique_ptr<DescriptorHeapD3D11> m_pDSVHeap;
    };

    class D3D11RHI_API SwapChainD3D11 : public SwapChain
    {
    public:
        SwapChainD3D11();
        ~SwapChainD3D11() override;

        bool Initialize(Device* device, const SwapChainDesc& desc) override;
        void Shutdown() override;
        bool IsValid() const override;

        void Present(uint32_t syncInterval, uint32_t presentFlags) override;
        void Resize(uint32_t width, uint32_t height) override;
        uint32_t GetFrameIndex() const override { return 0; }

        uint32_t GetWidth() const override { return m_desc.Width; }
        uint32_t GetHeight() const override { return m_desc.Height; }
        // Get render target view
        RHIRenderTargetView* GetRenderTargetView(uint32_t index) const override;
        // Get back buffer
        RHITexture* GetBackBuffer(uint32_t index) const override;
        // Get swap chain
        IDXGISwapChain* GetSwapChain() const { return m_pSwapChain.Get(); }

    private:
        DeviceD3D11* m_pRHI;
        
        ComPtr<IDXGISwapChain> m_pSwapChain;
        std::unique_ptr<RHITexture> m_pBackBuffer;
        std::unique_ptr<RHIRenderTargetView> m_pRenderTargetView;
        SwapChainDesc m_desc;
    };

    class D3D11RHI_API ShaderCompilerBackendD3D11 : public ShaderCompilerBackend
    {
    public:
        ShaderCompilerBackendD3D11() {};
        ~ShaderCompilerBackendD3D11() override { Shutdown();  };
        bool Initialize() override;
        void Shutdown() override;
        bool IsValid() const override;

        ShaderCompileOptionInternal AddBackendArguments(const ShaderCompileOptions& options) override;
        void PostProcessShader(const ShaderCompileOptions& options, const ShaderPostProcessArgs* postProcessArgs, 
            const ShaderCompileResult& in_result, ShaderCompileResult& out_result) override;
        ShaderReflectionGenerationMode GetShaderReflectionGenerationMode() override;

        // ------------------- Tools -------------------
        std::string SPIRVCompileEnvironment() const override;
    private:
        DeviceD3D11* m_pRHI;
    };
}
