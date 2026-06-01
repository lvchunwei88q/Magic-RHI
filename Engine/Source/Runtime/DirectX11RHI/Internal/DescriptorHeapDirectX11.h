#pragma once
#include <RHIResource.h>
#include <vector>

namespace RHI
{
    class DescriptorHeapDirectX11 : public RHIDescriptorHeap
    {
    public:
        DescriptorHeapDirectX11(RHIDescriptorHeapType InType, uint32_t InCapacity)
            : RHIDescriptorHeap(InType, InCapacity)
        {
            m_FreeList.reserve(InCapacity);
            for (uint32_t i = 0; i < InCapacity; ++i)
                m_FreeList.push_back(i);
        }
        ~DescriptorHeapDirectX11() override = default;

        [[nodiscard]] RHIDescriptorHandle Allocate() override;
        void Free(RHIDescriptorHandle handle) override;

        bool IsFull() const override { return m_FreeList.empty();  }

    private:
        std::vector<uint32_t> m_FreeList;  // 空闲的索引
    };
}
