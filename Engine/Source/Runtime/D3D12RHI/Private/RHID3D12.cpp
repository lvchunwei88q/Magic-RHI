/*
* 因为使用到了前向声明所以需要先引入声明定义
*/

#include <Common/Check.h>
#include <Common/RHIFeatureLevel.h>
#include "RHIRootSignatureD3D12.h"
#include "RHICommandListD3D12.h"
#include "RHID3D12.h"
#include "DirectXConfig.h"

namespace RHI
{
    namespace
    {
        // Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
        // If no such adapter can be found, *ppAdapter will be set to nullptr.
        _Use_decl_annotations_
        void GetHardwareAdapter(
            IDXGIFactory1* pFactory,
            IDXGIAdapter1** ppAdapter,
            bool requestHighPerformanceAdapter = true) // High-performance graphics card
        {
            *ppAdapter = nullptr;

            ComPtr<IDXGIAdapter1> adapter;

            ComPtr<IDXGIFactory6> factory6;
            if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
            {
                for (
                    UINT adapterIndex = 0;
                    SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                        adapterIndex,
                        requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                        IID_PPV_ARGS(&adapter)));
                    ++adapterIndex)
                {
                    DXGI_ADAPTER_DESC1 desc;
                    adapter->GetDesc1(&desc);

                    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                    {
                        // Don't select the Basic Render Driver adapter.
                        // If you want a software adapter, pass in "/warp" on the command line.
                        continue;
                    }

                    // Check to see whether the adapter supports Direct3D 12, but don't create the
                    // actual device yet.
                    if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                    {
                        break;
                    }
                }
            }

            if(adapter.Get() == nullptr)
            {
                for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
                {
                    DXGI_ADAPTER_DESC1 desc;
                    adapter->GetDesc1(&desc);

                    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                    {
                        // Don't select the Basic Render Driver adapter.
                        // If you want a software adapter, pass in "/warp" on the command line.
                        continue;
                    }

                    // Check to see whether the adapter supports Direct3D 12, but don't create the
                    // actual device yet.
                    if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                    {
                        break;
                    }
                }
            }
            
            *ppAdapter = adapter.Detach();
        }

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

    DeviceD3D12::DeviceD3D12()
    {
    }

    DeviceD3D12::~DeviceD3D12()
    {
        Shutdown();
    }

    bool DeviceD3D12::Initialize()
    {
        m_Initialization = InitialState::Initialize;
        UINT dxgiFactoryFlags = 0;

#ifdef _DEBUG
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

        ComPtr<IDXGIFactory4> factory;
        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
        GetHardwareAdapter(factory.Get(), &m_pAdapter);
        
        if (m_pAdapter) {
            DXGI_ADAPTER_DESC1 desc;
            m_pAdapter->GetDesc1(&desc);
            m_AdapterName = desc.Description;  // copy
            
        } else {
            m_AdapterName = L"Unknown Adapter";
        }

        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_12_2,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };
        
        OutputDebugStringA("We use different levels to create devices, so if your physical device or system does not support the latest device creation, you may receive a device creation error. However, you don't need to worry because we will create them step by step.");
        for (auto level : featureLevels) {
            if (SUCCEEDED(D3D12CreateDevice(m_pAdapter.Get(), level, 
                                             IID_PPV_ARGS(&m_pDevice)))) {
                m_FeatureLevel = level;
                break;
            }
        }
        OutputDebugStringA("The device has been created. You can now find errors through the subsequent output debug information.");
        
        CreateQueues(); // create queues

        // -------------------- Create descriptor heaps --------------------
        uint32_t m_StandardDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        uint32_t m_SamplerDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        uint32_t m_RTVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        uint32_t m_DSVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        ComPtr<ID3D12DescriptorHeap> pStandardHeap;
        ComPtr<ID3D12DescriptorHeap> pSamplerHeap;
        ComPtr<ID3D12DescriptorHeap> pRTVHeap;
        ComPtr<ID3D12DescriptorHeap> pDSVHeap;

        D3D12_DESCRIPTOR_HEAP_DESC standardHeapDesc = {};
        standardHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        standardHeapDesc.NumDescriptors = RHI_DESCRIPTOR_HEAP_SIZE_STANDARD;
        standardHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&standardHeapDesc, IID_PPV_ARGS(&pStandardHeap)));

        D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
        samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        samplerHeapDesc.NumDescriptors = RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER;
        samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&pSamplerHeap)));

        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.NumDescriptors = RHI_DESCRIPTOR_HEAP_SIZE_RENDER_TARGET;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&pRTVHeap)));

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.NumDescriptors = RHI_DESCRIPTOR_HEAP_SIZE_DEPTH_STENCIL;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&pDSVHeap)));

        m_pStandardHeap = std::make_unique<DescriptorHeapD3D12>(pStandardHeap.Get(), RHIDescriptorHeapType::Standard, RHI_DESCRIPTOR_HEAP_SIZE_STANDARD, m_StandardDescriptorSize);
        m_pSamplerHeap = std::make_unique<DescriptorHeapD3D12>(pSamplerHeap.Get(), RHIDescriptorHeapType::Sampler, RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER, m_SamplerDescriptorSize);
        m_pRTVHeap = std::make_unique<DescriptorHeapD3D12>(pRTVHeap.Get(), RHIDescriptorHeapType::RenderTarget, RHI_DESCRIPTOR_HEAP_SIZE_RENDER_TARGET, m_RTVDescriptorSize);
        m_pDSVHeap = std::make_unique<DescriptorHeapD3D12>(pDSVHeap.Get(), RHIDescriptorHeapType::DepthStencil, RHI_DESCRIPTOR_HEAP_SIZE_DEPTH_STENCIL, m_DSVDescriptorSize);
        // -------------------- Create descriptor heaps End --------------------

