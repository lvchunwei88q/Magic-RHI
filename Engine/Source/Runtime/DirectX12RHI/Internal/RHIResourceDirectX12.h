#pragma once
#include "d3dx12.h"
#include <d3d12.h>
#include <Common/RHIException.h>
#include <RHIResource.h>
#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    class RasterizerStateDirectX12 : public RHIRasterizerState
    {
    public:
        RasterizerStateDirectX12(ID3D12RootSignature* /*pRootSignature*/) {}
        ~RasterizerStateDirectX12() override = default;
    };

    class BlendStateDirectX12 : public RHIBlendState

    {
    public:
        BlendStateDirectX12(ID3D12RootSignature* /*pRootSignature*/) {}
        ~BlendStateDirectX12() override = default;
    };

    class DepthStencilStateDirectX12 : public RHIDepthStencilState
    {
    public:
        DepthStencilStateDirectX12(ID3D12RootSignature* /*pRootSignature*/) {}
        ~DepthStencilStateDirectX12() override = default;
    };

    class TextureDirectX12 : public RHITexture
    {
    public:
        TextureDirectX12(ID3D12Resource* pResource, const TextureDesc& desc)
            : m_pResource(pResource)
            , RHITexture(desc) {}
        ~TextureDirectX12() override = default;

        uint64_t GetSize() const override { return 0; }

        ID3D12Resource* GetResource() const { return m_pResource.Get(); }

    private:
        ComPtr<ID3D12Resource> m_pResource;
    };

    // TODO 加入View 构造参数
    class ShaderResourceViewDirectX12 : public RHIShaderResourceView
    {
    public:
        ShaderResourceViewDirectX12(RHIDescriptorHandle handle)
            : m_Handle(handle) {}
        ~ShaderResourceViewDirectX12() override = default;

        const RHIDescriptorHandle& GetHandle() const { return m_Handle; }
        const D3D12_CPU_DESCRIPTOR_HANDLE* GetCPUDescriptorHandle() const { return &m_CPUHandle; }

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
        RHIDescriptorHandle m_Handle;
    };

    class UnorderedAccessViewDirectX12 : public RHIUnorderedAccessView
    {
    public:
        UnorderedAccessViewDirectX12(RHIDescriptorHandle handle)
            : m_Handle(handle) {}
        ~UnorderedAccessViewDirectX12() override = default;

        const RHIDescriptorHandle& GetHandle() const { return m_Handle; }
        const D3D12_CPU_DESCRIPTOR_HANDLE* GetCPUDescriptorHandle() const { return &m_CPUHandle; }

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
        RHIDescriptorHandle m_Handle;
    };

    class RenderTargetViewDirectX12 : public RHIRenderTargetView
    {
    public:
        RenderTargetViewDirectX12(RHIDescriptorHandle handle, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
            : m_CPUHandle(cpuHandle)
            , m_Handle(handle) {}
        ~RenderTargetViewDirectX12() override = default;

        const RHIDescriptorHandle& GetHandle() const { return m_Handle; }
        const D3D12_CPU_DESCRIPTOR_HANDLE* GetCPUDescriptorHandle() const { return &m_CPUHandle; }

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
        RHIDescriptorHandle m_Handle;
    };

    class DepthStencilViewDirectX12 : public RHIDepthStencilView
    {
    public:
        DepthStencilViewDirectX12(RHIDescriptorHandle handle)
            : m_Handle(handle) {}
        ~DepthStencilViewDirectX12() override = default;

        const RHIDescriptorHandle& GetHandle() const { return m_Handle; }
        const D3D12_CPU_DESCRIPTOR_HANDLE* GetCPUDescriptorHandle() const { return &m_CPUHandle; }

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
        RHIDescriptorHandle m_Handle;
    };

    class VertexShaderDirectX12 : public RHIVertexShader
    {
    public:
        VertexShaderDirectX12(const std::vector<uint8_t>& bytecode) : m_Bytecode(bytecode) {}
        ~VertexShaderDirectX12() override = default;
        const std::vector<uint8_t>& GetBytecode() const { return m_Bytecode; }
    private:
        std::vector<uint8_t> m_Bytecode;
    };

    class PixelShaderDirectX12 : public RHIPixelShader
    {
    public:
        PixelShaderDirectX12(const std::vector<uint8_t>& bytecode) : m_Bytecode(bytecode) {}
        ~PixelShaderDirectX12() override = default;
        const std::vector<uint8_t>& GetBytecode() const { return m_Bytecode; }
    private:
        std::vector<uint8_t> m_Bytecode;
    };

    class GeometryShaderDirectX12 : public RHIGeometryShader
    {
    public:
        GeometryShaderDirectX12(const std::vector<uint8_t>& bytecode) : m_Bytecode(bytecode) {}
        ~GeometryShaderDirectX12() override = default;
        const std::vector<uint8_t>& GetBytecode() const { return m_Bytecode; }
    private:
        std::vector<uint8_t> m_Bytecode;
    };

    class HullShaderDirectX12 : public RHIHullShader
    {
    public:
        HullShaderDirectX12(const std::vector<uint8_t>& bytecode) : m_Bytecode(bytecode) {}
        ~HullShaderDirectX12() override = default;
        const std::vector<uint8_t>& GetBytecode() const { return m_Bytecode; }
    private:
        std::vector<uint8_t> m_Bytecode;
    };

    class DomainShaderDirectX12 : public RHIDomainShader
    {
    public:
        DomainShaderDirectX12(const std::vector<uint8_t>& bytecode) : m_Bytecode(bytecode) {}
        ~DomainShaderDirectX12() override = default;
        const std::vector<uint8_t>& GetBytecode() const { return m_Bytecode; }
    private:
        std::vector<uint8_t> m_Bytecode;
    };

    class ComputeShaderDirectX12 : public RHIComputeShader
    {
    public:
        ComputeShaderDirectX12(const std::vector<uint8_t>& bytecode) : m_Bytecode(bytecode) {}
        ~ComputeShaderDirectX12() override = default;
        const std::vector<uint8_t>& GetBytecode() const { return m_Bytecode; }
    private:
        std::vector<uint8_t> m_Bytecode;
    };

    class SamplerStateDirectX12 : public RHISamplerState
    {
    public:
        SamplerStateDirectX12(D3D12_SAMPLER_DESC desc, RHIDescriptorHandle handle) 
            : m_SamplerDesc(desc), RHISamplerState(handle) {} // 初始化 Handle

        ~SamplerStateDirectX12() override = default;

        const D3D12_SAMPLER_DESC& GetSamplerDesc() const { return m_SamplerDesc; }

    private:
        D3D12_SAMPLER_DESC m_SamplerDesc;
    };

    class BufferDirectX12 : public RHIBuffer
    {
    public:
        BufferDirectX12(ID3D12Resource* pResource, const BufferDesc& InDesc, ID3D12Device* InDevice)
            : RHIBuffer(InDesc, RRT_Buffer)
            , m_pResource(pResource)
            , m_Device(InDevice)
        {
        }

        ~BufferDirectX12() override = default;

        uint64_t GetGPUVirtualAddress() const override{return m_pResource->GetGPUVirtualAddress();}

        ID3D12Resource* GetResource() const { return m_pResource.Get(); }
        
        void* Map() override
        {
            if (GetHeapType() != BufferHeapType::Default){
                void* pData = nullptr;
                D3D12_RANGE readRange(0, 0);
                HRESULT hr = m_pResource->Map(0, &readRange, &pData);
                if (FAILED(hr))
                {
                    return nullptr;
                }
                return pData;
            }
#if RHI_ENABLE_RESOURCE_INFO
            ThrowErrorMessage("Failed to unmap default buffer");
#endif
            return nullptr;
        }

        void Unmap() override
        {
            if (GetHeapType() != BufferHeapType::Default)
            {
                D3D12_RANGE writeRange(0, 0);
                m_pResource->Unmap(0, &writeRange);
            }
#if RHI_ENABLE_RESOURCE_INFO
            ThrowErrorMessage("Failed to unmap default buffer");
#endif
        }

    private:
        ComPtr<ID3D12Resource> m_pResource;
        ID3D12Device* m_Device;
    };
}
