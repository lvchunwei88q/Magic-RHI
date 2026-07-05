#pragma once

#include "Common/VULKANRHI_API.h"
#include <RHIInterface.h>
#include <RHIResource.h>
#include <cstdint>

// Use Win32 platform
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#include <vulkan.h>
#include "vulkan_win32.h"

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
        DeviceVulKan(const DeviceVulKan&) = delete;
        DeviceVulKan& operator=(const DeviceVulKan&) = delete;

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
        RHIDescriptorHandle CreateRTVHeapDescriptorView(RHITexture* Texture) override;
        RHIDescriptorHandle CreateDSVHeapDescriptorView(RHITexture* Texture) override;

        [[nodiscard]] std::unique_ptr<RHIVertexShader> CreateVertexShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIPixelShader> CreatePixelShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIGeometryShader> CreateGeometryShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIHullShader> CreateHullShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIDomainShader> CreateDomainShader(const CreateShaderDesc& desc) override;
        [[nodiscard]] std::unique_ptr<RHIComputeShader> CreateComputeShader(const CreateShaderDesc& desc) override;
        // TODO: Get shader model version from device shader model version.
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

        [[nodiscard]] const VkDevice* GetDevice()                   const { return &m_Device; }
        [[nodiscard]] const VkPhysicalDevice* GetPhysicalDevice()   const { return &m_PhysicalDevice; }
        [[nodiscard]] const VkInstance* GetInstance()               const { return &m_Instance; }
        [[nodiscard]] const VkSurfaceKHR* GetVulkanWindowSSurface() const { return &m_Surface; }
        // Create a Vulkan window surface
        bool CreateVulkanWindowSurface(HWND windowHandle);

        uint32_t GetGraphicsQueueFamilyIndex() const { return m_GraphicsQueueFamilyIndex; }
        uint32_t GetComputeQueueFamilyIndex() const { return m_ComputeQueueFamilyIndex; }
        uint32_t GetCopyQueueFamilyIndex() const { return m_CopyQueueFamilyIndex; }

    private:
        bool CreateInstance();
        bool PickPhysicalDevice();
        bool CreateLogicalDevice();
        void CreateQueues();
        bool CreateDescriptorPools();

        VkInstance m_Instance = nullptr;
        VkPhysicalDevice m_PhysicalDevice = nullptr;
        VkDevice m_Device = nullptr;
        VkSurfaceKHR m_Surface = nullptr;

        std::wstring m_AdapterName;
        uint32_t SdkVkApiVersion = VK_API_VERSION_1_0;
        uint32_t PhysicalDeviceApiVersion = VK_API_VERSION_1_0;

        uint32_t m_GraphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t m_ComputeQueueFamilyIndex = UINT32_MAX;
        uint32_t m_CopyQueueFamilyIndex = UINT32_MAX;

        std::unique_ptr<GraphicsCommandQueueVulKan> m_GraphicsQueue;
        std::unique_ptr<ComputeCommandQueueVulKan>  m_ComputeQueue;
        std::unique_ptr<CopyCommandQueueVulKan>     m_CopyQueue;

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
        SwapChainVulKan(const SwapChainVulKan&) = delete;
        SwapChainVulKan& operator=(const SwapChainVulKan&) = delete;

        bool Initialize(Device* device, const SwapChainDesc& desc) override;
        void Shutdown() override;
        bool IsValid() const override;

        void Present(uint32_t syncInterval, uint32_t presentFlags) override;
        void Resize(uint32_t width, uint32_t height) override;
        uint32_t GetFrameIndex() const override { return m_CurrentFrame; }

        uint32_t GetWidth() const override { return m_desc.Width; }
        uint32_t GetHeight() const override { return m_desc.Height; }
        RHIRenderTargetView* GetRenderTargetView(uint32_t index) const override;
        RHITexture* GetBackBuffer(uint32_t index) const override;

        VkSwapchainKHR GetSwapChain() const { return m_SwapChain; }
        VkFormat GetFormat() const { return m_SwapChainFormat; }
        uint32_t AcquireNextImage();

        // Get the Vulkan device.
        VkDevice GetDevice() const;
        VkPhysicalDevice GetPhysicalDevice() const;
        // Create and Get the Vulkan window surface.
        VkSurfaceKHR GetVulkanWindowSurface() const;
        bool CreateVulkanWindowSurface(HWND windowHandle);

    private:
        bool VkCreateSwapChain();
        bool VkCreateRenderTargetViews();
        bool VkCreateSemaphores();
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        DeviceVulKan* m_pRHI = nullptr;
        SwapChainDesc m_desc;
        InitialState m_Initialization = InitialState::Shutdown;

        VkSwapchainKHR m_SwapChain = nullptr;
        VkFormat m_SwapChainFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D m_SwapChainExtent = {};
        uint32_t m_CurrentFrame = 0;

        // Because the Vulkan driver might optimize some settings,
        // the final number of buffers could end up being greater than or equal to what we set, so we use a vector.
        std::vector<std::unique_ptr<RHITexture>> m_pBackBuffers;
        std::vector<std::unique_ptr<RHIRenderTargetView>> m_pRenderTargetViews;

        VkSemaphore m_RenderFinishedSemaphore = nullptr;
        VkSemaphore m_ImageAvailableSemaphore = nullptr;
    };

    class VULKANRHI_API ShaderCompilerBackendVulKan : public ShaderCompilerBackend
    {
    public:
        ShaderCompilerBackendVulKan() {};
        ~ShaderCompilerBackendVulKan() override { Shutdown(); };
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
    };
    
}
