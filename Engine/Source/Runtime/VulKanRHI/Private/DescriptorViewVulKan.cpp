#include "RHIVulKan.h"

namespace RHI
{
    namespace
    {
    }
    
    RHIDescriptorHandle DeviceVulKan::CreateStandardHeapDescriptorView(RHIBuffer* Buffer, DescriptorRangeType Type)
    {
        return {};
    }

    RHIDescriptorHandle DeviceVulKan::CreateStandardHeapDescriptorView(RHITexture* Texture, DescriptorRangeType Type)
    {
        return {};
    }

    RHIDescriptorHandle DeviceVulKan::CreateSamplerHeapDescriptorView(const SamplerStateDesc& desc)
    {
        return {};
    }

    RHIDescriptorHandle DeviceVulKan::CreateRTVHeapDescriptorView(RHITexture* Texture)
    {
        return {};
    }

    RHIDescriptorHandle DeviceVulKan::CreateDSVHeapDescriptorView(RHITexture* Texture)
    {
        return {};
    }
}
