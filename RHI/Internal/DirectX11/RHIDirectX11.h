#pragma once

#include <RHI.h>
#include <d3d11.h>
#include <dxgi.h>

#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    class RHIDirectX11 : public Device
    {
    public:
        RHIDirectX11();
        ~RHIDirectX11() override;

        bool Initialize() override;
        void Shutdown() override;

        bool IsValid() const override;
        RHIType GetType() const override { return RHIType::DirectX11; }
        const std::wstring& GetAdapterName() const override { return m_AdapterName; }

        const D3D_FEATURE_LEVEL& GetFeatureLevel() const { return m_FeatureLevel; }
        ID3D11Device* GetDevice() const { return m_pDevice.Get(); }
        ID3D11DeviceContext* GetDeviceContext() const { return m_pDeviceContext.Get(); }

    private:
        ComPtr<ID3D11Device> m_pDevice;
        ComPtr<ID3D11DeviceContext> m_pDeviceContext;
        std::wstring m_AdapterName;
        D3D_FEATURE_LEVEL m_FeatureLevel;
    };
}
