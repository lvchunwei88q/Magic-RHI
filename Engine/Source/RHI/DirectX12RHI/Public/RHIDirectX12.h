#pragma once

#include "Common/DIRECTX12RHI_API.h"
#include <RHI.h>
#include "DescriptorHeapAllocator.h"
#include <d3d12.h>
#include <dxgi1_6.h>

#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

// CommandQueue Forward Declaration
class CommandQueueDirectX12;
using GraphicsCommandQueueDirectX12 = CommandQueueDirectX12;
using ComputeCommandQueueDirectX12 = CommandQueueDirectX12;
using CopyCommandQueueDirectX12 = CommandQueueDirectX12;

namespace RHI
{
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
        
        std::shared_ptr<RHISamplerState> CreateSamplerState(const SamplerStateDesc& desc) override;
        void DeleteSamplerState(std::shared_ptr<RHI::RHISamplerState>& samplerState) override;

        std::shared_ptr<RHIBuffer> CreateBuffer(const BufferDesc& desc) override;
        void DeleteBuffer(std::shared_ptr<RHI::RHIBuffer>& buffer) override;

        std::shared_ptr<RHICommandList> CreateCommandList(RHICmdListType type) override;
        std::shared_ptr<RHICommandQueue> GetCommandQueue(RHICmdListType Type) const override;

        void CreateQueues();

        FeatureLevel GetFeatureLevel() const override;
        ID3D12Device* GetDevice() const { return m_pDevice.Get(); }
        IDXGIAdapter1* GetAdapter() const { return m_pAdapter.Get(); }
        
        ID3D12DescriptorHeap* GetStandardHeap() const { return m_pStandardHeap.Get(); }
        ID3D12DescriptorHeap* GetSamplerHeap() const { return m_pSamplerHeap.Get(); }
        ID3D12DescriptorHeap* GetRTVHeap() const { return m_pRTVHeap.Get(); }
        ID3D12DescriptorHeap* GetDSVHeap() const { return m_pDSVHeap.Get(); }

        D3D12_CPU_DESCRIPTOR_HANDLE GetStandardCPUHandle(uint32_t index) const;
        D3D12_GPU_DESCRIPTOR_HANDLE GetStandardGPUHandle(uint32_t index) const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetSamplerCPUHandle(uint32_t index) const;
        D3D12_GPU_DESCRIPTOR_HANDLE GetSamplerGPUHandle(uint32_t index) const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUHandle(uint32_t index) const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUHandle(uint32_t index) const;

    private:
        ComPtr<ID3D12Device> m_pDevice;
        std::wstring m_AdapterName;
        D3D_FEATURE_LEVEL m_FeatureLevel;
        
        ComPtr<IDXGIAdapter1> m_pAdapter; // GPU

        // CommandQueue
        std::shared_ptr<GraphicsCommandQueueDirectX12> m_GraphicsQueue;
        std::shared_ptr<ComputeCommandQueueDirectX12> m_ComputeQueue;
        std::shared_ptr<CopyCommandQueueDirectX12> m_CopyQueue;

        ComPtr<ID3D12DescriptorHeap> m_pStandardHeap;
        ComPtr<ID3D12DescriptorHeap> m_pSamplerHeap;
        ComPtr<ID3D12DescriptorHeap> m_pRTVHeap;
        ComPtr<ID3D12DescriptorHeap> m_pDSVHeap;

        // DescriptorHeapAllocator
        uint32_t m_StandardDescriptorSize;
        uint32_t m_SamplerDescriptorSize;
        uint32_t m_RTVDescriptorSize;
        uint32_t m_DSVDescriptorSize;

        RHIDescriptorHeapAllocator<RHI_DESCRIPTOR_HEAP_SIZE_STANDARD> m_StandardHeapAllocator;
        RHIDescriptorHeapAllocator<RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER> m_SamplerHeapAllocator;
        RHIDescriptorHeapAllocator<RHI_DESCRIPTOR_HEAP_SIZE_RENDER_TARGET> m_RTVHeapAllocator;
        RHIDescriptorHeapAllocator<RHI_DESCRIPTOR_HEAP_SIZE_DEPTH_STENCIL> m_DSVHeapAllocator;
    };
}
