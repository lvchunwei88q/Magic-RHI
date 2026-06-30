#include "RHIVulKan.h"

namespace RHI
{
    namespace
    {
    }

    DeviceVulKan::DeviceVulKan()
    {
    }
    
    DeviceVulKan::~DeviceVulKan()
    {
        Shutdown();
    }

    bool DeviceVulKan::Initialize()
    {
        return true;
    }

    void DeviceVulKan::Shutdown()
    {
    }

    bool DeviceVulKan::IsValid() const
    {
        return true;
    }

    FeatureLevel DeviceVulKan::GetFeatureLevel() const
    {
        return FeatureLevel::Level_12_2;
    }
}
