#include "DescriptorHeapDirectX11.h"
#include "RHIDirectX11.h"
#include "DirectXHelper.h"

namespace RHI
{
    RHIDescriptorHandle DescriptorHeapDirectX11::Allocate()
    {
        if (IsFull())
        {
            ThrowErrorMessage("Descriptor heap is full");
            return RHIDescriptorHandle();
        }
        return RHIDescriptorHandle(HeapType, 0);
    }

    void DescriptorHeapDirectX11::Free(RHIDescriptorHandle handle)
    {
    }
}
