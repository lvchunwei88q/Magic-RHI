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
    // ============================================================
    // DirectX 12 Feature Levels
    // ============================================================
    D3D12_11_0   = 0xB000,   // DirectX 12, Feature Level 11.0 (SM 5.0)
    D3D12_11_1   = 0xB100,   // DirectX 12, Feature Level 11.1 (SM 5.0+)
    D3D12_12_0   = 0xC000,   // DirectX 12, Feature Level 12.0 (SM 5.1)
    D3D12_12_1   = 0xC100,   // DirectX 12, Feature Level 12.1 (SM 5.1 + ROV)
    D3D12_12_2   = 0xC200,   // DirectX 12 Ultimate, Feature Level 12.2 (SM 6.0 + RT + Mesh)

    // ============================================================
    // Vulkan API Versions
    // ============================================================
    Vulkan_1_0   = 0x010000, // Vulkan 1.0
    Vulkan_1_1   = 0x010100, // Vulkan 1.1
    Vulkan_1_2   = 0x010200, // Vulkan 1.2
    Vulkan_1_3   = 0x010300, // Vulkan 1.3
    Vulkan_1_4   = 0x010400, // Vulkan 1.4
};

inline const char* GetFeatureLevelName(FeatureLevel level)
{
    switch (level)
    {
    // ============================================================
    // DirectX 12 Feature Levels
    // ============================================================
    case FeatureLevel::D3D12_11_0: return "DirectX 12.0 - Feature Level 11.0";
    case FeatureLevel::D3D12_11_1: return "DirectX 12.0 - Feature Level 11.1";
    case FeatureLevel::D3D12_12_0: return "DirectX 12.0 - Feature Level 12.0";
    case FeatureLevel::D3D12_12_1: return "DirectX 12.1 - Feature Level 12.1";
    case FeatureLevel::D3D12_12_2: return "DirectX 12 Ultimate - Feature Level 12.2";

    // ============================================================
    // Vulkan API Versions
    // ============================================================
    case FeatureLevel::Vulkan_1_0: return "Vulkan 1.0";
    case FeatureLevel::Vulkan_1_1: return "Vulkan 1.1";
    case FeatureLevel::Vulkan_1_2: return "Vulkan 1.2";
    case FeatureLevel::Vulkan_1_3: return "Vulkan 1.3";
    case FeatureLevel::Vulkan_1_4: return "Vulkan 1.4";

    // ============================================================
    // Unknown
    // ============================================================
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