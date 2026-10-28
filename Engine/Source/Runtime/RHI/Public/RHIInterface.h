#pragma once

#include "Common/RHI_API.h"
#include "Common/RHITypes.h"
#include "Common/RHIFeatureLevel.h"
#include "Common/RHIDefinitions.h"
// Platform Check
#include "RHIPlatformDetection.h"

// form core minimal
#include <CoreMinimal.h>

#include <memory>
#include <string>
#include "Common/RHIConfig.h" // RHI configuration


namespace RHI
{
    /*
     * We used smart pointers in the class, so when defining the pointers,
     * we must already have complete information about these forward-declared classes,
     * otherwise it will cause undefined issues.
     */
    // CommandList & CommandQueue Forward Declaration
    class RHICommandPool;
    class RHICommandList;
    class RHICommandQueue;
    class RHIFrameCommandQueue;
    // forward declarations
    struct RHIDescriptorHandle;
    class RHIDescriptorHeap;
    struct SamplerStateDesc;
    struct BufferDesc;
    class RHISamplerState;
    class RHIBuffer;
    class RHITexture;
    class RHIRenderTargetView;
    class RHIDepthStencilView;
    // forward shader type
    struct ShaderCompileDesc;
    class RHIVertexShader;
    class RHIPixelShader;
    class RHIGeometryShader;
    class RHIHullShader;
    class RHIDomainShader;
    class RHIComputeShader;

    class RHIRootSignature;
    struct RootSignatureDesc;

    class RHIPipelineState;
    struct GraphicsPipelineStateDesc;
    struct ComputePipelineStateDesc;

    class RHI_API Device
    {
    public:
        /**
         * @brief
         */
        virtual ~Device() = default;

        /**
         * @brief
         * @return
         */
        virtual bool Initialize() = 0;
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
        virtual RHIType GetType() const = 0;
        /**
         * @brief
         * @return
         */
        virtual const std::wstring& GetAdapterName() const = 0;
        /**
         * @brief
         * @return
         */
        virtual FeatureLevel GetFeatureLevel() const = 0;

        /**
         * @brief
         * @param desc
         * @return
         */
        [[nodiscard]] virtual std::shared_ptr<RHISamplerState> CreateSamplerState(const SamplerStateDesc& desc) = 0;
        /**
         * @brief
         * @param samplerState
         */
        virtual void DeleteSamplerState(std::shared_ptr<RHISamplerState>& samplerState) = 0;

        /**
         * @brief
         * @param desc
         * @return
         */
        [[nodiscard]] virtual std::shared_ptr<RHIBuffer> CreateBuffer(BufferDesc& desc) = 0;
        /**
         * @brief
         * @param buffer
         */
        virtual void DeleteBuffer(std::shared_ptr<RHIBuffer>& buffer) = 0;

        /**
         * @brief
         * @param Buffer
         * @param Type
         * @return
         */
        virtual RHIDescriptorHandle CreateStandardHeapDescriptorView(RHIBuffer* Buffer,DescriptorRangeType Type) = 0;
        /**
         * @brief
         * @param Texture
         * @param Type
         * @return
         */
        virtual RHIDescriptorHandle CreateStandardHeapDescriptorView(RHITexture* Texture,DescriptorRangeType Type) = 0;
        /**
         * @brief
         * @param desc
         * @return
         */
        virtual RHIDescriptorHandle CreateSamplerHeapDescriptorView(const SamplerStateDesc& desc) = 0;
        /**
         * @brief
         * @param Texture
         * @return
         */
        virtual RHIDescriptorHandle CreateRTVHeapDescriptorView(RHITexture* Texture) = 0;
        /**
         * @brief
         * @param Texture
         * @return
         */
        virtual RHIDescriptorHandle CreateDSVHeapDescriptorView(RHITexture* Texture) = 0;

        /**
         * @brief In our design, all the work of compiling shaders should be fully done in RHIShaderCompiler.
         * Here, we only deal with creating the shader structures for the corresponding platform,
         * and we need to pass in the shader bytecode, so calling it should be really fast.
         * @param desc
         * @return
         */
        [[nodiscard]] virtual std::unique_ptr<RHIVertexShader> CreateVertexShader(const CreateShaderDesc& desc) = 0;
        /**
         * @brief
         * @param desc
         * @return
         */
        [[nodiscard]] virtual std::unique_ptr<RHIPixelShader> CreatePixelShader(const CreateShaderDesc& desc) = 0;
        /**
         * @brief
         * @param desc
         * @return
         */
        [[nodiscard]] virtual std::unique_ptr<RHIGeometryShader> CreateGeometryShader(const CreateShaderDesc& desc) = 0;
        /**
         * @brief
         * @param desc
         * @return
         */
        [[nodiscard]] virtual std::unique_ptr<RHIHullShader> CreateHullShader(const CreateShaderDesc& desc) = 0;
        /**
         * @brief
         * @param desc
         * @return
         */
        [[nodiscard]] virtual std::unique_ptr<RHIDomainShader> CreateDomainShader(const CreateShaderDesc& desc) = 0;
        /**
         * @brief
         * @param desc
         * @return
         */
        [[nodiscard]] virtual std::unique_ptr<RHIComputeShader> CreateComputeShader(const CreateShaderDesc& desc) = 0;
        /**
         * @brief
         * @return
         */
        virtual ShaderModelVersion GetShaderModelVersion() const = 0;

