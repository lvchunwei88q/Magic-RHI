#pragma once
#include "d3dx12.h"
#include <d3d12.h>
#include <Common/Check.h>
#include <RHIResource.h>
#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    class RasterizerStateD3D12 : public RHIRasterizerState
    {
    public:
        RasterizerStateD3D12(ID3D12RootSignature* /*pRootSignature*/) {}
        ~RasterizerStateD3D12() override = default;
    };

    class BlendStateD3D12 : public RHIBlendState

    {
    public:
        BlendStateD3D12(ID3D12RootSignature* /*pRootSignature*/) {}
        ~BlendStateD3D12() override = default;
    };

    class DepthStencilStateD3D12 : public RHIDepthStencilState
    {
    public:
        DepthStencilStateD3D12(ID3D12RootSignature* /*pRootSignature*/) {}
        ~DepthStencilStateD3D12() override = default;
    };

    class BufferD3D12 : public RHIBuffer
    {
    public:
        BufferD3D12(ID3D12Resource* pResource, const BufferDesc& InDesc, ID3D12Device* InDevice)
            : RHIBuffer(InDesc, RRT_Buffer)
            , m_pResource(pResource)
            , m_Device(InDevice)
        {
        }

        ~BufferD3D12() override = default;

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

    class TextureD3D12 : public RHITexture
    {
    public:
        TextureD3D12(ID3D12Resource* pResource, const TextureDesc& desc)
            : m_pResource(pResource)
            , RHITexture(desc) {}
        ~TextureD3D12() override = default;

        uint64_t GetSize() const override { return 0; }

        ID3D12Resource* GetResource() const { return m_pResource.Get(); }

    private:
        ComPtr<ID3D12Resource> m_pResource;
    };

    // TODO 加入View 构造参数
    class ConstantBufferViewD3D12 : public RHIConstantBufferView
    {
    public:
        ConstantBufferViewD3D12(D3D12_GPU_VIRTUAL_ADDRESS gpuAddress)
            : m_GPUAddress(gpuAddress) {}
        ~ConstantBufferViewD3D12() override = default;

        const D3D12_CPU_DESCRIPTOR_HANDLE* GetCPUDescriptorHandle() const { return &m_CPUHandle; }
        void SetCPUDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) { m_CPUHandle = cpuHandle; }
        uint64_t GetGPUVirtualAddress() const override { return (uint64_t)m_GPUAddress; }

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
        D3D12_GPU_VIRTUAL_ADDRESS m_GPUAddress;
    };

    class ShaderResourceViewD3D12 : public RHIShaderResourceView
    {
    public:
        ShaderResourceViewD3D12(D3D12_GPU_VIRTUAL_ADDRESS gpuAddress)
            : m_GPUAddress(gpuAddress) {}
        ~ShaderResourceViewD3D12() override = default;

        const D3D12_CPU_DESCRIPTOR_HANDLE* GetCPUDescriptorHandle() const { return &m_CPUHandle; }
        void SetCPUDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) { m_CPUHandle = cpuHandle; }
        uint64_t GetGPUVirtualAddress() const override { return (uint64_t)m_GPUAddress; }

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
        D3D12_GPU_VIRTUAL_ADDRESS m_GPUAddress;
    };

    class UnorderedAccessViewD3D12 : public RHIUnorderedAccessView
    {
    public:
        UnorderedAccessViewD3D12(D3D12_GPU_VIRTUAL_ADDRESS gpuAddress)
            : m_GPUAddress(gpuAddress) {}
        ~UnorderedAccessViewD3D12() override = default;

        const D3D12_CPU_DESCRIPTOR_HANDLE* GetCPUDescriptorHandle() const { return &m_CPUHandle; }
        void SetCPUDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) { m_CPUHandle = cpuHandle; }
        uint64_t GetGPUVirtualAddress() const override { return (uint64_t)m_GPUAddress; }

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
        D3D12_GPU_VIRTUAL_ADDRESS m_GPUAddress;
    };

    class RenderTargetViewD3D12 : public RHIRenderTargetView
    {
    public:
        RenderTargetViewD3D12(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
            : m_CPUHandle(cpuHandle) {}
        ~RenderTargetViewD3D12() override = default;

        const D3D12_CPU_DESCRIPTOR_HANDLE* GetCPUDescriptorHandle() const { return &m_CPUHandle; }

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
    };

    class DepthStencilViewD3D12 : public RHIDepthStencilView
    {
    public:
        DepthStencilViewD3D12(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
            : m_CPUHandle(cpuHandle) {}
        ~DepthStencilViewD3D12() override = default;

        const D3D12_CPU_DESCRIPTOR_HANDLE* GetCPUDescriptorHandle() const { return &m_CPUHandle; }

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
    };

    class VertexShaderD3D12 : public RHIVertexShader
    {
    public:
        VertexShaderD3D12(const std::vector<uint8_t>& bytecode) : m_Bytecode(bytecode) {}
        ~VertexShaderD3D12() override = default;
        const std::vector<uint8_t>& GetBytecode() const { return m_Bytecode; }
    private:
        std::vector<uint8_t> m_Bytecode;
    };

    class PixelShaderD3D12 : public RHIPixelShader
    {
    public:
        PixelShaderD3D12(const std::vector<uint8_t>& bytecode) : m_Bytecode(bytecode) {}
        ~PixelShaderD3D12() override = default;
        const std::vector<uint8_t>& GetBytecode() const { return m_Bytecode; }
    private:
        std::vector<uint8_t> m_Bytecode;
    };

    class GeometryShaderD3D12 : public RHIGeometryShader
    {
    public:
        GeometryShaderD3D12(const std::vector<uint8_t>& bytecode) : m_Bytecode(bytecode) {}
        ~GeometryShaderD3D12() override = default;
        const std::vector<uint8_t>& GetBytecode() const { return m_Bytecode; }
    private:
        std::vector<uint8_t> m_Bytecode;
    };

    class HullShaderD3D12 : public RHIHullShader
    {
    public:
        HullShaderD3D12(const std::vector<uint8_t>& bytecode) : m_Bytecode(bytecode) {}
        ~HullShaderD3D12() override = default;
        const std::vector<uint8_t>& GetBytecode() const { return m_Bytecode; }
    private:
        std::vector<uint8_t> m_Bytecode;
    };

    class DomainShaderD3D12 : public RHIDomainShader
    {
    public:
        DomainShaderD3D12(const std::vector<uint8_t>& bytecode) : m_Bytecode(bytecode) {}
        ~DomainShaderD3D12() override = default;
        const std::vector<uint8_t>& GetBytecode() const { return m_Bytecode; }
    private:
        std::vector<uint8_t> m_Bytecode;
    };

    class ComputeShaderD3D12 : public RHIComputeShader
    {
    public:
        ComputeShaderD3D12(const std::vector<uint8_t>& bytecode) : m_Bytecode(bytecode) {}
        ~ComputeShaderD3D12() override = default;
        const std::vector<uint8_t>& GetBytecode() const { return m_Bytecode; }
    private:
        std::vector<uint8_t> m_Bytecode;
    };

    class SamplerStateD3D12 : public RHISamplerState
    {
    public:
        SamplerStateD3D12(D3D12_SAMPLER_DESC desc, RHIDescriptorHandle handle) 
            : m_SamplerDesc(desc), RHISamplerState(handle) {} // 初始化 Handle

        ~SamplerStateD3D12() override = default;

        const D3D12_SAMPLER_DESC& GetSamplerDesc() const { return m_SamplerDesc; }

    private:
        D3D12_SAMPLER_DESC m_SamplerDesc;
    };
}
