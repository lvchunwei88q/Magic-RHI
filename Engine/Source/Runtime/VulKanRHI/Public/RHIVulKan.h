#pragma once

#include "Common/VULKANRHI_API.h"
#include <RHIInterface.h>
#include <RHIResource.h>
#include <cstdint>
#include <vulkan.h>

// DescriptorHeap
#include "DescriptorHeapVulKan.h"

#include <string>
#include <vector>
#include <memory>

namespace RHI
{
    class CommandQueueVulKan;
    using GraphicsCommandQueueVulKan = CommandQueueVulKan;
    using ComputeCommandQueueVulKan = CommandQueueVulKan;
    using CopyCommandQueueVulKan = CommandQueueVulKan;

    class VULKANRHI_API DeviceVulKan : public Device
    {
    public:
        DeviceVulKan();
        ~DeviceVulKan() override;

        bool Initialize() override;
        void Shutdown() override;
        bool IsValid() const override;
        
        RHIType GetType() const override { return RHIType::VulKan; }
        const std::wstring& GetAdapterName() const override { return m_AdapterName; }
        FeatureLevel GetFeatureLevel() const override;
        
        [[nodiscard]] std::shared_ptr<RHISamplerState> CreateSamplerState(const SamplerStateDesc& desc) override;
        void DeleteSamplerState(std::shared_ptr<RHI::RHISamplerState>& samplerState) override;

        [[nodiscard]] std::shared_ptr<RHIBuffer> CreateBuffer(BufferDesc& desc) override;
        void DeleteBuffer(std::shared_ptr<RHI::RHIBuffer>& buffer) override;

        RHIDescriptorHandle CreateStandardHeapDescriptorView(RHIBuffer* Buffer,DescriptorRangeType Type) override;
        RHIDescriptorHandle CreateStandardHeapDescriptorView(RHITexture* Texture,DescriptorRangeType Type) override;
        RHIDescriptorHandle CreateSamplerHeapDescriptorView(const SamplerStateDesc& desc) override;
        RHIDescriptorHandle CreateRTVHeapDescriptorView(RHIRenderTargetView* InView) override;
        RHIDescriptorHandle CreateDSVHeapDescriptorView(RHIDepthStencilView* InView) override;

        [[nodiscard]] std::unique_ptr<RHIVertexShader> CreateVertexShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIPixelShader> CreatePixelShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIGeometryShader> CreateGeometryShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIHullShader> CreateHullShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIDomainShader> CreateDomainShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIComputeShader> CreateComputeShader(const CreateShaderDesc& desc) override;
        ShaderModelVersion GetShaderModelVersion() const override { return ShaderModelVersion::SM_6_9; }

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

        VkDevice GetDevice() const { return m_Device; }
        VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
        VkInstance GetInstance() const { return m_Instance; }

    private:
        bool CreateInstance();
        bool PickPhysicalDevice();
        bool CreateLogicalDevice();
        bool CreateDescriptorPools();

        VkInstance m_Instance = nullptr;
        VkPhysicalDevice m_PhysicalDevice = nullptr;
        VkDevice m_Device = nullptr;

        std::wstring m_AdapterName;
        uint32_t ApiVersion = VK_API_VERSION_1_0;

        uint32_t m_GraphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t m_ComputeQueueFamilyIndex = UINT32_MAX;
        uint32_t m_CopyQueueFamilyIndex = UINT32_MAX;

        //std::unique_ptr<GraphicsCommandQueueVulKan> m_GraphicsQueueWrapper;
        //std::unique_ptr<ComputeCommandQueueVulKan>  m_ComputeQueueWrapper;
        //std::unique_ptr<CopyCommandQueueVulKan>     m_CopyQueueWrapper;

        std::unique_ptr<DescriptorHeapVulKan> m_pStandardHeap;
        std::unique_ptr<DescriptorHeapVulKan> m_pSamplerHeap;
        std::unique_ptr<DescriptorHeapVulKan> m_pRTVHeap;
        std::unique_ptr<DescriptorHeapVulKan> m_pDSVHeap;
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
        uint32_t GetFrameIndex() const override { return 0; }

        uint32_t GetWidth() const override { return m_desc.Width; }
        uint32_t GetHeight() const override { return m_desc.Height; }
        RHIRenderTargetView* GetRenderTargetView(uint32_t index) const override;
        RHITexture* GetBackBuffer(uint32_t index) const override;

    private:
        DeviceVulKan* m_pRHI;
        SwapChainDesc m_desc;
    };

    class VULKANRHI_API ShaderCompilerBackendVulKan : public ShaderCompilerBackend
    {
    public:
        ShaderCompilerBackendVulKan() {};
        ~ShaderCompilerBackendVulKan() override { Shutdown(); };
        bool Initialize() override;
        void Shutdown() override;
        bool IsValid() const override;

        ShaderCompileOptionInternal AddBackendArguments(const ShaderCompileOptions& options) override { return {}; }
        void PostProcessShader(const ShaderCompileOptions& options, const ShaderPostProcessArgs* postProcessArgs, 
            const ShaderCompileResult& in_result, ShaderCompileResult& out_result) override {}
        ShaderReflectionGenerationMode GetShaderReflectionGenerationMode() override;

        // ------------------- Tools -------------------
        std::string SPIRVCompileEnvironment() const override;
    private:
    };
    
}
