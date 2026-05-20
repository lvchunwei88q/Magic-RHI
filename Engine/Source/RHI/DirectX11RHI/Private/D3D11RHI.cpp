// ========== D3D12RHI/Private/D3D12RHI.cpp ==========

#include <RHIDynamicLoader.h>
#include "RHIDirectX11.h"
#include "SwapChainDirectX11.h"

namespace RHI
{
    DIRECTX11RHI_API CreateDevice_Function
    {
        return std::make_unique<RHIDirectX11>();
    }
    
    DIRECTX11RHI_API CreateSwapChain_Function
    {
        RHIDirectX11* d3d11Device = static_cast<RHIDirectX11*>(device);
        auto swapChain = std::make_unique<SwapChainDirectX11>();
        if (swapChain) {
            return swapChain;
        }
        return nullptr;
    }
    
    DIRECTX11RHI_API GetRHIType_Function
    {
        return RHIType::DirectX11;
    }
}