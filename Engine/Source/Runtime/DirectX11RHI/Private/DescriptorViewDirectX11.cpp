#include "RHIDirectX11.h"
#include "RHIResourceDirectX11.h"

namespace RHI
{
    RHIDescriptorHandle RHIDirectX11::CreateStandardHeapDescriptorView(RHIBuffer* Buffer,DescriptorRangeType Type)
    {
        if (!m_pStandardHeap || m_pStandardHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }
        
        auto dx11Buffer = SafeCast<BufferDirectX11>(Buffer);
        RHIDescriptorHandle handle = m_pStandardHeap->Allocate();
        if (!handle.IsValid())
        {
#if RHI_ENABLE_RESOURCE_INFO
            ThrowErrorMessage("Failed to allocate descriptor");
#endif
            return RHIDescriptorHandle();
        }
        
        dx11Buffer->SetBindlessHandle(handle);
        m_pStandardHeap->SetDescriptor(handle, dx11Buffer);
        return handle;
    }

    RHIDescriptorHandle RHIDirectX11::CreateStandardHeapDescriptorView(RHITexture* Texture,DescriptorRangeType Type)
    {
        if (!m_pStandardHeap || m_pStandardHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pStandardHeap->Allocate();
    }

    RHIDescriptorHandle RHIDirectX11::CreateSamplerHeapDescriptorView(const SamplerStateDesc& /*desc*/)
    {
        if (!m_pSamplerHeap || m_pSamplerHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pSamplerHeap->Allocate();
    }

    RHIDescriptorHandle RHIDirectX11::CreateRTVHeapDescriptorView(RHIRenderTargetView* /*InView*/)
    {
        if (!m_pRTVHeap || m_pRTVHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pRTVHeap->Allocate();
    }

    RHIDescriptorHandle RHIDirectX11::CreateDSVHeapDescriptorView(RHIDepthStencilView* /*InView*/)
    {
        if (!m_pDSVHeap || m_pDSVHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pDSVHeap->Allocate();
    }
}
