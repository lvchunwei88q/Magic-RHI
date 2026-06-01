/*
* 因为使用到了前向声明所以需要先引入声明定义
 */
#include "SwapChainDirectX11.h"
#include "RHIDirectX11.h"
#include "DirectXHelper.h"
#include "DirectXConfig.h"

namespace RHI
{
    SwapChainDirectX11::SwapChainDirectX11()
    {
    }

    SwapChainDirectX11::~SwapChainDirectX11()
    {
        Shutdown();
    }

    bool SwapChainDirectX11::Initialize(Device* device, const SwapChainDesc& desc)
    {
        m_pRHI = static_cast<RHIDirectX11*>(device);
        if (!m_pRHI)
        {
            return false;
        }

        m_desc = desc;

        ComPtr<IDXGIDevice> dxgiDevice;
        ThrowIfFailed(m_pRHI->GetDevice()->QueryInterface(IID_PPV_ARGS(&dxgiDevice)));

        ComPtr<IDXGIAdapter> adapter;
        ThrowIfFailed(dxgiDevice->GetAdapter(&adapter));

        ComPtr<IDXGIFactory> factory;
        ThrowIfFailed(adapter->GetParent(IID_PPV_ARGS(&factory)));

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferCount = RHI_MULTI_BUFFERING;
        swapChainDesc.BufferDesc.Width = desc.Width;
        swapChainDesc.BufferDesc.Height = desc.Height;
        swapChainDesc.BufferDesc.Format = DXGI_RTV_FORMAT;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = static_cast<HWND>(desc.WindowHandle);
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Flags = 0;

        ThrowIfFailed(factory->CreateSwapChain(
            m_pRHI->GetDevice(),
            &swapChainDesc,
            m_pSwapChain.GetAddressOf()
        ));

#if RHI_SWAP_CHAIN_CLOSE_FULL_SCREEN
        // This RHI does not support fullscreen transitions.
        ThrowIfFailed(factory->MakeWindowAssociation(static_cast<HWND>(desc.WindowHandle), DXGI_MWA_NO_ALT_ENTER));
#endif

        ComPtr<ID3D11Texture2D> backBuffer;
        ThrowIfFailed(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));

        ThrowIfFailed(m_pRHI->GetDevice()->CreateRenderTargetView(
            backBuffer.Get(),
            nullptr,
            m_pRenderTargetView.GetAddressOf()
        ));

        m_pRHI->GetDeviceContext()->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);

        D3D11_VIEWPORT viewport = {};
        viewport.Width = static_cast<float>(desc.Width);
        viewport.Height = static_cast<float>(desc.Height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        m_pRHI->GetDeviceContext()->RSSetViewports(1, &viewport);

        return true;
    }

    void SwapChainDirectX11::Shutdown()
    {
        m_pRenderTargetView.Reset();
        m_pSwapChain.Reset();
    }

    bool SwapChainDirectX11::IsValid() const
    {
        return m_pSwapChain != nullptr;
    }

    void SwapChainDirectX11::Present()
    {
        m_pSwapChain->Present(m_desc.VSync ? 1 : 0, 0);
    }

    void SwapChainDirectX11::Resize(uint32_t width, uint32_t height)
    {
        m_desc.Width = width;
        m_desc.Height = height;

        m_pRenderTargetView.Reset();

        ThrowIfFailed(m_pSwapChain->ResizeBuffers(
            0,          // 0: Remain unchanged
            width,
            height,
            DXGI_RTV_FORMAT,
            0
        ));

        ComPtr<ID3D11Texture2D> backBuffer;
        ThrowIfFailed(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));

        ComPtr<ID3D11Device> dx11Device;
        ThrowIfFailed(m_pSwapChain->GetDevice(IID_PPV_ARGS(&dx11Device)));

        ThrowIfFailed(dx11Device.Get()->CreateRenderTargetView(
            backBuffer.Get(),
            nullptr,
            m_pRenderTargetView.GetAddressOf()
        ));

        ID3D11DeviceContext* pContext = m_pRHI->GetDeviceContext();
        pContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);

        D3D11_VIEWPORT viewport = {};
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        pContext->RSSetViewports(1, &viewport);
    }
}
