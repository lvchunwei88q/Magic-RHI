// ========== D3D12RHI/Private/D3D12RHI.cpp ==========

#include <RHIDynamicLoader.h>
#include "RHID3D12.h"
#include "SwapChainD3D12.h"

namespace RHI
{
    D3D12RHI_API CreateDevice_Function
    {
        auto device = std::make_unique<RHID3D12>();
        if (device) {
            return device;
        }
        return nullptr;
    }
    
    D3D12RHI_API CreateSwapChain_Function
    {
        auto swapChain = std::make_unique<SwapChainD3D12>();
        if (swapChain) {
            return swapChain;
        }
        return nullptr;
    }
    
    D3D12RHI_API GetRHIType_Function
    {
        return RHIType::D3D12;
    }
}