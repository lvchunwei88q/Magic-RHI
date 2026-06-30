// ========== D3D12RHI/Private/D3D12RHI.cpp ==========

#include <RHIDynamicLoader.h>
#include "RHIVulKan.h"

namespace RHI
{
    VULKANRHI_API CreateDevice_Function
    {
        auto device = std::make_unique<DeviceVulKan>();
        if (device) {
            return device;
        }
        return nullptr;
    }
    
    VULKANRHI_API CreateSwapChain_Function
    {
        auto swapChain = std::make_unique<SwapChainVulKan>();
        if (swapChain) {
            return swapChain;
        }
        return nullptr;
    }
    
    VULKANRHI_API CreateShaderCompilerBackend_Function
    {
        auto shaderCompilerBackend = std::make_unique<ShaderCompilerBackendVulKan>();
        if (shaderCompilerBackend) {
            return shaderCompilerBackend;
        }
        return nullptr;
    }
    
    VULKANRHI_API GetRHIType_Function
    {
        return RHIType::VulKan;
    }
}