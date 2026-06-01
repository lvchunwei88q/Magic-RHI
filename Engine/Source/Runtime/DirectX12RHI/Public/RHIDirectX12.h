#pragma once

#include "Common/DIRECTX12RHI_API.h"
#include <RHI.h>
#include <d3d12.h>
#include <dxgi1_6.h>
// Shader Compiler
#include <include/d3d12shader.h>
#include <include/dxcapi.h>

// DescriptorHeap
#include "DescriptorHeapDirectX12.h"

#include <vector>
#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    // CommandQueue Forward Declaration
    class CommandQueueDirectX12;
    using GraphicsCommandQueueDirectX12 = CommandQueueDirectX12;
    using ComputeCommandQueueDirectX12 = CommandQueueDirectX12;
    using CopyCommandQueueDirectX12 = CommandQueueDirectX12;

    class DIRECTX12RHI_API RHIDirectX12 : public Device
    {
    public:
        RHIDirectX12();
        ~RHIDirectX12() override;

        bool Initialize() override;
        void Shutdown() override;

        bool IsValid() const override;
        RHIType GetType() const override { return RHIType::DirectX12; }
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

        [[nodiscard]] std::shared_ptr<RHICommandList> CreateCommandList(RHICmdListType type) override;
        [[nodiscard]] std::shared_ptr<RHICommandQueue> GetCommandQueue(RHICmdListType Type) const override;

        [[nodiscard]] std::shared_ptr<RHIVertexShader> CompileVertexShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIPixelShader> CompilePixelShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIGeometryShader> CompileGeometryShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIHullShader> CompileHullShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIDomainShader> CompileDomainShader(const ShaderCompileDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIComputeShader> CompileComputeShader(const ShaderCompileDesc& desc) override;

        void CreateQueues();

        [[nodiscard]] std::shared_ptr<RHIRootSignature> CreateRootSignature(const RootSignatureDesc& desc) override;
        void DeleteRootSignature(std::shared_ptr<RHI::RHIRootSignature>& rootSignature) override;

        [[nodiscard]] std::shared_ptr<RHIPipelineState> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) override;
        [[nodiscard]] std::shared_ptr<RHIPipelineState> CreateComputePipelineState(const ComputePipelineStateDesc& desc) override;
        void DeletePipelineState(std::shared_ptr<RHIPipelineState>& pipelineState) override;
        
        FeatureLevel GetFeatureLevel() const override;
        ID3D12Device* GetDevice() const { return m_pDevice.Get(); }
        IDXGIAdapter1* GetAdapter() const { return m_pAdapter.Get(); }

    public:
        std::string GetShaderTarget(const char* prefix) const;

        bool CompileShaderToBytecode(const std::string& source, const std::string& entryPoint, 
                                    const std::string& profile, bool enableDebug,
                                    std::vector<uint8_t>& outBytecode,std::string& basePath);
    private:
        ComPtr<ID3D12Device> m_pDevice;
        std::wstring m_AdapterName;
        D3D_FEATURE_LEVEL m_FeatureLevel;
        
        ComPtr<IDXGIAdapter1> m_pAdapter; // GPU

        // CommandQueue
        std::shared_ptr<GraphicsCommandQueueDirectX12> m_GraphicsQueue;
        std::shared_ptr<ComputeCommandQueueDirectX12> m_ComputeQueue;
        std::shared_ptr<CopyCommandQueueDirectX12> m_CopyQueue;

        std::unique_ptr<DescriptorHeapDirectX12> m_pStandardHeap;
        std::unique_ptr<DescriptorHeapDirectX12> m_pSamplerHeap;
        std::unique_ptr<DescriptorHeapDirectX12> m_pRTVHeap;
        std::unique_ptr<DescriptorHeapDirectX12> m_pDSVHeap;

        // ShaderCompiler
        ComPtr<IDxcCompiler3> compiler;
        ComPtr<IDxcUtils> utils;
        ComPtr<IDxcIncludeHandler> includeHandler;
    };
}
