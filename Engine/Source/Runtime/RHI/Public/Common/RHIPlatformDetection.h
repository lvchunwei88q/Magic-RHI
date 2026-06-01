#pragma once
// 现在只支持Windows 平台 因为MagicToonEngine支持Windows平台
#include <cstdint>
#include "Common/RHI_API.h"

namespace RHI
{
    // RHI 类型枚举
    enum class RHIType : uint8_t
    {
        Unknown = 0,
        DirectX11,      // DirectX 11
        DirectX12,      // DirectX 12
        // More RHI types
    };

    RHIType RHI_API GetBestAvailableRHI();
    
    inline bool IsMultiThreadingSupported(RHIType type) {
        switch (type) {
            case RHIType::DirectX12:
                return true;  // 现代 API，原生支持多线程录制
                
            case RHIType::DirectX11:
                return false; // 传统 API，需要特殊处理或禁用
                
            default:
                return false;
        }
    }
}