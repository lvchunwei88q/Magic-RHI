/*
 * 因为使用到了前向声明所以需要先引入声明定义
 */
#include "RHIResourceDirectX12.h"
#include "RHIDirectX12.h"
#include "SwapChainDirectX12.h"
#include "DirectXHelper.h"
#include "DirectXConfig.h"

namespace RHI
{
    SwapChainDirectX12::SwapChainDirectX12()
        : m_frameIndex(0)
    {
    }

    SwapChainDirectX12::~SwapChainDirectX12()
    {
        Shutdown();
    }

    bool SwapChainDirectX12::Initialize(Device* device, const SwapChainDesc& desc)
    {
        m_pRHI = static_cast<RHIDirectX12*>(device);
        if (!m_pRHI)
        {
            return false;
        }

        m_desc = desc;
        
        ComPtr<IDXGIAdapter1> pAdapter = m_pRHI->GetAdapter();

        ComPtr<IDXGIFactory4> factory;
        ThrowIfFailed(pAdapter->GetParent(IID_PPV_ARGS(&factory)));

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = RHI_MULTI_BUFFERING;
        swapChainDesc.Width = desc.Width;
        swapChainDesc.Height = desc.Height;
        swapChainDesc.Format = DXGI_RTV_FORMAT;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;

        // 只有图形队列可以绑定交换链
        ComPtr<IDXGISwapChain1> swapChain1;
        ThrowIfFailed(factory->CreateSwapChainForHwnd(
            static_cast<GraphicsCommandQueueDirectX12*>(m_pRHI->GetCommandQueue(RHICmdListType::Graphics).get())->GetCommandQueue(),
            static_cast<HWND>(desc.WindowHandle),
            &swapChainDesc,
            nullptr,
            nullptr,
            swapChain1.GetAddressOf()
        ));

#if RHI_SWAP_CHAIN_CLOSE_FULL_SCREEN
        // This RHI does not support fullscreen transitions.
        ThrowIfFailed(factory->MakeWindowAssociation(static_cast<HWND>(desc.WindowHandle), DXGI_MWA_NO_ALT_ENTER));
#endif

        ThrowIfFailed(swapChain1.As(&m_pSwapChain));
        m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = RHI_MULTI_BUFFERING;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        ThrowIfFailed(m_pRHI->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_pRtvHeap.GetAddressOf())));

        CreateRTVs();

        return true;
    }

    void SwapChainDirectX12::CreateRTVs()
    {
        ComPtr<ID3D12Device> dx12Device;
        ThrowIfFailed(m_pSwapChain->GetDevice(IID_PPV_ARGS(&dx12Device)));
        
        UINT rtvDescriptorSize = dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRtvHeap->GetCPUDescriptorHandleForHeapStart());
        
        for (UINT n = 0; n < RHI_MULTI_BUFFERING; n++)
        {
            ThrowIfFailed(m_pSwapChain->GetBuffer(n, IID_PPV_ARGS(m_pRenderTargets[n].GetAddressOf())));
            dx12Device->CreateRenderTargetView(m_pRenderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, rtvDescriptorSize);
        }
    }

    void SwapChainDirectX12::Shutdown()
    {
        for (UINT n = 0; n < RHI_MULTI_BUFFERING; n++)
        {
            m_pRenderTargets[n].Reset();
        }
        m_pRtvHeap.Reset();
        m_pSwapChain.Reset();
    }

    bool SwapChainDirectX12::IsValid() const
    {
        return m_pSwapChain != nullptr;
    }

    void SwapChainDirectX12::Present()
    {
        m_pSwapChain->Present(m_desc.VSync ? 1 : 0, 0);
        m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
    }

    void SwapChainDirectX12::Resize(uint32_t width, uint32_t height)
    {
        m_desc.Width = width;
        m_desc.Height = height;

        for (UINT n = 0; n < RHI_MULTI_BUFFERING; n++)
        {
            m_pRenderTargets[n].Reset();
        }

        ThrowIfFailed(m_pSwapChain->ResizeBuffers(
            RHI_MULTI_BUFFERING,
            width,
            height,
            DXGI_RTV_FORMAT,
            0
        ));

        m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
        CreateRTVs();
    }
}
