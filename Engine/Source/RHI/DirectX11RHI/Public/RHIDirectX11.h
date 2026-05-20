#pragma once

#include "Common/DIRECTX11RHI_API.h"
#include "RHI.h"
#include <d3d11.h>
#include <dxgi.h>

#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

// CommandQueue Forward Declaration
class CommandQueueDirectX11;

namespace RHI
{
    class DIRECTX11RHI_API RHIDirectX11 : public Device
    {
    public:
        RHIDirectX11();
        ~RHIDirectX11() override;

        bool Initialize() override;
        void Shutdown() override;

        bool IsValid() const override;
        RHIType GetType() const override { return RHIType::DirectX11; }
        const std::wstring& GetAdapterName() const override { return m_AdapterName; }

        std::shared_ptr<RHISamplerState> CreateSamplerState(const SamplerStateDesc& desc) override;
        void DeleteSamplerState(std::shared_ptr<RHI::RHISamplerState>& samplerState) override;

        std::shared_ptr<RHIBuffer> CreateBuffer(const BufferDesc& desc) override;
        void DeleteBuffer(std::shared_ptr<RHI::RHIBuffer>& buffer) override;

        std::shared_ptr<RHICommandList> CreateCommandList(RHICmdListType type) override;
        std::shared_ptr<RHICommandQueue> GetCommandQueue(RHICmdListType Type) const override;

        FeatureLevel GetFeatureLevel() const override;
        ID3D11Device* GetDevice() const { return m_pDevice.Get(); }
        ID3D11DeviceContext* GetDeviceContext() const { return m_pDeviceContext.Get(); }

    private:
        ComPtr<ID3D11Device> m_pDevice;
        ComPtr<ID3D11DeviceContext> m_pDeviceContext;
        std::wstring m_AdapterName;
        D3D_FEATURE_LEVEL m_FeatureLevel;
        
        std::shared_ptr<CommandQueueDirectX11> m_CommandQueue;
    };
}
