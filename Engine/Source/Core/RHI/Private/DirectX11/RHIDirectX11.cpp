/*
* 因为使用到了前向声明所以需要先引入声明定义
 */
#include "DirectX11/RHIResourceDirectX11.h"
#include "DirectX11/RHIDirectX11.h"
#include "DirectXHelper.h"

namespace RHI
{
    namespace
    {
    }
    
    RHIDirectX11::RHIDirectX11()
    {
    }

    RHIDirectX11::~RHIDirectX11()
    {
        Shutdown();
    }

    bool RHIDirectX11::Initialize()
    {
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        
        ThrowIfFailed(D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            creationFlags,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            m_pDevice.GetAddressOf(),
            &m_FeatureLevel,
            m_pDeviceContext.GetAddressOf()
        ));

#ifdef _DEBUG
        ComPtr<ID3D11InfoQueue> infoQueue;
        if (SUCCEEDED(m_pDevice.As(&infoQueue)))
        {
            infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        }
#endif

        ComPtr<IDXGIDevice> dxgiDevice;
        
        ThrowIfFailed(m_pDevice.As(&dxgiDevice));
        ComPtr<IDXGIAdapter> adapter;
        ThrowIfFailed(dxgiDevice->GetAdapter(adapter.GetAddressOf()));
            
        DXGI_ADAPTER_DESC desc = {};
        ThrowIfFailed(adapter->GetDesc(&desc));
        m_AdapterName = desc.Description;

        return true;
    }

    void RHIDirectX11::Shutdown()
    {
        m_pDeviceContext.Reset();
        m_pDevice.Reset();
    }

    bool RHIDirectX11::IsValid() const
    {
        return m_pDevice != nullptr;
    }

    std::shared_ptr<RHICommandList> RHIDirectX11::CreateCommandList(RHICmdListType type)
    {
        // DX 11 不支持命令列表     
        return std::make_shared<CommandListDirectX11>(type);
    }

    std::shared_ptr<RHICommandQueue> RHIDirectX11::GetCommandQueue(RHICmdListType Type) const
    {
        // DX 11 不支持命令队列
        return m_CommandQueue;
    }
}