        /**
         * @brief
         * @param type
         * @return
         */
        [[nodiscard]] virtual std::shared_ptr<RHICommandPool> CreateCommandPool(RHICmdType type) = 0;
        /**
         * @brief
         * @param pool
         * @return
         */
        [[nodiscard]] virtual std::shared_ptr<RHICommandList> CreateCommandList(std::shared_ptr<RHICommandPool>& pool) = 0;
        /**
         * @brief
         * @return
         */
        [[nodiscard]] virtual RHIFrameCommandQueue* GetFrameCommandQueue() const = 0;
        /**
         * @brief
         * @param Type
         * @return
         */
        [[nodiscard]] virtual RHICommandQueue* GetBasicCommandQueue(RHICmdType Type) const = 0;

        /**
         * @brief
         * @param desc
         * @return
         */
        [[nodiscard]] virtual std::shared_ptr<RHIRootSignature> CreateRootSignature(const RootSignatureDesc& desc) = 0;
        /**
         * @brief
         * @param rootSignature
         */
        virtual void DeleteRootSignature(std::shared_ptr<RHIRootSignature>& rootSignature) = 0;

        /**
         * @brief
         * @param desc
         * @return
         */
        [[nodiscard]] virtual std::shared_ptr<RHIPipelineState> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) = 0;
        /**
         * @brief
         * @param desc
         * @return
         */
        [[nodiscard]] virtual std::shared_ptr<RHIPipelineState> CreateComputePipelineState(const ComputePipelineStateDesc& desc) = 0;
        /**
         * @brief
         * @param pipelineState
         */
        virtual void DeletePipelineState(std::shared_ptr<RHIPipelineState>& pipelineState) = 0;

        /**
         * @brief
         * @param type
         * @return
         */
        [[nodiscard]] virtual RHIDescriptorHeap* GetDescriptorHeap(RHIDescriptorHeapType type) const = 0;
    protected:
        InitialState m_Initialization;
    };

    class RHI_API SwapChain
    {
    public:
        /**
         * @brief
         */
        virtual ~SwapChain() = default;

        /**
         * @brief
         * @param device
         * @param desc
         * @return
         */
        virtual bool Initialize(Device* device, const SwapChainDesc& desc) = 0;
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
         * @param width
         * @param height
         */
        virtual void Resize(uint32_t width, uint32_t height) = 0;
        /**
         * @brief
         * @return
         */
        virtual uint32_t AcquireNextImage() = 0;

        /**
         * @brief
         * @return
         */
        virtual uint32_t GetWidth() const = 0;
        /**
         * @brief
         * @return
         */
        virtual uint32_t GetHeight() const = 0;

        /**
         * @brief
         * @param index
         * @return
         */
        virtual RHIRenderTargetView* GetRenderTargetView(uint32_t index) const = 0;
        /**
         * @brief
         * @return
         */
        virtual RHITextureFormat GetRenderTargetFormat() const = 0;
        /**
         * @brief
         * @param index
         * @return
         */
        virtual RHITexture* GetBackBuffer(uint32_t index) const = 0;
        /**
         * @brief
         * @return
         */
        virtual GraphicsQueueSwapChainInfo GetGraphicsQueueSwapChainInfo() const = 0;

    protected:
        InitialState m_Initialization;
    };

    /**
     * @brief This should be handled by RHI's RHIShaderCompiler system, users usually don't need to use it.
     */
    class RHI_API ShaderCompilerBackend
    {
    public:
        /**
         * @brief
         */
        virtual ~ShaderCompilerBackend() = default;
        /**
         * @brief
         * @return
         */
        virtual bool Initialize() = 0;
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
         * @param options
         * @return
         */
        virtual ShaderCompileOptionInternal AddBackendArguments(const ShaderCompileOptions& options) = 0;
        /**
         * @brief
         * @param options
         * @param postProcessArgs
         * @param in_result
         * @param out_result
         */
        virtual void PostProcessShader(const ShaderCompileOptions& options, const ShaderPostProcessArgs* postProcessArgs, 
            const ShaderCompileResult& in_result, ShaderCompileResult& out_result) = 0;
        /**
         * @brief
         * @return
         */
        virtual ShaderPipelineGenerationMode GetShaderPipelineGenerationMode() = 0;

        // ------------------- Tools -------------------
        /**
         * @brief
         * @return
         */
        virtual std::string SPIRVCompileEnvironment() const = 0;
    protected:
        InitialState m_Initialization;
    };
}
