#pragma once

#include "Common/RHI_API.h"
#include "Common/RHITypes.h"
#include "Common/RHIFeatureLevel.h"
#include "Common/RHIPlatformDetection.h"
#include "Common/RHIDefinitions.h"

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
    class RHICommandAllocator;
    class RHICommandList;
    class RHICommandQueue;
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
        virtual ~Device() = default;

        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual bool IsValid() const = 0;
        
        virtual RHIType GetType() const = 0;
        virtual const std::wstring& GetAdapterName() const = 0;
        virtual FeatureLevel GetFeatureLevel() const = 0;

        [[nodiscard]] virtual std::shared_ptr<RHISamplerState> CreateSamplerState(const SamplerStateDesc& desc) = 0;
        virtual void DeleteSamplerState(std::shared_ptr<RHISamplerState>& samplerState) = 0;

        [[nodiscard]] virtual std::shared_ptr<RHIBuffer> CreateBuffer(BufferDesc& desc) = 0;
        virtual void DeleteBuffer(std::shared_ptr<RHIBuffer>& buffer) = 0;

        virtual RHIDescriptorHandle CreateStandardHeapDescriptorView(RHIBuffer* Buffer,DescriptorRangeType Type) = 0;
        virtual RHIDescriptorHandle CreateStandardHeapDescriptorView(RHITexture* Texture,DescriptorRangeType Type) = 0;
        virtual RHIDescriptorHandle CreateSamplerHeapDescriptorView(const SamplerStateDesc& desc) = 0;
        virtual RHIDescriptorHandle CreateRTVHeapDescriptorView(class RHIRenderTargetView* InView) = 0;
        virtual RHIDescriptorHandle CreateDSVHeapDescriptorView(class RHIDepthStencilView* InView) = 0;

        /* In our design, all the work of compiling shaders should be fully done in RHIShaderCompiler.
        * Here, we only deal with creating the shader structures for the corresponding platform,
        * and we need to pass in the shader bytecode, so calling it should be really fast.
        */
        [[nodiscard]] virtual std::unique_ptr<RHIVertexShader> CreateVertexShader(const CreateShaderDesc& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<RHIPixelShader> CreatePixelShader(const CreateShaderDesc& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<RHIGeometryShader> CreateGeometryShader(const CreateShaderDesc& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<RHIHullShader> CreateHullShader(const CreateShaderDesc& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<RHIDomainShader> CreateDomainShader(const CreateShaderDesc& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<RHIComputeShader> CreateComputeShader(const CreateShaderDesc& desc) = 0;
        virtual ShaderModelVersion GetShaderModelVersion() const = 0;

        [[nodiscard]] virtual std::shared_ptr<RHICommandAllocator> CreateCommandAllocator(RHICmdType type) = 0;
        [[nodiscard]] virtual std::shared_ptr<RHICommandList> CreateCommandList(std::shared_ptr<RHICommandAllocator>& allocator) = 0;
        [[nodiscard]] virtual RHICommandQueue* GetCommandQueue(RHICmdType Type) const = 0;

        [[nodiscard]] virtual std::shared_ptr<RHIRootSignature> CreateRootSignature(const RootSignatureDesc& desc) = 0;
        virtual void DeleteRootSignature(std::shared_ptr<RHIRootSignature>& rootSignature) = 0;

        [[nodiscard]] virtual std::shared_ptr<RHIPipelineState> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) = 0;
        [[nodiscard]] virtual std::shared_ptr<RHIPipelineState> CreateComputePipelineState(const ComputePipelineStateDesc& desc) = 0;
        virtual void DeletePipelineState(std::shared_ptr<RHIPipelineState>& pipelineState) = 0;

        [[nodiscard]] virtual RHIDescriptorHeap* GetDescriptorHeap(RHIDescriptorHeapType type) const = 0;
    protected:
        InitialState m_Initialization;
    };

    class RHI_API SwapChain
    {
    public:
        virtual ~SwapChain() = default;

        virtual bool Initialize(Device* device, const SwapChainDesc& desc) = 0;
        virtual void Shutdown() = 0;
        virtual bool IsValid() const = 0;

        virtual void Present(uint32_t syncInterval, uint32_t presentFlags) = 0;
        virtual void Resize(uint32_t width, uint32_t height) = 0;
        virtual uint32_t GetFrameIndex() const = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

        virtual RHIRenderTargetView* GetRenderTargetView(uint32_t index) const = 0;
        virtual RHITexture* GetBackBuffer(uint32_t index) const = 0;

    protected:
        InitialState m_Initialization;
    };

    // This should be handled by RHI's RHIShaderCompiler system, users usually don't need to use it.
    class RHI_API ShaderCompilerBackend
    {
    public:
        virtual ~ShaderCompilerBackend() = default;
        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual bool IsValid() const = 0;

        virtual ShaderCompileOptionInternal AddBackendArguments(const ShaderCompileOptions& options) = 0;
        virtual void PostProcessShader(const ShaderCompileOptions& options, const ShaderCompileResult& in_result, ShaderCompileResult& out_result) = 0;
        virtual ShaderReflectionGenerationMode GetShaderReflectionGenerationMode() = 0;

        // ------------------- Tools -------------------
        virtual std::string SPIRVCompileEnvironment() const = 0;
    protected:
        InitialState m_Initialization;
    };
}
