
#include "RHIDirectX12.h"
#include "RHIRootSignatureDirectX12.h"
#include "DirectXHelper.h"

namespace RHI
{
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
            
            switch (paramDesc.Type)
            {
            case RootParameterType::CBV:
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                rootParam.Descriptor.ShaderRegister = paramDesc.ShaderRegister;
                rootParam.Descriptor.RegisterSpace = paramDesc.RegisterSpace;
                rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                break;

            case RootParameterType::SRV:
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
                rootParam.Descriptor.ShaderRegister = paramDesc.ShaderRegister;
                rootParam.Descriptor.RegisterSpace = paramDesc.RegisterSpace;
                rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                break;

            case RootParameterType::UAV:
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
                rootParam.Descriptor.ShaderRegister = paramDesc.ShaderRegister;
                rootParam.Descriptor.RegisterSpace = paramDesc.RegisterSpace;
                rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                break;

            case RootParameterType::Constants:
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                rootParam.Constants.Num32BitValues = 1;
                rootParam.Constants.ShaderRegister = paramDesc.ShaderRegister;
                rootParam.Constants.RegisterSpace = paramDesc.RegisterSpace;
                rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                break;
            case RootParameterType::DescriptorTable:
            default:
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

                // TODO: 处理描述符表参数
                break;
            }

            rootParameters.push_back(rootParam);
        }

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
