#pragma once

#include "Common/DIRECTX11RHI_API.h"
#include <RHI.h>
#include <d3d11.h>
#include <dxgi.h>

#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    class RHIDirectX11;

    class DIRECTX11RHI_API SwapChainDirectX11 : public SwapChain
    {
    public:
        SwapChainDirectX11();
        ~SwapChainDirectX11() override;

        bool Initialize(Device* device, const SwapChainDesc& desc) override;
        void Shutdown() override;

        bool IsValid() const override;
        void Present() override;
        void Resize(uint32_t width, uint32_t height) override;

        IDXGISwapChain* GetSwapChain() const { return m_pSwapChain.Get(); }

    private:
        RHIDirectX11* m_pRHI;
        
        ComPtr<IDXGISwapChain> m_pSwapChain;
        ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
        SwapChainDesc m_desc;
    };
}
