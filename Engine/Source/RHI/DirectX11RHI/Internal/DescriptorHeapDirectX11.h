#pragma once
#include <RHIResource.h>

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
    };
}
