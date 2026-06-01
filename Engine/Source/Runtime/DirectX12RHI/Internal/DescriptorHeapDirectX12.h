#pragma once
#include <RHIResource.h>
#include <d3d12.h>
#include <vector>
#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    class DescriptorHeapDirectX12 : public RHIDescriptorHeap
    {
    public:
        // 构造函数
        // @param pHeap: DirectX 12 描述符堆
        // @param InType: 描述符堆类型
        // @param InCapacity: 描述符堆容量(描述符数量)
        // @param InDescriptorSize: 每个描述符的大小
        DescriptorHeapDirectX12(ID3D12DescriptorHeap* pHeap, RHIDescriptorHeapType InType, uint32_t InCapacity, uint32_t InDescriptorSize)
            : RHIDescriptorHeap(InType, InCapacity)
            , m_pHeap(pHeap)
            , m_DescriptorSize(InDescriptorSize)
        {
            m_FreeList.reserve(InCapacity); // 预分配空闲索引容器的内存
            for (uint32_t i = 0; i < InCapacity; ++i)
                m_FreeList.push_back(i);
            
        }
        ~DescriptorHeapDirectX12() override = default;

        ID3D12DescriptorHeap* GetHeap() const { return m_pHeap.Get(); }
        uint32_t GetDescriptorSize() const { return m_DescriptorSize; }

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const;
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const;

        [[nodiscard]] RHIDescriptorHandle Allocate() override;
        void Free(RHIDescriptorHandle handle) override;

    private:
        ComPtr<ID3D12DescriptorHeap> m_pHeap;
        uint32_t m_DescriptorSize;

        std::vector<uint32_t> m_FreeList;  // 空闲的索引
    };
}
