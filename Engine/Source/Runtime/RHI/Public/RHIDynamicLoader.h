// ========== RHI/Public/RHIDynamicLoader.h ==========

#pragma once
#include <memory>
#include "RHIInterface.h"

/*
 * Note: Here we use def + cmake to ensure C name mangling,
 * otherwise the name mangling when calling the function may not match the name mangling of the exported DLL functions, which can cause function calls to fail.
 * Syntax similar:
 * target_link_options(Target PRIVATE /DEF:${CMAKE_CURRENT_SOURCE_DIR}/Private/TargetFile.def)
**/

namespace RHI
{
    // DLL Export function type definition
    typedef std::unique_ptr<Device>         (*PFN_CreateDevice)();
    typedef std::unique_ptr<SwapChain>      (*PFN_CreateSwapChain)();
    typedef std::unique_ptr<CreateShader>   (*PFN_CreateCreateShader)();
    typedef RHIType                         (*PFN_GetRHIType)();
}

// Macro definition to unify function call methods
#define CreateDevice_Function           std::unique_ptr<RHI::Device> CreateDevice()
#define CreateSwapChain_Function        std::unique_ptr<RHI::SwapChain> CreateSwapChain()
#define CreateCreateShader_Function     std::unique_ptr<RHI::CreateShader> CreateCreateShader()
#define GetRHIType_Function             RHI::RHIType GetRHIType()
