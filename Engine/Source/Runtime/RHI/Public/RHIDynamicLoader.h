// ========== RHI/Public/RHIDynamicLoader.h ==========

#pragma once
#include <memory>
#include "RHI.h"

/*
*注意这里使用def+cmake的方式保证C++的名称修饰，否则会导致函数调用时的名称修饰与DLL导出的
*函数调用时的名称修饰不一致，从而导致函数调用失败。
*语法类似:
*target_link_options(Target PRIVATE 
*    /DEF:${CMAKE_CURRENT_SOURCE_DIR}/Private/TargetFile.def
*)
**/

namespace RHI
{
    // DLL 导出函数类型定义
    typedef std::unique_ptr<Device> (*PFN_CreateDevice)();
    typedef std::unique_ptr<SwapChain> (*PFN_CreateSwapChain)();
    typedef RHIType (*PFN_GetRHIType)();
}

// 使用宏定义统一函数调用方式
#define CreateDevice_Function     std::unique_ptr<RHI::Device> CreateDevice()
#define CreateSwapChain_Function  std::unique_ptr<RHI::SwapChain> CreateSwapChain()
#define GetRHIType_Function       RHI::RHIType GetRHIType()
