#pragma once

#include <Windows.h>
#include <dxgi1_6.h>
#include <RHIInterface.h>

namespace RHI
{
    constexpr DXGI_FORMAT ToDXGIFormat(RHITextureFormat Format)
    {
        switch (Format)
        {
        case RHITextureFormat::R8_UNORM: return DXGI_FORMAT_R8_UNORM;
        case RHITextureFormat::R8_SNORM: return DXGI_FORMAT_R8_SNORM;
        case RHITextureFormat::R8_UINT: return DXGI_FORMAT_R8_UINT;
        case RHITextureFormat::R8_SINT: return DXGI_FORMAT_R8_SINT;
        case RHITextureFormat::R8_SRGB: return DXGI_FORMAT_R8_UNORM;

        case RHITextureFormat::R16_UNORM: return DXGI_FORMAT_R16_UNORM;
        case RHITextureFormat::R16_SNORM: return DXGI_FORMAT_R16_SNORM;
        case RHITextureFormat::R16_UINT: return DXGI_FORMAT_R16_UINT;
        case RHITextureFormat::R16_SINT: return DXGI_FORMAT_R16_SINT;
        case RHITextureFormat::R16_FLOAT: return DXGI_FORMAT_R16_FLOAT;

        case RHITextureFormat::R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
        case RHITextureFormat::R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
        case RHITextureFormat::R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
        case RHITextureFormat::R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
        case RHITextureFormat::R8G8_SRGB: return DXGI_FORMAT_R8G8_UNORM;

        case RHITextureFormat::R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
        case RHITextureFormat::R16G16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
        case RHITextureFormat::R16G16_UINT: return DXGI_FORMAT_R16G16_UINT;
        case RHITextureFormat::R16G16_SINT: return DXGI_FORMAT_R16G16_SINT;
        case RHITextureFormat::R16G16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;

        case RHITextureFormat::R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
        case RHITextureFormat::R32G32_SINT: return DXGI_FORMAT_R32G32_SINT;
        case RHITextureFormat::R32G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;

        case RHITextureFormat::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case RHITextureFormat::R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
        case RHITextureFormat::R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
        case RHITextureFormat::R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
        case RHITextureFormat::R8G8B8A8_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

        case RHITextureFormat::R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
        case RHITextureFormat::R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
        case RHITextureFormat::R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
        case RHITextureFormat::R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
        case RHITextureFormat::R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;

        case RHITextureFormat::R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
        case RHITextureFormat::R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
        case RHITextureFormat::R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;

        case RHITextureFormat::R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
        case RHITextureFormat::R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
        case RHITextureFormat::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;

        case RHITextureFormat::R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
        case RHITextureFormat::R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_UINT;
        case RHITextureFormat::R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
        case RHITextureFormat::B5G6R5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
        case RHITextureFormat::B5G5R5A1_UNORM: return DXGI_FORMAT_B5G5R5A1_UNORM;
        case RHITextureFormat::B4G4R4A4_UNORM: return DXGI_FORMAT_B4G4R4A4_UNORM;

        case RHITextureFormat::B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
        case RHITextureFormat::B8G8R8A8_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        case RHITextureFormat::B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
        case RHITextureFormat::B8G8R8X8_SRGB: return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

        case RHITextureFormat::D16_UNORM: return DXGI_FORMAT_D16_UNORM;
        case RHITextureFormat::D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case RHITextureFormat::D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
        case RHITextureFormat::D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

        case RHITextureFormat::BC1_UNORM: return DXGI_FORMAT_BC1_UNORM;
        case RHITextureFormat::BC1_UNORM_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
        case RHITextureFormat::BC2_UNORM: return DXGI_FORMAT_BC2_UNORM;
        case RHITextureFormat::BC2_UNORM_SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;
        case RHITextureFormat::BC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
        case RHITextureFormat::BC3_UNORM_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
        case RHITextureFormat::BC4_UNORM: return DXGI_FORMAT_BC4_UNORM;
        case RHITextureFormat::BC4_SNORM: return DXGI_FORMAT_BC4_SNORM;
        case RHITextureFormat::BC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
        case RHITextureFormat::BC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
        case RHITextureFormat::BC6H_UF16: return DXGI_FORMAT_BC6H_UF16;
        case RHITextureFormat::BC6H_SF16: return DXGI_FORMAT_BC6H_SF16;
        case RHITextureFormat::BC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
        case RHITextureFormat::BC7_UNORM_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;

        case RHITextureFormat::R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
        case RHITextureFormat::R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_TYPELESS;
        case RHITextureFormat::R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
        case RHITextureFormat::R32_TYPELESS: return DXGI_FORMAT_R32_TYPELESS;

        case RHITextureFormat::Unknown:
        case RHITextureFormat::Max:
        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }
}
