/*
* 因为使用到了前向声明所以需要先引入声明定义
 */
#include "DirectX12/RHIResourceDirectX12.h"
#include "DirectX12/RHIDirectX12.h"
#include "DirectXHelper.h"

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
    }
    RHIDirectX12::RHIDirectX12()
    {
    }

    RHIDirectX12::~RHIDirectX12()
    {
        Shutdown();
    }

    bool RHIDirectX12::Initialize()
    {
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
        
        for (auto level : featureLevels) {
            if (SUCCEEDED(D3D12CreateDevice(m_pAdapter.Get(), level, 
                                             IID_PPV_ARGS(&m_pDevice)))) {
                m_FeatureLevel = level;
                break;
            }
        }
        
        CreateQueues(); // 创建队列

        m_StandardDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        m_SamplerDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        m_RTVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_DSVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        // -------------------- Create descriptor heaps --------------------
        D3D12_DESCRIPTOR_HEAP_DESC standardHeapDesc = {};
        standardHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        standardHeapDesc.NumDescriptors = RHI_DESCRIPTOR_HEAP_SIZE_STANDARD;
        standardHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&standardHeapDesc, IID_PPV_ARGS(&m_pStandardHeap)));

        D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
        samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        samplerHeapDesc.NumDescriptors = RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER;
        samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_pSamplerHeap)));

        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.NumDescriptors = RHI_DESCRIPTOR_HEAP_SIZE_RENDER_TARGET;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRTVHeap)));

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.NumDescriptors = RHI_DESCRIPTOR_HEAP_SIZE_DEPTH_STENCIL;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDSVHeap)));
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

    void RHIDirectX12::Shutdown()
    {
        m_pDSVHeap.Reset();
        m_pRTVHeap.Reset();
        m_pSamplerHeap.Reset();
        m_pStandardHeap.Reset();
        m_GraphicsQueue.reset();
        m_ComputeQueue.reset();
        m_CopyQueue.reset();
        m_pDevice.Reset();
    }

    void RHIDirectX12::CreateQueues()
    {
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

        m_GraphicsQueue = std::make_shared<GraphicsCommandQueueDirectX12>(
            RHICmdListType::Graphics, 
            pGraphicsQueue.Get(),
            m_pDevice.Get()
        );
        m_ComputeQueue = std::make_shared<ComputeCommandQueueDirectX12>(
            RHICmdListType::Compute, 
            pComputeQueue.Get(),
            m_pDevice.Get()
        );
        m_CopyQueue = std::make_shared<CopyCommandQueueDirectX12>(
            RHICmdListType::Copy, 
            pCopyQueue.Get(),
            m_pDevice.Get()
        );
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RHIDirectX12::GetStandardCPUHandle(uint32_t index) const
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pStandardHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += index * m_StandardDescriptorSize;
        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE RHIDirectX12::GetStandardGPUHandle(uint32_t index) const
    {
        D3D12_GPU_DESCRIPTOR_HANDLE handle = m_pStandardHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += index * m_StandardDescriptorSize;
        return handle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RHIDirectX12::GetSamplerCPUHandle(uint32_t index) const
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pSamplerHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += index * m_SamplerDescriptorSize;
        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE RHIDirectX12::GetSamplerGPUHandle(uint32_t index) const
    {
        D3D12_GPU_DESCRIPTOR_HANDLE handle = m_pSamplerHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += index * m_SamplerDescriptorSize;
        return handle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RHIDirectX12::GetRTVCPUHandle(uint32_t index) const
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pRTVHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += index * m_RTVDescriptorSize;
        return handle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RHIDirectX12::GetDSVCPUHandle(uint32_t index) const
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pDSVHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += index * m_DSVDescriptorSize;
        return handle;
    }

    bool RHIDirectX12::IsValid() const
    {
        return m_pDevice != nullptr;
    }
    
    std::shared_ptr<RHICommandList> RHIDirectX12::CreateCommandList(RHICmdListType type)
    {
        D3D12_COMMAND_LIST_TYPE d3dType;
        switch (type)
        {
        case RHICmdListType::Graphics:
            d3dType = D3D12_COMMAND_LIST_TYPE_DIRECT;
            break;
        case RHICmdListType::Compute:
            d3dType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            break;
        case RHICmdListType::Copy:
            d3dType = D3D12_COMMAND_LIST_TYPE_COPY;
            break;
        default:
            return nullptr;
        }
    
        // Create command allocator
        ComPtr<ID3D12CommandAllocator> pAllocator;
        ThrowIfFailed(m_pDevice->CreateCommandAllocator(d3dType, IID_PPV_ARGS(&pAllocator)));
    
        // Create command list
        ComPtr<ID3D12GraphicsCommandList> pCmdList;
        ThrowIfFailed(m_pDevice->CreateCommandList(0, d3dType, pAllocator.Get(), nullptr, IID_PPV_ARGS(&pCmdList)));
    
        // Initial state is closed
        ThrowIfFailed(pCmdList->Close());
    
        // Create RHI wrapper object
        return std::make_shared<CommandListDirectX12>(type, pCmdList.Get(), pAllocator.Get());
    }
    
    std::shared_ptr<RHICommandQueue> RHIDirectX12::GetCommandQueue(RHICmdListType Type) const
    {
        switch (Type)
        {
        case RHICmdListType::Graphics:
            return m_GraphicsQueue;
        case RHICmdListType::Compute:
            return m_ComputeQueue;
        case RHICmdListType::Copy:
            return m_CopyQueue;
        default:
            return nullptr;
        }
    }
}
