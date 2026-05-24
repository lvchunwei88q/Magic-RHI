#pragma once

#include "Common/DIRECTX12RHI_API.h"
#include <RHIRootSignature.h>
#include <d3d12.h>
#include <d3dx12.h>

#include <wrl.h>
using Microsoft::WRL::ComPtr;

namespace RHI
{
    class DIRECTX12RHI_API RHIRootSignatureDirectX12 : public RHIRootSignature
    {
    public:
        RHIRootSignatureDirectX12();
        ~RHIRootSignatureDirectX12() override;

        bool Initialize(Device* device, const RootSignatureDesc& desc) override;
        void Shutdown() override;

        bool IsValid() const override;

        ID3D12RootSignature* GetRootSignature() const { return m_pRootSignature.Get(); }

    private:
        ComPtr<ID3D12RootSignature> m_pRootSignature;
    };
}
