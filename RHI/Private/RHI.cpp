#include "RHI.h"
#include <DirectX11/RHIDirectX11.h>
#include <DirectX12/RHIDirectX12.h>

namespace RHI
{
    std::unique_ptr<Device> Device::Create(RHIType type)
    {
        switch (type)
        {
        case RHIType::DirectX11:
            return std::make_unique<RHIDirectX11>();
        case RHIType::DirectX12:
            return std::make_unique<RHIDirectX12>();
        default:
            return nullptr;
        }
    }
}
