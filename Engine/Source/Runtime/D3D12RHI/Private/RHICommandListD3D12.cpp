#include "RHICommandListD3D12.h"
#include "RHID3D12.h"

namespace RHI
{
    namespace
    {
        [[nodiscard]] static ComPtr<ID3D12GraphicsCommandList> CreateLevelCommandList(
            ID3D12Device* pDevice,
            D3D12_COMMAND_LIST_TYPE d3dType,
            ID3D12CommandAllocator* pAllocator,
            FeatureLevel featureLevel)
        {
            // 0=NOT, 1=BASE, 2=接口1, 3=接口2, 4=接口4
            static int s_MaxSupportedVersion = 0;

            auto ProbeVersion = [&](auto&& pProbeList, int version) -> bool
            {
                HRESULT hr = pDevice->CreateCommandList(
                    0, d3dType, pAllocator, nullptr,
                    IID_PPV_ARGS(&pProbeList));
                if (SUCCEEDED(hr))
                {
                    s_MaxSupportedVersion = version;
                    pProbeList.Reset();
                    return true;
                }
                return false;
            };

            if (s_MaxSupportedVersion == 0)
            {
                if (featureLevel >= FeatureLevel::D3D12_12_2)
                {
                    ComPtr<ID3D12GraphicsCommandList4> pCmdList4;
                    ProbeVersion(pCmdList4, 4);
                }

                if (s_MaxSupportedVersion == 0 && featureLevel >= FeatureLevel::D3D12_12_0)
                {
                    ComPtr<ID3D12GraphicsCommandList2> pCmdList2;
                    ProbeVersion(pCmdList2, 3);
                }

                if (s_MaxSupportedVersion == 0 && featureLevel >= FeatureLevel::D3D12_11_0)
                {
                    ComPtr<ID3D12GraphicsCommandList1> pCmdList1;
                    ProbeVersion(pCmdList1, 2);
                }
                if (s_MaxSupportedVersion == 0)
                {
                    s_MaxSupportedVersion = 1;
                }
            }

            switch (s_MaxSupportedVersion)
            {
                case 4:
                {
                    ComPtr<ID3D12GraphicsCommandList4> pCmdList4;
                    ThrowIfFailed(pDevice->CreateCommandList(
                        0, d3dType, pAllocator, nullptr,
                        IID_PPV_ARGS(&pCmdList4)));
                    return pCmdList4;
                }
                case 3:
                {
                    ComPtr<ID3D12GraphicsCommandList2> pCmdList2;
                    ThrowIfFailed(pDevice->CreateCommandList(
                        0, d3dType, pAllocator, nullptr,
                        IID_PPV_ARGS(&pCmdList2)));
                    return pCmdList2;
                }
                case 2:
                {
                    ComPtr<ID3D12GraphicsCommandList1> pCmdList1;
                    ThrowIfFailed(pDevice->CreateCommandList(
                        0, d3dType, pAllocator, nullptr,
                        IID_PPV_ARGS(&pCmdList1)));
                    return pCmdList1;
                }
                default:
                {
                    ComPtr<ID3D12GraphicsCommandList> pCmdList;
                    ThrowIfFailed(pDevice->CreateCommandList(
                        0, d3dType, pAllocator, nullptr,
                        IID_PPV_ARGS(&pCmdList)));
                    return pCmdList;
                }
            }
        }

        D3D12_COMMAND_LIST_TYPE ConvertRHICmdTypeToD3D12(RHICmdType type)
        {
            switch (type)
            {
            case RHICmdType::Graphics:
                return D3D12_COMMAND_LIST_TYPE_DIRECT;
            case RHICmdType::Compute:
                return D3D12_COMMAND_LIST_TYPE_COMPUTE;
            case RHICmdType::Copy:
                return D3D12_COMMAND_LIST_TYPE_COPY;
            default:
#ifdef _DEBUG
                // 调试模式下，抛出异常
                ThrowErrorMessage("Invalid RHICmdType");
#endif
                return static_cast<D3D12_COMMAND_LIST_TYPE>(-1);
            }
        }
    }

