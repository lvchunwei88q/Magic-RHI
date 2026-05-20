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
}