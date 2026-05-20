#pragma once

#include <dxgiformat.h>
#include <d3dcommon.h>

#include "Common/RHIResourceType.h"

#define DXGI_RTV_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM

inline RHI::FeatureLevel FromD3DFeatureLevel(D3D_FEATURE_LEVEL level)
{
    switch (level)
    {
    case D3D_FEATURE_LEVEL_9_1:  return RHI::FeatureLevel::Level_9_1;
    case D3D_FEATURE_LEVEL_9_2:  return RHI::FeatureLevel::Level_9_2;
    case D3D_FEATURE_LEVEL_9_3:  return RHI::FeatureLevel::Level_9_3;
    case D3D_FEATURE_LEVEL_10_0: return RHI::FeatureLevel::Level_10_0;
    case D3D_FEATURE_LEVEL_10_1: return RHI::FeatureLevel::Level_10_1;
    case D3D_FEATURE_LEVEL_11_0: return RHI::FeatureLevel::Level_11_0;
    case D3D_FEATURE_LEVEL_11_1: return RHI::FeatureLevel::Level_11_1;
    case D3D_FEATURE_LEVEL_12_0: return RHI::FeatureLevel::Level_12_0;
    case D3D_FEATURE_LEVEL_12_1: return RHI::FeatureLevel::Level_12_1;
    case D3D_FEATURE_LEVEL_12_2: return RHI::FeatureLevel::Level_12_2;
    
    default:
        return RHI::FeatureLevel::Level_11_0;  // 默认值
    }
}