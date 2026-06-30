#pragma once

#include "Common/VULKANRHI_API.h"
#include <RHIInterface.h>
#include <RHIResource.h>
#include <vulkan.h>

#include <string>
#include <vector>
#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    // CommandQueue Forward Declaration

    class VULKANRHI_API DeviceVulKan : public Device
    {
    public:
        DeviceVulKan();
        ~DeviceVulKan() override;

        bool Initialize() override;
        void Shutdown() override;
        bool IsValid() const override;
        
        RHIType GetType() const override { return RHIType::VulKan; }
        const std::wstring& GetAdapterName() const override { return L"Vulkan"; }
        FeatureLevel GetFeatureLevel() const override;
        
        [[nodiscard]] std::shared_ptr<RHISamplerState> CreateSamplerState(const SamplerStateDesc& desc) override { return nullptr; }
        void DeleteSamplerState(std::shared_ptr<RHI::RHISamplerState>& samplerState) override { }

        [[nodiscard]] std::shared_ptr<RHIBuffer> CreateBuffer(BufferDesc& desc) override { return nullptr; }
        void DeleteBuffer(std::shared_ptr<RHI::RHIBuffer>& buffer) override { }

        RHIDescriptorHandle CreateStandardHeapDescriptorView(RHIBuffer* Buffer,DescriptorRangeType Type) override { return {}; }
        RHIDescriptorHandle CreateStandardHeapDescriptorView(RHITexture* Texture,DescriptorRangeType Type) override { return {}; }
        RHIDescriptorHandle CreateSamplerHeapDescriptorView(const SamplerStateDesc& desc) override { return {}; }
        RHIDescriptorHandle CreateRTVHeapDescriptorView(RHIRenderTargetView* InView) override { return {}; }
        RHIDescriptorHandle CreateDSVHeapDescriptorView(RHIDepthStencilView* InView) override { return {}; }

        [[nodiscard]] std::unique_ptr<RHIVertexShader> CreateVertexShader(const CreateShaderDesc& desc) override { return nullptr; }
        [[nodiscard]] std::unique_ptr<RHIPixelShader> CreatePixelShader(const CreateShaderDesc& desc) override { return nullptr; }
        [[nodiscard]] std::unique_ptr<RHIGeometryShader> CreateGeometryShader(const CreateShaderDesc& desc) override { return nullptr; }
        [[nodiscard]] std::unique_ptr<RHIHullShader> CreateHullShader(const CreateShaderDesc& desc) override { return nullptr; }
        [[nodiscard]] std::unique_ptr<RHIDomainShader> CreateDomainShader(const CreateShaderDesc& desc) override { return nullptr; }
        [[nodiscard]] std::unique_ptr<RHIComputeShader> CreateComputeShader(const CreateShaderDesc& desc) override { return nullptr; }
        ShaderModelVersion GetShaderModelVersion() const override { return ShaderModelVersion::SM_6_9; }

        [[nodiscard]] std::shared_ptr<RHICommandAllocator> CreateCommandAllocator(RHICmdType type) override { return nullptr; }
        [[nodiscard]] std::shared_ptr<RHICommandList> CreateCommandList(std::shared_ptr<RHICommandAllocator>& allocator) override { return nullptr; }
        [[nodiscard]] RHICommandQueue* GetCommandQueue(RHICmdType Type) const override { return nullptr; }

        [[nodiscard]] std::shared_ptr<RHIRootSignature> CreateRootSignature(const RootSignatureDesc& desc) override { return nullptr; }
        void DeleteRootSignature(std::shared_ptr<RHI::RHIRootSignature>& rootSignature) override { }

        [[nodiscard]] std::shared_ptr<RHIPipelineState> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) override { return nullptr; }
        [[nodiscard]] std::shared_ptr<RHIPipelineState> CreateComputePipelineState(const ComputePipelineStateDesc& desc) override { return nullptr; }
        void DeletePipelineState(std::shared_ptr<RHIPipelineState>& pipelineState) override { }

        [[nodiscard]] RHIDescriptorHeap* GetDescriptorHeap(RHIDescriptorHeapType type) const override { return nullptr; }

    private:
    };

    class VULKANRHI_API SwapChainVulKan : public SwapChain
    {
    public:
        SwapChainVulKan();
        ~SwapChainVulKan() override;

        bool Initialize(Device* device, const SwapChainDesc& desc) override;
        void Shutdown() override;
        bool IsValid() const override;

        void Present(uint32_t syncInterval, uint32_t presentFlags) override;
        void Resize(uint32_t width, uint32_t height) override;
        uint32_t GetFrameIndex() const override { return 0; }// Get current frame index

        uint32_t GetWidth() const override { return m_desc.Width; }
        uint32_t GetHeight() const override { return m_desc.Height; }
        // Get render target view
        RHIRenderTargetView* GetRenderTargetView(uint32_t index) const override;
        // Get back buffer
        RHITexture* GetBackBuffer(uint32_t index) const override;

    private:
        DeviceVulKan* m_pRHI;

        SwapChainDesc m_desc;
    };

    class VULKANRHI_API ShaderCompilerBackendVulKan : public ShaderCompilerBackend
    {
    public:
        ShaderCompilerBackendVulKan() {};
        ~ShaderCompilerBackendVulKan() override { Shutdown();  };
        bool Initialize() override;
        void Shutdown() override;
        bool IsValid() const override;

        ShaderCompileOptionInternal AddBackendArguments(const ShaderCompileOptions& options) override { return {}; }
        void PostProcessShader(const ShaderCompileOptions& options, const ShaderPostProcessArgs* postProcessArgs, 
            const ShaderCompileResult& in_result, ShaderCompileResult& out_result) override { }
        ShaderReflectionGenerationMode GetShaderReflectionGenerationMode() override;

        // ------------------- Tools -------------------
        std::string SPIRVCompileEnvironment() const override;
    private:
    };
    
}
