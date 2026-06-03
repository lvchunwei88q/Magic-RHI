/*
 * 因为使用到了前向声明所以需要先引入声明定义
 */
#include "RHICommandListDirectX12.h"
#include "RHIDirectX12.h"
#include "SwapChainDirectX12.h"
#include "DirectXHelper.h"
#include "DirectXConfig.h"

namespace RHI
{
    SwapChainDirectX12::SwapChainDirectX12()
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
            SafeCast<CommandQueueDirectX12>(m_pRHI->GetCommandQueue(RHICmdType::Graphics).get())->GetCommandQueue(),
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

        ComPtr<IDXGISwapChain3> swapChain3;
        if (SUCCEEDED(swapChain1.As(&swapChain3))) {
            m_pSwapChain3 = swapChain3;  // 支持现代接口
            m_pSwapChain1 = swapChain1;  // 降级到基础接口
        } else {
#ifdef RHI_ENABLE_RESOURCE_DEBUG_INFO
            ThrowErrorMessage("SwapChain3 is not supported on this device.");
#endif
            m_pSwapChain1 = swapChain1;  // 降级到基础接口
        }

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
        ThrowIfFailed(m_pSwapChain1->GetDevice(IID_PPV_ARGS(&dx12Device)));
        
        UINT rtvDescriptorSize = dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRtvHeap->GetCPUDescriptorHandleForHeapStart());
        
        for (UINT n = 0; n < RHI_MULTI_BUFFERING; n++)
        {
            ThrowIfFailed(m_pSwapChain1->GetBuffer(n, IID_PPV_ARGS(m_pRenderTargets[n].GetAddressOf())));
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
        m_pSwapChain1.Reset();
    }

    bool SwapChainDirectX12::IsValid() const
    {
        return m_pSwapChain1 != nullptr || m_pSwapChain3 != nullptr;
    }

    void SwapChainDirectX12::Present()
    {
        m_pSwapChain1->Present(m_desc.VSync ? 1 : 0, 0);
    }

    void SwapChainDirectX12::Resize(uint32_t width, uint32_t height)
    {
        m_desc.Width = width;
        m_desc.Height = height;

        for (UINT n = 0; n < RHI_MULTI_BUFFERING; n++)
        {
            m_pRenderTargets[n].Reset();
        }

        ThrowIfFailed(m_pSwapChain1->ResizeBuffers(
            RHI_MULTI_BUFFERING,
            width,
            height,
            DXGI_RTV_FORMAT,
            0
        ));

        CreateRTVs();
    }
}
