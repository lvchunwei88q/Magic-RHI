#pragma once

#include "Common/D3D12RHI_API.h"
#include <RHIInterface.h>
#include <d3d12.h>
#include <dxgi1_6.h>
// Shader Compiler
#include <include/dxcapi.h>

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

    class D3D12RHI_API RHID3D12 : public Device
    {
    public:
        RHID3D12();
        ~RHID3D12() override;

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

        [[nodiscard]] std::shared_ptr<RHIVertexShader> CompileVertexShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIPixelShader> CompilePixelShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIGeometryShader> CompileGeometryShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIHullShader> CompileHullShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIDomainShader> CompileDomainShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIComputeShader> CompileComputeShader(const ShaderCompileDesc& desc) override;
        ShaderModelVersion GetShaderModelVersion() const override;

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

    public:
        std::string GetShaderTarget(const char* prefix) const;

        bool CompileShaderToBytecode(const std::string& source, const std::string& entryPoint, 
                                    const std::string& profile, bool enableDebug, const std::vector<ShaderMacro>& macros,
                                    std::vector<uint8_t>& outBytecode,std::string& basePath);
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

        // ShaderCompiler
        ComPtr<IDxcCompiler3> compiler;
        ComPtr<IDxcUtils> utils;
        ComPtr<IDxcIncludeHandler> includeHandler;
    };
}
