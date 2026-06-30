#include "RHIVulKan.h"

namespace RHI
{
    namespace
    {
    }

    std::shared_ptr<RHISamplerState> DeviceVulKan::CreateSamplerState(const SamplerStateDesc& desc)
    {
        return nullptr;
    }

    void DeviceVulKan::DeleteSamplerState(std::shared_ptr<RHISamplerState>& samplerState)
    {
    }

    std::shared_ptr<RHIBuffer> DeviceVulKan::CreateBuffer(BufferDesc& desc)
    {
        return nullptr;
    }

    void DeviceVulKan::DeleteBuffer(std::shared_ptr<RHIBuffer>& buffer)
    {
    }

}