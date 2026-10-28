/*
 * Because we used forward declarations and C++ smart pointers,
 * we need to include the definitions of the forward declarations first.
 */
#include <Common/Check.h>
#include <cstdint>
#include "CoreMinimal.h"
#include "RHICommandListD3D12.h"
#include "RHID3D12.h"

namespace RHI
{
    namespace
    {
        DXGI_FORMAT SwapChainBackBufferFormat_DX12 = DXGI_FORMAT_R8G8B8A8_UNORM; // Use sRGB format for correct color space
        DXGI_FORMAT SwapChainRTVFormat_DX12 = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // Use sRGB format for correct color space
        RHITextureFormat SwapChainRTVFormat_RHI = RHITextureFormat::R8G8B8A8_SRGB; // Use sRGB format for correct color space
    }

    SwapChainD3D12::SwapChainD3D12()
    {
    }

    SwapChainD3D12::~SwapChainD3D12()
    {
        Shutdown();
    }

    bool SwapChainD3D12::Initialize(Device* device, const SwapChainDesc& desc)
    {
        m_Initialization = InitialState::Initialize;
        // Get the window handle from the device
        HWND windowHandle = static_cast<HWND>(desc.WindowHandleRef);

        m_pRHI = SafeCast<DeviceD3D12>(device);
        if (!m_pRHI)
            return false;

        m_desc = desc;
        ComPtr<IDXGIAdapter1> pAdapter = m_pRHI->GetAdapter();

        ComPtr<IDXGIFactory4> factory;
        ThrowIfFailed(pAdapter->GetParent(IID_PPV_ARGS(&factory)));

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = RHI_MULTI_BUFFERING;
        swapChainDesc.Width = desc.Width;
        swapChainDesc.Height = desc.Height;
        swapChainDesc.Format = SwapChainBackBufferFormat_DX12;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;

        // Only a graphics queue can be bound to the swap chain
        GraphicsCommandQueueD3D12* GraphicsCommandQueue = SafeCast<GraphicsCommandQueueD3D12> (m_pRHI->GetFrameCommandQueue());
        ComPtr<IDXGISwapChain1> swapChain1;
        ThrowIfFailed(factory->CreateSwapChainForHwnd(
            GraphicsCommandQueue->GetCommandQueue(),
            windowHandle,
            &swapChainDesc,
            nullptr,
            nullptr,
            swapChain1.GetAddressOf()
        ));

#if RHI_SWAP_CHAIN_CLOSE_FULL_SCREEN
        // This RHI does not support fullscreen transitions.
        ThrowIfFailed(factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER));
#endif

        ComPtr<IDXGISwapChain3> swapChain3;
        if (SUCCEEDED(swapChain1.As(&swapChain3))) {
            m_pSwapChain3 = swapChain3;  // Supports modern interface extensions
            m_pSwapChain1 = swapChain1;  // Downgrade to basic interface if not supported
        } else {
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("SwapChain3 is not supported on this device.");
#endif
            m_pSwapChain1 = swapChain1;  // Downgrade to basic interface if not supported
        }

        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = RHI_MULTI_BUFFERING;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        ThrowIfFailed(m_pRHI->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_pRtvHeap.GetAddressOf())));
        // Create render target views
        CreateRenderTargetView();
        return true;
    }

    void SwapChainD3D12::CreateRenderTargetView()
    {
        ComPtr<ID3D12Device> dx12Device;
        ThrowIfFailed(m_pSwapChain1->GetDevice(IID_PPV_ARGS(&dx12Device)));
        
        UINT rtvDescriptorSize = dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRtvHeap->GetCPUDescriptorHandleForHeapStart());
        
        // define the render target view description
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = SwapChainRTVFormat_DX12; // Use sRGB format for correct color space
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;

        for (UINT n = 0; n < RHI_MULTI_BUFFERING; n++)
        {
            ComPtr<ID3D12Resource> pResource;
            ThrowIfFailed(m_pSwapChain1->GetBuffer(n, IID_PPV_ARGS(&pResource)));
            dx12Device->CreateRenderTargetView(pResource.Get(), &rtvDesc, rtvHandle);
            
            TextureDesc back_desc                               = { RHITextureFormat::R8G8B8A8_UNORM , m_desc.Width, m_desc.Height, 1, 1, 1, 0 };
            std::unique_ptr<RenderTargetViewD3D12> pDerived = std::make_unique<RenderTargetViewD3D12>(rtvHandle);
            std::unique_ptr<RHITexture> pBackBuffer             = std::make_unique<TextureD3D12>(pResource.Get(),back_desc);
            // Set descriptor handle offset
            rtvHandle.Offset(1, rtvDescriptorSize);
            
            // Store the render target view and back buffer
            m_pRenderTargetViews[n] = std::move(pDerived);
            m_pBackBuffers[n] = std::move(pBackBuffer);
        }
    }

    void SwapChainD3D12::Shutdown()
    {
        m_Initialization = InitialState::Shutdown;
        for (UINT n = 0; n < RHI_MULTI_BUFFERING; n++)
        {
            m_pRenderTargetViews[n].reset();
            m_pBackBuffers[n].reset();
        }

        m_pRtvHeap.Reset();
        m_pSwapChain3.Reset();
        m_pSwapChain1.Reset();
        Core::InfoCapture::Capture("[D3D12RHI] Swap chain shutdown complete");
    }

    bool SwapChainD3D12::IsValid() const {
        // check if swap chain is valid
        return (m_pSwapChain1 != nullptr || m_pSwapChain3 != nullptr) 
            && m_Initialization == InitialState::Initialize;
    }

    // void SwapChainD3D12::Present(uint32_t syncInterval, uint32_t presentFlags)
    // {
    //     m_pSwapChain1->Present(syncInterval, presentFlags);
    // }

    void SwapChainD3D12::Resize(uint32_t width, uint32_t height)
    {
        m_desc.Width = width;
        m_desc.Height = height;

        for (UINT n = 0; n < RHI_MULTI_BUFFERING; n++)
        {
            m_pRenderTargetViews[n].reset();
            m_pBackBuffers[n].reset();
        }

        ThrowIfFailed(m_pSwapChain1->ResizeBuffers(
            RHI_MULTI_BUFFERING,
            width,
            height,
            SwapChainBackBufferFormat_DX12,
            0
        ));

        // Create new render target views
        CreateRenderTargetView();
    }

    RHIRenderTargetView* SwapChainD3D12::GetRenderTargetView(uint32_t index) const
    {
        return m_pRenderTargetViews[index].get();
    }

    RHITextureFormat SwapChainD3D12::GetRenderTargetFormat() const
    {
        // Return the RHI texture format corresponding to the swap chain format
        return SwapChainRTVFormat_RHI;
    }

    RHITexture*  SwapChainD3D12::GetBackBuffer(uint32_t index) const
    {
        return m_pBackBuffers[index].get();
    }

    GraphicsQueueSwapChainInfo SwapChainD3D12::GetGraphicsQueueSwapChainInfo() const 
    {
        GraphicsQueueSwapChainInfo Info;
        Info.imageIndex = m_pSwapChain3->GetCurrentBackBufferIndex();
        Info.SwapChain = (uint64_t)m_pSwapChain1.Get();
        Info.VSync = m_desc.VSync;
        return Info;
    }
}
