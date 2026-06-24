// ========== D3D12RHI/Private/D3D12RHI.cpp ==========

#include <RHIDynamicLoader.h>
#include "RHID3D11.h"
#include "SwapChainD3D11.h"

namespace RHI
{
    D3D11RHI_API CreateDevice_Function
    {
        auto device = std::make_unique<RHID3D11>();
        if (device) {
            return device;
        }
        return nullptr;
    }
    
    D3D11RHI_API CreateSwapChain_Function
    {
        auto swapChain = std::make_unique<SwapChainD3D11>();
        if (swapChain) {
            return swapChain;
        }
        return nullptr;
    }
    
    D3D11RHI_API GetRHIType_Function
    {
        return RHIType::D3D11;
    }
}