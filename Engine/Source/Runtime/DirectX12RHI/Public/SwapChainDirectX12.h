#pragma once

#include "Common/DIRECTX12RHI_API.h"
#include <RHI.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>

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

        IDXGISwapChain3* GetSwapChain3() const { return m_pSwapChain3.Get(); }
        IDXGISwapChain1* GetSwapChain() const { return m_pSwapChain1.Get(); }

    private:
        RHIDirectX12* m_pRHI;
        
        void CreateRTVs();

        ComPtr<IDXGISwapChain3> m_pSwapChain3;
        ComPtr<IDXGISwapChain1> m_pSwapChain1;
        ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
        ComPtr<ID3D12Resource> m_pRenderTargets[RHI_MULTI_BUFFERING];

        SwapChainDesc m_desc;
    };
}
