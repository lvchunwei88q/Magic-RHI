#pragma once

#include <RHIRootSignature.h>
#include <d3d12.h>
#include <d3dx12.h>

#include <wrl.h>
using Microsoft::WRL::ComPtr;

namespace RHI
{
    class RHIRootSignatureD3D12 : public RHIRootSignature
    {
    public:
        RHIRootSignatureD3D12();
        ~RHIRootSignatureD3D12() override;

        bool Initialize(Device* device, const RootSignatureDesc& desc) override;
        void Shutdown() override;

        bool IsValid() const override;

        ID3D12RootSignature* GetRootSignature() const { return m_pRootSignature.Get(); }

    private:
        ComPtr<ID3D12RootSignature> m_pRootSignature;
    };
}
