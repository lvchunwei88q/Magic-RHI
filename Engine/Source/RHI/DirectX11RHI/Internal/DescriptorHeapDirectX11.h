#pragma once
#include <RHIResource.h>
#include <vector>

namespace RHI
{
    class DescriptorHeapDirectX11 : public RHIDescriptorHeap
    {
    public:
        DescriptorHeapDirectX11(RHIDescriptorHeapType InType)
            : RHIDescriptorHeap(InType,0)
        {}
        ~DescriptorHeapDirectX11() override = default;

        [[nodiscard]] RHIDescriptorHandle Allocate() override;
        void Free(RHIDescriptorHandle handle) override;

        bool IsFull() const override { return false; }

    private:
        std::vector<uint32_t> m_pHeap;
        uint32_t m_DescriptorSize;

        std::vector<uint32_t> m_FreeList;  // 空闲的索引
    };
}
