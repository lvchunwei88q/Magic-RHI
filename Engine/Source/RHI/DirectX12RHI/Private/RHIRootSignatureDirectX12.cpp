
#include "RHIDirectX12.h"
#include "RHIRootSignatureDirectX12.h"
#include "DirectXHelper.h"

namespace RHI
{

    namespace
    {
        D3D12_SHADER_VISIBILITY Translate(ShaderVisibility vis)
        {
            // All → ALL
            if (vis == ShaderVisibility::All || vis == ShaderVisibility::AllGraphics)
                return D3D12_SHADER_VISIBILITY_ALL;

            // 单个 bit → 映射
            if (vis == ShaderVisibility::VertexBit)   return D3D12_SHADER_VISIBILITY_VERTEX;
            if (vis == ShaderVisibility::PixelBit)    return D3D12_SHADER_VISIBILITY_PIXEL;
            if (vis == ShaderVisibility::HullBit)     return D3D12_SHADER_VISIBILITY_HULL;
            if (vis == ShaderVisibility::DomainBit)   return D3D12_SHADER_VISIBILITY_DOMAIN;
            if (vis == ShaderVisibility::GeometryBit) return D3D12_SHADER_VISIBILITY_GEOMETRY;
            if (vis == ShaderVisibility::AmpBit)      return D3D12_SHADER_VISIBILITY_AMPLIFICATION;
            if (vis == ShaderVisibility::MeshBit)     return D3D12_SHADER_VISIBILITY_MESH;

            // 组合且不是全部 → 回退 ALL
            return D3D12_SHADER_VISIBILITY_ALL;
        }

        D3D12_DESCRIPTOR_RANGE_TYPE TranslateRangeType(DescriptorRangeType type)
        {
            switch (type)
            {
            case DescriptorRangeType::SRV:     return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            case DescriptorRangeType::UAV:     return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            case DescriptorRangeType::CBV:     return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            case DescriptorRangeType::Sampler: return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            // default → SRV
            default:                           return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            }
        }

        // 将 RHI 的 DescriptorRangeDesc 数组翻译为 D3D12_DESCRIPTOR_RANGE 数组
        D3D12_DESCRIPTOR_RANGE* TranslateRanges(const DescriptorRangeDesc* src, uint32_t count)
        {
            auto* ranges = new D3D12_DESCRIPTOR_RANGE[count];
            for (uint32_t i = 0; i < count; ++i)
            {
                ranges[i].RangeType          = TranslateRangeType(src[i].RangeType);            // Type
                ranges[i].NumDescriptors     = src[i].NumDescriptors;                                 // Num    
                ranges[i].BaseShaderRegister = src[i].ShaderRegister;                          
                ranges[i].RegisterSpace      = src[i].RegisterSpace;
                ranges[i].OffsetInDescriptorsFromTableStart = src[i].OffsetInDescriptorsFromTableStart;     // Offset -> Append
            }
            return ranges;
        }

    }

    RHIRootSignatureDirectX12::RHIRootSignatureDirectX12()
    {
    }

    RHIRootSignatureDirectX12::~RHIRootSignatureDirectX12()
    {
        Shutdown();
    }

    bool RHIRootSignatureDirectX12::Initialize(Device* device, const RootSignatureDesc& desc)
    {
        RHIDirectX12* dx12Device = static_cast<RHIDirectX12*>(device);
        if (!dx12Device)
        {
            return false;
        }

        std::vector<D3D12_ROOT_PARAMETER> rootParameters;
        rootParameters.reserve(desc.Parameters.size());

        for (const auto& paramDesc : desc.Parameters)
        {
            D3D12_ROOT_PARAMETER rootParam = {};
            rootParam.ShaderVisibility = Translate(paramDesc.Visibility);

            switch (paramDesc.Type)
            {
            case RootParameterType::CBV:
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                rootParam.Descriptor.ShaderRegister = paramDesc.Descriptor.ShaderRegister;
                rootParam.Descriptor.RegisterSpace  = paramDesc.Descriptor.RegisterSpace;
                break;

            case RootParameterType::SRV:
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
                rootParam.Descriptor.ShaderRegister = paramDesc.Descriptor.ShaderRegister;
                rootParam.Descriptor.RegisterSpace  = paramDesc.Descriptor.RegisterSpace;
                break;

            case RootParameterType::UAV:
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
                rootParam.Descriptor.ShaderRegister = paramDesc.Descriptor.ShaderRegister;
                rootParam.Descriptor.RegisterSpace  = paramDesc.Descriptor.RegisterSpace;
                break;

            case RootParameterType::Constants:
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                rootParam.Constants.ShaderRegister = paramDesc.Constants.ShaderRegister;
                rootParam.Constants.RegisterSpace  = paramDesc.Constants.RegisterSpace;
                rootParam.Constants.Num32BitValues = paramDesc.Constants.Num32BitValues;
                break;

            // 一个根签名可以有多个槽位，其中DescriptorTable有可以有多个资源描述符数组，每个资源描述符数组又可以指向多个资源
            case RootParameterType::DescriptorTable:
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                rootParam.DescriptorTable.NumDescriptorRanges = paramDesc.DescriptorTable.NumDescriptorRanges;
                rootParam.DescriptorTable.pDescriptorRanges   = TranslateRanges(paramDesc.DescriptorTable.pDescriptorRanges, 
                                                                                paramDesc.DescriptorTable.NumDescriptorRanges);
                break;
            }

            rootParameters.push_back(rootParam);
        }

        // 创建根签名描述符 可以有多个 槽位描述符
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(
            static_cast<UINT>(rootParameters.size()),
            rootParameters.data(),
            0,
            nullptr,
            static_cast<D3D12_ROOT_SIGNATURE_FLAGS>(desc.Flags)
        );

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;

        ThrowIfFailed(D3D12SerializeRootSignature(
            &rootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION_1_0,
            &signature,
            &error
        ));

        ThrowIfFailed(dx12Device->GetDevice()->CreateRootSignature(
            0,
            signature->GetBufferPointer(),
            signature->GetBufferSize(),
            IID_PPV_ARGS(m_pRootSignature.GetAddressOf())
        ));

        return true;
    }

    void RHIRootSignatureDirectX12::Shutdown()
    {
        m_pRootSignature.Reset();
    }

    bool RHIRootSignatureDirectX12::IsValid() const
    {
        return m_pRootSignature != nullptr;
    }
}
