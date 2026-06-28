// ========== D3D12RHI/Private/D3D12RHI.cpp ==========

#include <RHIDynamicLoader.h>
#include "RHID3D12.h"

namespace RHI
{
    D3D12RHI_API CreateDevice_Function
    {
        auto device = std::make_unique<DeviceD3D12>();
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
    
    D3D12RHI_API CreateCreateShader_Function
    {
        auto createShader = std::make_unique<CreateShaderD3D12>();
        if (createShader) {
            return createShader;
        }
        return nullptr;
    }
    
    D3D12RHI_API GetRHIType_Function
    {
        return RHIType::D3D12;
    }
}