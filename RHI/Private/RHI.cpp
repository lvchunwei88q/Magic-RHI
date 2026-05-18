#include "RHI.h"
#include "DirectX11/RHIDirectX11.h"
#include "DirectX11/SwapChainDirectX11.h"
#include "DirectX12/RHIDirectX12.h"
#include "DirectX12/SwapChainDirectX12.h"

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

    std::unique_ptr<SwapChain> SwapChain::Create(RHIType type)
    {
        switch (type)
        {
        case RHIType::DirectX11:
            return std::make_unique<SwapChainDirectX11>();
        case RHIType::DirectX12:
            return std::make_unique<SwapChainDirectX12>();
        default:
            return nullptr;
        }
    }
}