    // ===========================================================================
    // Create command dispatcher
    std::shared_ptr<RHICommandAllocator> DeviceD3D12::CreateCommandAllocator(RHICmdType type)
    {
        D3D12_COMMAND_LIST_TYPE d3dType = ConvertRHICmdTypeToD3D12(type);

        // Create command allocator this is the only interface that has no version 1，2，3 etc
        ComPtr<ID3D12CommandAllocator> pAllocator;
        ThrowIfFailed(m_pDevice->CreateCommandAllocator(d3dType, IID_PPV_ARGS(&pAllocator)));
        return std::make_shared<CommandAllocatorD3D12>(type, pAllocator.Get());
    }
    
    std::shared_ptr<RHICommandList> DeviceD3D12::CreateCommandList(std::shared_ptr<RHICommandAllocator>& allocator)
    {
        CommandAllocatorD3D12* pAllocator = SafeCast<CommandAllocatorD3D12>(allocator.get());
        if (pAllocator == nullptr) {
            ThrowErrorMessage("CommandAllocatorD3D12 is nullptr");
            return nullptr;
        }
        RHICmdType type = pAllocator->GetCmdType();
        D3D12_COMMAND_LIST_TYPE d3dType = ConvertRHICmdTypeToD3D12(type);
    
        // Create command list
        ComPtr<ID3D12GraphicsCommandList> pCmdList = CreateLevelCommandList(
            m_pDevice.Get(),
            d3dType,
            pAllocator->GetCommandAllocator(),
            GetFeatureLevel()
        );

        // Initial state is closed
        ThrowIfFailed(pCmdList->Close());
    
        // Create RHI wrapper object
        return std::make_shared<CommandListD3D12>(pAllocator, pCmdList.Get());
    }
    // ===========================================================================

    void CommandListD3D12::BeginRecording()
    {
        // use command allocator
        CommandAllocatorD3D12* dx12CmdAllocator = GetAllocator();

        if(dx12CmdAllocator == nullptr){
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("CommandAllocatorD3D12 is nullptr");
#endif
            return;
        }

        ThrowIfFailed(dx12CmdAllocator->GetCommandAllocator()->Reset());
        ThrowIfFailed(m_pCommandList->Reset(dx12CmdAllocator->GetCommandAllocator(), nullptr));
    }

    void CommandListD3D12::EndRecording()
    {
        ThrowIfFailed(m_pCommandList->Close());
    }

    void CommandQueueD3D12::ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists)
    {
        std::vector<ID3D12CommandList*> d3dCmdLists;
        d3dCmdLists.reserve(cmdLists.size());
        
        for (const auto& cmdList : cmdLists)
        {
            auto dx12CmdList = std::static_pointer_cast<CommandListD3D12>(cmdList);
            d3dCmdLists.push_back(dx12CmdList->GetCommandList());
        }
        
        m_pCommandQueue->ExecuteCommandLists((UINT)d3dCmdLists.size(), d3dCmdLists.data());
    } 

    void CommandQueueD3D12::BeginFrame()
    {
        // NOT implemented
    }
    
    void CommandQueueD3D12::EndFrame()
    {
        // add index
        UINT64 fenceValue = m_nextFenceValue++;
        ThrowIfFailed(m_pCommandQueue->Signal(m_Fence.Get(), fenceValue));
        
        // Save fence value
        m_fenceValues[m_currentFrame] = fenceValue;
        // Advance to next frame
        m_currentFrame = (m_currentFrame + 1) % RHI_MULTI_BUFFERING;
    }

    void CommandQueueD3D12::WaitForGPU()
    {
        for (int i = 0; i < RHI_MULTI_BUFFERING; ++i) {
            UINT64 fenceValue = m_fenceValues[i];
            if (m_Fence->GetCompletedValue() < fenceValue)
            {
                ThrowIfFailed(m_Fence->SetEventOnCompletion(fenceValue, m_fenceEvent));
                WaitForSingleObject(m_fenceEvent, INFINITE);
            }
        }
    }

    void CommandQueueD3D12::Signal(uint64_t fenceValue)
    {   
        ThrowIfFailed(m_pCommandQueue->Signal(m_Fence.Get(), fenceValue));
    }

    bool CommandQueueD3D12::GetTimestampFrequency(uint64_t* frequency)
    {
        return SUCCEEDED(m_pCommandQueue->GetTimestampFrequency(frequency));
    }

    uint64_t CommandQueueD3D12::GetFrameIndex() const
    {
        return m_currentFrame;
    }
}
