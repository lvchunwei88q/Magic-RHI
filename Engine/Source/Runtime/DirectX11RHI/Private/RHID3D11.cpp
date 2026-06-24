/*
* 因为使用到了前向声明所以需要先引入声明定义
 */
#include <Common/RHIFeatureLevel.h>
#include <Common/Check.h>
#include "RHIRootSignatureD3D11.h"
#include "RHICommandListD3D11.h"
#include "RHID3D11.h"
#include "DirectXConfig.h"

namespace RHI
{
    namespace
    {
        FeatureLevel FromD3DFeatureLevel(D3D_FEATURE_LEVEL level)
        {
            switch (level)
            {
            case D3D_FEATURE_LEVEL_9_1:  return FeatureLevel::Level_9_1;
            case D3D_FEATURE_LEVEL_9_2:  return FeatureLevel::Level_9_2;
            case D3D_FEATURE_LEVEL_9_3:  return FeatureLevel::Level_9_3;
            case D3D_FEATURE_LEVEL_10_0: return FeatureLevel::Level_10_0;
            case D3D_FEATURE_LEVEL_10_1: return FeatureLevel::Level_10_1;
            case D3D_FEATURE_LEVEL_11_0: return FeatureLevel::Level_11_0;
            case D3D_FEATURE_LEVEL_11_1: return FeatureLevel::Level_11_1;
            case D3D_FEATURE_LEVEL_12_0: return FeatureLevel::Level_12_0;
            case D3D_FEATURE_LEVEL_12_1: return FeatureLevel::Level_12_1;
            case D3D_FEATURE_LEVEL_12_2: return FeatureLevel::Level_12_2;
            
            default:
                return FeatureLevel::Level_11_0;  // 默认值
            }
        }
    }
    
    RHID3D11::RHID3D11()
    {
    }

    RHID3D11::~RHID3D11()
    {
        Shutdown();
    }

    bool RHID3D11::Initialize()
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

        // 初始化命令队列
        m_CommandQueue = std::make_unique<CommandQueueD3D11>(RHICmdType::Graphics, m_pDeviceContext.Get(), m_pDevice.Get());

        // -------------------- Create descriptor heaps --------------------
        m_pStandardHeap = std::make_unique<DescriptorHeapD3D11>(RHIDescriptorHeapType::Standard, RHI_DESCRIPTOR_HEAP_SIZE_STANDARD);
        m_pSamplerHeap = std::make_unique<DescriptorHeapD3D11>(RHIDescriptorHeapType::Sampler, RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER);
        m_pRTVHeap = std::make_unique<DescriptorHeapD3D11>(RHIDescriptorHeapType::RenderTarget, RHI_DESCRIPTOR_HEAP_SIZE_RENDER_TARGET);
        m_pDSVHeap = std::make_unique<DescriptorHeapD3D11>(RHIDescriptorHeapType::DepthStencil, RHI_DESCRIPTOR_HEAP_SIZE_DEPTH_STENCIL);
        // -------------------- Create descriptor heaps End --------------------
        return true;
    }

    void RHID3D11::Shutdown()
    {
        if (m_pDeviceContext) {
            m_pDeviceContext->ClearState();
            m_pDeviceContext->Flush();  // 刷新所有待执行的命令
        }
        
#ifdef _DEBUG
        ID3D11Debug* debug = nullptr;
        m_pDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug);
        if (debug) {
            debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
            debug->Release();
        }
#endif
        m_pDeviceContext.Reset();
        m_pDevice.Reset();
    }

    FeatureLevel RHID3D11::GetFeatureLevel() const{
        return FromD3DFeatureLevel(m_FeatureLevel);
    }

    bool RHID3D11::IsValid() const
    {
        return m_pDevice != nullptr;
    }

    std::shared_ptr<RHICommandAllocator> RHID3D11::CreateCommandAllocator(RHICmdType type)
    {
        return std::make_shared<CommandAllocatorD3D11>(type, m_pDeviceContext.Get());
    }

    std::shared_ptr<RHICommandList> RHID3D11::CreateCommandList(std::shared_ptr<RHICommandAllocator>& allocator)
    {
        // Incoming allocator, Responsible for submitting data to the command list in DX11
        return std::make_shared<CommandListD3D11>(allocator.get());
    }

    /*
    * 获取图形命令队列 因为DX11 不支持多个命令队列，所以返回的是同一个队列
    */
    RHICommandQueue* RHID3D11::GetCommandQueue(RHICmdType Type) const
    {
        return m_CommandQueue.get();
    }

    std::shared_ptr<RHIRootSignature> RHID3D11::CreateRootSignature(const RootSignatureDesc& desc)
    {
        auto rootSignature = std::make_shared<RHIRootSignatureD3D11>();
        rootSignature->Initialize(this, desc);
        return rootSignature;
    }

    void RHID3D11::DeleteRootSignature(std::shared_ptr<RHIRootSignature>& rootSignature)
    {
        if (rootSignature)
        {
            rootSignature->Shutdown();
            rootSignature.reset();
        }
    }

    RHIDescriptorHeap* RHID3D11::GetDescriptorHeap(RHIDescriptorHeapType type) const
    {
        switch (type)
        {
        case RHIDescriptorHeapType::Standard:
            return m_pStandardHeap.get();
        case RHIDescriptorHeapType::Sampler:
            return m_pSamplerHeap.get();
        case RHIDescriptorHeapType::RenderTarget:
            return m_pRTVHeap.get();
        case RHIDescriptorHeapType::DepthStencil:
            return m_pDSVHeap.get();
        default:
            return nullptr;
        }
    }
}
