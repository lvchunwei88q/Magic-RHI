// ========== D3D11RHI/Private/D3D11RHI.cpp ==========

#include <RHIDynamicLoader.h>
#include "RHID3D11.h"

namespace RHI
{
    D3D11RHI_API CreateDevice_Function
    {
        auto device = std::make_unique<DeviceD3D11>();
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

    D3D11RHI_API CreateShaderCompilerBackend_Function
    {
        auto shaderCompilerBackend = std::make_unique<ShaderCompilerBackendD3D11>();
        if (shaderCompilerBackend) {
            return shaderCompilerBackend;
        }
        return nullptr;
    }
    
    D3D11RHI_API GetRHIType_Function
    {
        return RHIType::D3D11;
    }
}