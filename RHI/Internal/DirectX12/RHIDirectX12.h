#pragma once

#include <RHI.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>

#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    class RHIDirectX12 : public Device
    {
    public:
        RHIDirectX12();
        ~RHIDirectX12() override;

        bool Initialize() override;
        void Shutdown() override;

        bool IsValid() const override;
        RHIType GetType() const override { return RHIType::DirectX12; }
        const std::wstring& GetAdapterName() const override { return m_AdapterName; }
        
        const D3D_FEATURE_LEVEL& GetFeatureLevel() const { return m_FeatureLevel; }
        ID3D12Device* GetDevice() const { return m_pDevice.Get(); }

    private:
        ComPtr<ID3D12Device> m_pDevice;
        std::wstring m_AdapterName;
        D3D_FEATURE_LEVEL m_FeatureLevel;
    };
}
