// ========== RHI/Public/RHIDynamicLoader.h ==========

#pragma once
#include <memory>
#include "RHI.h"

namespace RHI
{
    // DLL 导出函数类型定义
    typedef std::unique_ptr<Device> (*PFN_CreateDevice)();
    typedef std::unique_ptr<SwapChain> (*PFN_CreateSwapChain)(Device* device, const SwapChainDesc& desc);
    typedef RHIType (*PFN_GetRHIType)();
}

#define CreateDevice_Function  std::unique_ptr<RHI::Device> CreateDevice()
#define CreateSwapChain_Function  std::unique_ptr<RHI::SwapChain> CreateSwapChain(RHI::Device* device, const RHI::SwapChainDesc& desc)
#define GetRHIType_Function  RHI::RHIType GetRHIType()