#ifdef _DEBUG
        ComPtr<ID3D12InfoQueue> infoQueue;
        if (SUCCEEDED(m_pDevice.As(&infoQueue)))
        {
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        }
#endif
        return true;
    }

    void DeviceD3D12::Shutdown()
    {
        m_Initialization = InitialState::Shutdown;
        m_pDSVHeap.reset();
        m_pRTVHeap.reset();
        m_pSamplerHeap.reset();
        m_pStandardHeap.reset();

        m_GraphicsQueue.reset();
        m_ComputeQueue.reset();
        m_CopyQueue.reset();

        m_pDevice.Reset();
    }

    FeatureLevel DeviceD3D12::GetFeatureLevel() const{
        return FromD3DFeatureLevel(m_FeatureLevel);
    }

    void DeviceD3D12::CreateQueues()
    {
        // 创建命令队列 这是唯一的Com接口没有 1，2，3 等版本的接口
        ComPtr<ID3D12CommandQueue> pGraphicsQueue;
        ComPtr<ID3D12CommandQueue> pComputeQueue;
        ComPtr<ID3D12CommandQueue> pCopyQueue;

        // 图形队列（DIRECT）- 全能型，执行所有命令
        D3D12_COMMAND_QUEUE_DESC graphicsQueueDesc = {};
        graphicsQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        graphicsQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        graphicsQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        ThrowIfFailed(m_pDevice->CreateCommandQueue(&graphicsQueueDesc, IID_PPV_ARGS(&pGraphicsQueue)));

        // 计算队列（COMPUTE）- 只执行计算和拷贝命令
        D3D12_COMMAND_QUEUE_DESC computeQueueDesc = {};
        computeQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        computeQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        computeQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        ThrowIfFailed(m_pDevice->CreateCommandQueue(&computeQueueDesc, IID_PPV_ARGS(&pComputeQueue)));

        // 拷贝队列（COPY）- 只执行拷贝命令
        D3D12_COMMAND_QUEUE_DESC copyQueueDesc = {};
        copyQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        copyQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        copyQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        ThrowIfFailed(m_pDevice->CreateCommandQueue(&copyQueueDesc, IID_PPV_ARGS(&pCopyQueue)));

        m_GraphicsQueue = std::make_unique<GraphicsCommandQueueD3D12>(
            RHICmdType::Graphics, 
            pGraphicsQueue.Get(),
            m_pDevice.Get()
        );
        m_ComputeQueue = std::make_unique<ComputeCommandQueueD3D12>(
            RHICmdType::Compute, 
            pComputeQueue.Get(),
            m_pDevice.Get()
        );
        m_CopyQueue = std::make_unique<CopyCommandQueueD3D12>(
            RHICmdType::Copy, 
            pCopyQueue.Get(),
            m_pDevice.Get()
        );
    }

    bool DeviceD3D12::IsValid() const
    {
        return m_pDevice != nullptr && m_Initialization == InitialState::Initialize;
    }

    /*
    * Get the graphics command queues here is divided into graphics queues, compute queues, and copy queues
    */
    RHICommandQueue* DeviceD3D12::GetCommandQueue(RHICmdType Type) const
    {
        switch (Type)
        {
        case RHICmdType::Graphics:
            return m_GraphicsQueue.get();
        case RHICmdType::Compute:
            return m_ComputeQueue.get();
        case RHICmdType::Copy:
            return m_CopyQueue.get();
        default:
            return nullptr;
        }
    }

    RHIDescriptorHeap* DeviceD3D12::GetDescriptorHeap(RHIDescriptorHeapType type) const
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
