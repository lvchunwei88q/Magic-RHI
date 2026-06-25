#pragma once
// Windows is the only platform
#include <cstdint>
#include "Common/RHI_API.h"

namespace RHI
{
    // RHI Type Enum
    enum class RHIType : uint8_t
    {
        Unknown = 0,
        D3D11,      // DirectX 11
        D3D12,      // DirectX 12
        // More RHI types
    };

    inline const char* ToString(RHIType type)
    {
        switch (type)
        {
            case RHIType::Unknown:      return "Unknown";
            case RHIType::D3D11:    return "D3D11";
            case RHIType::D3D12:    return "D3D12";
            default:                    return "Invalid";
        }
    }

    RHIType RHI_API GetBestAvailableRHI();
    
    inline bool IsMultiThreadingSupported(RHIType type) {
        switch (type) {
            case RHIType::D3D12:
                return true;  // 现代 API，原生支持多线程录制
                
            case RHIType::D3D11:
                return false; // 传统 API，需要特殊处理或禁用
                
            default:
                return false;
        }
    }
}