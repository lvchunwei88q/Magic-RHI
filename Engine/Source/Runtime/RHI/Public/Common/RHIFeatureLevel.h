#pragma once
#include <cstdint>
#include <Tools/EnumClassFlags.h>

#define MAX_uint32_t 0xFFFFFFFF
#define MAX_uint8_t 0xFF

namespace RHI
{
    /* Feature Level Type */
enum class FeatureLevel : uint32_t
{
	// DirectX 9 Level
	Level_9_1 = 0x9100,   // Shader Model 2.0
	Level_9_2 = 0x9200,   // Shader Model 2.0+ 
	Level_9_3 = 0x9300,   // Shader Model 3.0
	// DirectX 10 Level
	Level_10_0 = 0xa000,  // Shader Model 4.0
	Level_10_1 = 0xa100,  // Shader Model 4.1
	// DirectX 11 Level
	Level_11_0 = 0xb000,  // Shader Model 5.0
	Level_11_1 = 0xb100,  // Shader Model 5.0 + More Features
	// DirectX 12 Level
	Level_12_0 = 0xc000,  // Shader Model 5.1
	Level_12_1 = 0xc100,  // Shader Model 5.1 + Rasterizer Order Independent
	Level_12_2 = 0xc200   // Shader Model 6.0 + Ray Tracing + Mesh Shaders
	// More ...
};

inline const char* GetFeatureLevelName(FeatureLevel level)
{
	switch (level)
	{
	case FeatureLevel::Level_9_1:  return "DirectX 9.1";
	case FeatureLevel::Level_9_2:  return "DirectX 9.2";
	case FeatureLevel::Level_9_3:  return "DirectX 9.3";
	case FeatureLevel::Level_10_0: return "DirectX 10.0";
	case FeatureLevel::Level_10_1: return "DirectX 10.1";
	case FeatureLevel::Level_11_0: return "DirectX 11.0";
	case FeatureLevel::Level_11_1: return "DirectX 11.1";
	case FeatureLevel::Level_12_0: return "DirectX 12.0";
	case FeatureLevel::Level_12_1: return "DirectX 12.1";
	case FeatureLevel::Level_12_2: return "DirectX 12.2 (Ultimate)";
	// More ...
	default: return "Unknown";
	}
}

// Shader Model Level
enum class ShaderModelVersion
{
    SM_5_0 = 50,    // 5.0
    SM_5_1 = 51,    // 5.1
    SM_6_0 = 60,    // 6.0
    SM_6_1 = 61,    // 6.1
    SM_6_2 = 62,    // 6.2
    SM_6_3 = 63,    // 6.3
    SM_6_4 = 64,    // 6.4
    SM_6_5 = 65,    // 6.5
    SM_6_6 = 66,    // 6.6
    SM_6_7 = 67,    // 6.7
    SM_6_8 = 68,    // 6.8
    SM_6_9 = 69,    // 6.9
    Unknown = 0
};

inline const char* ShaderModelToString(ShaderModelVersion version)
{
    switch (version)
    {
    case ShaderModelVersion::SM_5_0: return "5_0";
    case ShaderModelVersion::SM_5_1: return "5_1";
    case ShaderModelVersion::SM_6_0: return "6_0";
    case ShaderModelVersion::SM_6_1: return "6_1";
    case ShaderModelVersion::SM_6_2: return "6_2";
    case ShaderModelVersion::SM_6_3: return "6_3";
    case ShaderModelVersion::SM_6_4: return "6_4";
    case ShaderModelVersion::SM_6_5: return "6_5";
    case ShaderModelVersion::SM_6_6: return "6_6";
    case ShaderModelVersion::SM_6_7: return "6_7";
    case ShaderModelVersion::SM_6_8: return "6_8";
    case ShaderModelVersion::SM_6_9: return "6_9";
    default: return "Unknown";
    }
}

inline size_t ShaderModelToNumber(ShaderModelVersion version)
{
    switch (version)
    {
    case ShaderModelVersion::SM_5_0: return 50;
    case ShaderModelVersion::SM_5_1: return 51;
    case ShaderModelVersion::SM_6_0: return 60;
    case ShaderModelVersion::SM_6_1: return 61;
    case ShaderModelVersion::SM_6_2: return 62;
    case ShaderModelVersion::SM_6_3: return 63;
    case ShaderModelVersion::SM_6_4: return 64;
    case ShaderModelVersion::SM_6_5: return 65;
    case ShaderModelVersion::SM_6_6: return 66;
    case ShaderModelVersion::SM_6_7: return 67;
    case ShaderModelVersion::SM_6_8: return 68;
    case ShaderModelVersion::SM_6_9: return 69;
    default: return 0;
    }
}
}