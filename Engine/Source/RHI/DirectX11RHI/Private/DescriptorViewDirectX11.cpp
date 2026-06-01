#include "RHIDirectX11.h"
#include "RHIResourceDirectX11.h"

namespace RHI
{
    RHIDescriptorHandle RHIDirectX11::CreateStandardHeapDescriptorView(RHIBuffer* Buffer,DescriptorRangeType Type)
    {
        RHIDescriptorHandle handle = m_pStandardHeap->Allocate();
        if (!handle.IsValid())
        {
#if RHI_ENABLE_RESOURCE_INFO
            ThrowErrorMessage("Failed to allocate descriptor");
#endif
            return RHIDescriptorHandle();
        }
        
        Buffer->SetBindlessHandle(handle);
        return Buffer->GetBindlessHandle();
    }

    RHIDescriptorHandle RHIDirectX11::CreateStandardHeapDescriptorView(RHITexture* Texture,DescriptorRangeType Type)
    {
        return RHIDescriptorHandle();
    }

    RHIDescriptorHandle RHIDirectX11::CreateSamplerHeapDescriptorView(const SamplerStateDesc& /*desc*/)
    {
        return RHIDescriptorHandle();
    }

    RHIDescriptorHandle RHIDirectX11::CreateRTVHeapDescriptorView(RHIRenderTargetView* /*InView*/)
    {
        return RHIDescriptorHandle();
    }

    RHIDescriptorHandle RHIDirectX11::CreateDSVHeapDescriptorView(RHIDepthStencilView* /*InView*/)
    {
        return RHIDescriptorHandle();
    }
}
