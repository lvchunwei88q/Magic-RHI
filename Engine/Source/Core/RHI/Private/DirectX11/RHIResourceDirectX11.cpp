#include "DirectX11/RHIDirectX11.h"
#include "DirectX11/RHIResourceDirectX11.h"

namespace RHI
{
    namespace
    {
        // D3D11 转换
        D3D11_USAGE ToD3D11Usage(BufferHeapType type)
        {
            switch (type)
            {
            case BufferHeapType::Default:  return D3D11_USAGE_DEFAULT;
            case BufferHeapType::Upload:   return D3D11_USAGE_DYNAMIC;
            case BufferHeapType::Readback: return D3D11_USAGE_STAGING;
            default:                       return D3D11_USAGE_DEFAULT;
            }
        }

        UINT ToD3D11CPUAccessFlags(BufferHeapType type)
        {
            switch (type)
            {
            case BufferHeapType::Default:  return 0;
            case BufferHeapType::Upload:   return D3D11_CPU_ACCESS_WRITE;
            case BufferHeapType::Readback: return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
            default:                       return 0;
            }
        }

        D3D11_FILTER ConvertFilter(SamplerFilter filter)
        {
            switch (filter)
            {
            case SamplerFilter::Point:
                return D3D11_FILTER_MIN_MAG_MIP_POINT;
            case SamplerFilter::Bilinear:
                return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            case SamplerFilter::Trilinear:
                return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            case SamplerFilter::Anisotropic:
                return D3D11_FILTER_ANISOTROPIC;
            default:
                return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            }
        }

        D3D11_TEXTURE_ADDRESS_MODE ConvertAddressMode(SamplerAddressMode mode)
        {
            switch (mode)
            {
            case SamplerAddressMode::Wrap:
                return D3D11_TEXTURE_ADDRESS_WRAP;
            case SamplerAddressMode::Mirror:
                return D3D11_TEXTURE_ADDRESS_MIRROR;
            case SamplerAddressMode::Clamp:
                return D3D11_TEXTURE_ADDRESS_CLAMP;
            case SamplerAddressMode::Border:
                return D3D11_TEXTURE_ADDRESS_BORDER;
            case SamplerAddressMode::MirrorOnce:
                return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
            default:
                return D3D11_TEXTURE_ADDRESS_WRAP;
            }
        }

        D3D11_COMPARISON_FUNC ConvertComparisonFunc(SamplerComparisonFunc func)
        {
            switch (func)
            {
            case SamplerComparisonFunc::Never:
                return D3D11_COMPARISON_NEVER;
            case SamplerComparisonFunc::Less:
                return D3D11_COMPARISON_LESS;
            case SamplerComparisonFunc::Equal:
                return D3D11_COMPARISON_EQUAL;
            case SamplerComparisonFunc::LessEqual:
                return D3D11_COMPARISON_LESS_EQUAL;
            case SamplerComparisonFunc::Greater:
                return D3D11_COMPARISON_GREATER;
            case SamplerComparisonFunc::NotEqual:
                return D3D11_COMPARISON_NOT_EQUAL;
            case SamplerComparisonFunc::GreaterEqual:
                return D3D11_COMPARISON_GREATER_EQUAL;
            case SamplerComparisonFunc::Always:
                return D3D11_COMPARISON_ALWAYS;
            default:
                return D3D11_COMPARISON_NEVER;
            }
        }
    }

    std::shared_ptr<RHISamplerState> RHIDirectX11::CreateSamplerState(const SamplerStateDesc& desc)
    {
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = ConvertFilter(desc.Filter);
        samplerDesc.AddressU = ConvertAddressMode(desc.AddressU);
        samplerDesc.AddressV = ConvertAddressMode(desc.AddressV);
        samplerDesc.AddressW = ConvertAddressMode(desc.AddressW);
        samplerDesc.MipLODBias = desc.MipLODBias;
        samplerDesc.MaxAnisotropy = desc.MaxAnisotropy;
        samplerDesc.ComparisonFunc = ConvertComparisonFunc(desc.ComparisonFunc);
        samplerDesc.BorderColor[0] = desc.BorderColor[0];
        samplerDesc.BorderColor[1] = desc.BorderColor[1];
        samplerDesc.BorderColor[2] = desc.BorderColor[2];
        samplerDesc.BorderColor[3] = desc.BorderColor[3];
        samplerDesc.MinLOD = desc.MinLOD;
        samplerDesc.MaxLOD = desc.MaxLOD;

        ComPtr<ID3D11SamplerState> pSamplerState;
        ThrowIfFailed(m_pDevice->CreateSamplerState(&samplerDesc, pSamplerState.GetAddressOf()));

        return std::make_shared<SamplerStateDirectX11>(pSamplerState.Get());
    }

    void RHIDirectX11::DeleteSamplerState(std::shared_ptr<RHI::RHISamplerState>& samplerState)
    {
        samplerState.reset();
    }

    std::shared_ptr<RHIBuffer> RHIDirectX11::CreateBuffer(const BufferDesc& desc)
    {
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = static_cast<UINT>(desc.SizeInBytes);
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = desc.Stride;
        bufferDesc.Usage = ToD3D11Usage(desc.HeapType);
        bufferDesc.CPUAccessFlags = ToD3D11CPUAccessFlags(desc.HeapType);

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = desc.InitialData;

        ComPtr<ID3D11Buffer> pBuffer;
        if (desc.InitialData != nullptr)
        {
            ThrowIfFailed(m_pDevice->CreateBuffer(&bufferDesc, &initData, pBuffer.GetAddressOf()));
        }
#if RHI_ENABLE_RESOURCE_INFO
        else if(desc.InitialData == nullptr)
            ThrowIfFailed("Creating D3D11_HEAP_TYPE_DEFAULT requires providing heap data");
#endif
        else
        {
            ThrowIfFailed(m_pDevice->CreateBuffer(&bufferDesc, nullptr, pBuffer.GetAddressOf()));
        }

        auto buffer = std::make_shared<BufferDirectX11>(pBuffer.Get(), desc);
        buffer->SetDeviceContext(m_pDeviceContext.Get());

        return buffer;
    }

    void RHIDirectX11::DeleteBuffer(std::shared_ptr<RHI::RHIBuffer>& buffer)
    {
        buffer.reset();
    }
}
