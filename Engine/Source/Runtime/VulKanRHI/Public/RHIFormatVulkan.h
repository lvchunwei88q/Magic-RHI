#pragma once

#include <RHIInterface.h>
#include <vulkan.h>

namespace RHI
{
    constexpr VkFormat ToVkFormat(RHITextureFormat Format)
    {
        switch (Format)
        {
        case RHITextureFormat::R8_UNORM: return VK_FORMAT_R8_UNORM;
        case RHITextureFormat::R8_SNORM: return VK_FORMAT_R8_SNORM;
        case RHITextureFormat::R8_UINT: return VK_FORMAT_R8_UINT;
        case RHITextureFormat::R8_SINT: return VK_FORMAT_R8_SINT;
        case RHITextureFormat::R8_SRGB: return VK_FORMAT_R8_SRGB;

        case RHITextureFormat::R16_UNORM: return VK_FORMAT_R16_UNORM;
        case RHITextureFormat::R16_SNORM: return VK_FORMAT_R16_SNORM;
        case RHITextureFormat::R16_UINT: return VK_FORMAT_R16_UINT;
        case RHITextureFormat::R16_SINT: return VK_FORMAT_R16_SINT;
        case RHITextureFormat::R16_FLOAT: return VK_FORMAT_R16_SFLOAT;

        case RHITextureFormat::R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
        case RHITextureFormat::R8G8_SNORM: return VK_FORMAT_R8G8_SNORM;
        case RHITextureFormat::R8G8_UINT: return VK_FORMAT_R8G8_UINT;
        case RHITextureFormat::R8G8_SINT: return VK_FORMAT_R8G8_SINT;
        case RHITextureFormat::R8G8_SRGB: return VK_FORMAT_R8G8_SRGB;

        case RHITextureFormat::R16G16_UNORM: return VK_FORMAT_R16G16_UNORM;
        case RHITextureFormat::R16G16_SNORM: return VK_FORMAT_R16G16_SNORM;
        case RHITextureFormat::R16G16_UINT: return VK_FORMAT_R16G16_UINT;
        case RHITextureFormat::R16G16_SINT: return VK_FORMAT_R16G16_SINT;
        case RHITextureFormat::R16G16_FLOAT: return VK_FORMAT_R16G16_SFLOAT;

        case RHITextureFormat::R32G32_UINT: return VK_FORMAT_R32G32_UINT;
        case RHITextureFormat::R32G32_SINT: return VK_FORMAT_R32G32_SINT;
        case RHITextureFormat::R32G32_FLOAT: return VK_FORMAT_R32G32_SFLOAT;

        case RHITextureFormat::R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
        case RHITextureFormat::R8G8B8A8_SNORM: return VK_FORMAT_R8G8B8A8_SNORM;
        case RHITextureFormat::R8G8B8A8_UINT: return VK_FORMAT_R8G8B8A8_UINT;
        case RHITextureFormat::R8G8B8A8_SINT: return VK_FORMAT_R8G8B8A8_SINT;
        case RHITextureFormat::R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;

        case RHITextureFormat::R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
        case RHITextureFormat::R16G16B16A16_SNORM: return VK_FORMAT_R16G16B16A16_SNORM;
        case RHITextureFormat::R16G16B16A16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
        case RHITextureFormat::R16G16B16A16_SINT: return VK_FORMAT_R16G16B16A16_SINT;
        case RHITextureFormat::R16G16B16A16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;

        case RHITextureFormat::R32G32B32_UINT: return VK_FORMAT_R32G32B32_UINT;
        case RHITextureFormat::R32G32B32_SINT: return VK_FORMAT_R32G32B32_SINT;
        case RHITextureFormat::R32G32B32_FLOAT: return VK_FORMAT_R32G32B32_SFLOAT;

        case RHITextureFormat::R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
        case RHITextureFormat::R32G32B32A32_SINT: return VK_FORMAT_R32G32B32A32_SINT;
        case RHITextureFormat::R32G32B32A32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;

        case RHITextureFormat::R10G10B10A2_UNORM: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case RHITextureFormat::R10G10B10A2_UINT: return VK_FORMAT_A2B10G10R10_UINT_PACK32;
        case RHITextureFormat::R11G11B10_FLOAT: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case RHITextureFormat::B5G6R5_UNORM: return VK_FORMAT_B5G6R5_UNORM_PACK16;
        case RHITextureFormat::B5G5R5A1_UNORM: return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
        case RHITextureFormat::B4G4R4A4_UNORM: return VK_FORMAT_B4G4R4A4_UNORM_PACK16;

        case RHITextureFormat::B8G8R8A8_UNORM:
        case RHITextureFormat::B8G8R8X8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case RHITextureFormat::B8G8R8A8_SRGB:
        case RHITextureFormat::B8G8R8X8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;

        case RHITextureFormat::D16_UNORM: return VK_FORMAT_D16_UNORM;
        case RHITextureFormat::D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
        case RHITextureFormat::D32_FLOAT: return VK_FORMAT_D32_SFLOAT;
        case RHITextureFormat::D32_FLOAT_S8X24_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;

        case RHITextureFormat::BC1_UNORM: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case RHITextureFormat::BC1_UNORM_SRGB: return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case RHITextureFormat::BC2_UNORM: return VK_FORMAT_BC2_UNORM_BLOCK;
        case RHITextureFormat::BC2_UNORM_SRGB: return VK_FORMAT_BC2_SRGB_BLOCK;
        case RHITextureFormat::BC3_UNORM: return VK_FORMAT_BC3_UNORM_BLOCK;
        case RHITextureFormat::BC3_UNORM_SRGB: return VK_FORMAT_BC3_SRGB_BLOCK;
        case RHITextureFormat::BC4_UNORM: return VK_FORMAT_BC4_UNORM_BLOCK;
        case RHITextureFormat::BC4_SNORM: return VK_FORMAT_BC4_SNORM_BLOCK;
        case RHITextureFormat::BC5_UNORM: return VK_FORMAT_BC5_UNORM_BLOCK;
        case RHITextureFormat::BC5_SNORM: return VK_FORMAT_BC5_SNORM_BLOCK;
        case RHITextureFormat::BC6H_UF16: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case RHITextureFormat::BC6H_SF16: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case RHITextureFormat::BC7_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
        case RHITextureFormat::BC7_UNORM_SRGB: return VK_FORMAT_BC7_SRGB_BLOCK;

        case RHITextureFormat::R8G8B8A8_TYPELESS: return VK_FORMAT_R8G8B8A8_UNORM;
        case RHITextureFormat::R16G16B16A16_TYPELESS: return VK_FORMAT_R16G16B16A16_UNORM;
        case RHITextureFormat::R32G32B32A32_TYPELESS: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case RHITextureFormat::R32_TYPELESS: return VK_FORMAT_R32_SFLOAT;

        case RHITextureFormat::Unknown:
        case RHITextureFormat::Max:
        default:
            return VK_FORMAT_UNDEFINED;
        }
    }
}
