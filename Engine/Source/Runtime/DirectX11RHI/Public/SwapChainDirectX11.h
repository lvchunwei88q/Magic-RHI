#pragma once

#include "Common/DIRECTX11RHI_API.h"
#include <RHI.h>
#include <RHIResource.h>
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
        void Present(uint32_t syncInterval, uint32_t presentFlags) override;
        void Resize(uint32_t width, uint32_t height) override;
        uint32_t GetFrameIndex() const override { return 0; }

        uint32_t GetWidth() const override { return m_desc.Width; }
        uint32_t GetHeight() const override { return m_desc.Height; }
        // Get render target view
        RHIRenderTargetView* GetRenderTargetView(uint32_t index) const override;
        // Get back buffer
        RHITexture* GetBackBuffer(uint32_t index) const override;
        // Get swap chain
        IDXGISwapChain* GetSwapChain() const { return m_pSwapChain.Get(); }

    private:
        RHIDirectX11* m_pRHI;
        
        ComPtr<IDXGISwapChain> m_pSwapChain;
        std::unique_ptr<RHITexture> m_pBackBuffer;
        std::unique_ptr<RHIRenderTargetView> m_pRenderTargetView;
        SwapChainDesc m_desc;
    };
}
