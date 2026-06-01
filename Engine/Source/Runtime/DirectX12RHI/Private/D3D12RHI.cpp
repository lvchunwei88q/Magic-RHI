// ========== D3D12RHI/Private/D3D12RHI.cpp ==========

#include <RHIDynamicLoader.h>
#include "RHIDirectX12.h"
#include "SwapChainDirectX12.h"

namespace RHI
{
    DIRECTX12RHI_API CreateDevice_Function
    {
        auto device = std::make_unique<RHIDirectX12>();
        if (device) {
            return device;
        }
        return nullptr;
    }
    
    DIRECTX12RHI_API CreateSwapChain_Function
    {
        auto swapChain = std::make_unique<SwapChainDirectX12>();
        if (swapChain) {
            return swapChain;
        }
        return nullptr;
    }
    
    DIRECTX12RHI_API GetRHIType_Function
    {
        return RHIType::DirectX12;
    }
}