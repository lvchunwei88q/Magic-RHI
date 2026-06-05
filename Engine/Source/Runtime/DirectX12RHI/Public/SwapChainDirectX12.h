#pragma once

#include "Common/DIRECTX12RHI_API.h"
#include <RHI.h>
#include <RHIResource.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>

#include <memory>
#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    class RHIDirectX12;
    
    class DIRECTX12RHI_API SwapChainDirectX12 : public SwapChain
    {
    public:
        SwapChainDirectX12();
        ~SwapChainDirectX12() override;

        bool Initialize(Device* device, const SwapChainDesc& desc) override;
        void Shutdown() override;

        bool IsValid() const override;
        void Present(uint32_t syncInterval, uint32_t presentFlags) override;
        void Resize(uint32_t width, uint32_t height) override;
        uint32_t GetFrameIndex() const override { return m_pSwapChain3->GetCurrentBackBufferIndex(); }// 获取当前帧索引

        uint32_t GetWidth() const override { return m_desc.Width; }
        uint32_t GetHeight() const override { return m_desc.Height; }
        // Get render target view
        RHIRenderTargetView* GetRenderTargetView(uint32_t index) const override;
        // Get back buffer
        RHITexture* GetBackBuffer() const override;

        IDXGISwapChain3* GetSwapChain3() const { return m_pSwapChain3.Get(); }
        IDXGISwapChain1* GetSwapChain() const { return m_pSwapChain1.Get(); }

    private:
        RHIDirectX12* m_pRHI;
        
        void CreateRTVs();

        ComPtr<IDXGISwapChain3> m_pSwapChain3;
        ComPtr<IDXGISwapChain1> m_pSwapChain1;
        ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
        std::unique_ptr<RHITexture> m_pBackBuffers[RHI_MULTI_BUFFERING];
        std::unique_ptr<RHIRenderTargetView> m_pRenderTargetViews[RHI_MULTI_BUFFERING];

        SwapChainDesc m_desc;
    };
}
