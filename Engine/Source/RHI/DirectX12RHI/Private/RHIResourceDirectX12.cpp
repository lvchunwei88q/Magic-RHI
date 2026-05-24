/*
 * 因为使用到了前向声明所以需要先引入声明定义
 */
#include "RHICommandListDirectX12.h"
#include "RHIDirectX12.h"
#include "DirectXHelper.h"

namespace RHI
{
    namespace
    {
        // D3D12 转换
        D3D12_HEAP_TYPE ToD3D12HeapType(BufferHeapType type)
        {
            switch (type)
            {
            case BufferHeapType::Default:  return D3D12_HEAP_TYPE_DEFAULT;
            case BufferHeapType::Upload:   return D3D12_HEAP_TYPE_UPLOAD;
            case BufferHeapType::Readback: return D3D12_HEAP_TYPE_READBACK;
            default:                       return D3D12_HEAP_TYPE_DEFAULT;
            }
        }

        D3D12_RESOURCE_STATES ToD3D12InitialState(BufferHeapType type)
        {
            switch (type)
            {
            case BufferHeapType::Default:  return D3D12_RESOURCE_STATE_COMMON;
            case BufferHeapType::Upload:   return D3D12_RESOURCE_STATE_GENERIC_READ;
            case BufferHeapType::Readback: return D3D12_RESOURCE_STATE_COPY_DEST;
            default:                       return D3D12_RESOURCE_STATE_COMMON;
            }
        }

        D3D12_FILTER ConvertFilter(SamplerFilter filter)
        {
            switch (filter)
            {
            case SamplerFilter::Point:
                return D3D12_FILTER_MIN_MAG_MIP_POINT;
            case SamplerFilter::Bilinear:
                return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            case SamplerFilter::Trilinear:
                return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            case SamplerFilter::Anisotropic:
                return D3D12_FILTER_ANISOTROPIC;
            default:
                return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            }
        }

        D3D12_TEXTURE_ADDRESS_MODE ConvertAddressMode(SamplerAddressMode mode)
        {
            switch (mode)
            {
            case SamplerAddressMode::Wrap:
                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            case SamplerAddressMode::Mirror:
                return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            case SamplerAddressMode::Clamp:
                return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case SamplerAddressMode::Border:
                return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            case SamplerAddressMode::MirrorOnce:
                return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
            default:
                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            }
        }

        D3D12_COMPARISON_FUNC ConvertComparisonFunc(SamplerComparisonFunc func)
        {
            switch (func)
            {
            case SamplerComparisonFunc::Never:
                return D3D12_COMPARISON_FUNC_NEVER;
            case SamplerComparisonFunc::Less:
                return D3D12_COMPARISON_FUNC_LESS;
            case SamplerComparisonFunc::Equal:
                return D3D12_COMPARISON_FUNC_EQUAL;
            case SamplerComparisonFunc::LessEqual:
                return D3D12_COMPARISON_FUNC_LESS_EQUAL;
            case SamplerComparisonFunc::Greater:
                return D3D12_COMPARISON_FUNC_GREATER;
            case SamplerComparisonFunc::NotEqual:
                return D3D12_COMPARISON_FUNC_NOT_EQUAL;
            case SamplerComparisonFunc::GreaterEqual:
                return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
            case SamplerComparisonFunc::Always:
                return D3D12_COMPARISON_FUNC_ALWAYS;
            default:
                return D3D12_COMPARISON_FUNC_NEVER;
            }
        }
    }

    std::shared_ptr<RHISamplerState> RHIDirectX12::CreateSamplerState(const SamplerStateDesc& desc)
    {
        D3D12_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = ConvertFilter(desc.Filter);
        samplerDesc.AddressU = ConvertAddressMode(desc.AddressU);
        samplerDesc.AddressV = ConvertAddressMode(desc.AddressV);
        samplerDesc.AddressW = ConvertAddressMode(desc.AddressW);
        samplerDesc.MipLODBias = desc.MipLODBias;
        samplerDesc.MaxAnisotropy = desc.MaxAnisotropy;
        samplerDesc.ComparisonFunc = ConvertComparisonFunc(desc.ComparisonFunc);
        samplerDesc.BorderColor[0] = desc.BorderColor[0];
        samplerDesc.BorderColor[1] = desc.BorderColor[1];
        samplerDesc.BorderColor[2] = desc.BorderColor[2];
        samplerDesc.BorderColor[3] = desc.BorderColor[3];
        samplerDesc.MinLOD = desc.MinLOD;
        samplerDesc.MaxLOD = desc.MaxLOD;

        uint32_t index = m_SamplerHeapAllocator.Allocate();
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetSamplerCPUHandle(index);

        m_pDevice->CreateSampler(&samplerDesc, cpuHandle);

        RHIDescriptorHandle handle = {(uint8_t)RHIDescriptorHeapType::Sampler, index};

        return std::make_shared<SamplerStateDirectX12>(samplerDesc, handle);
    }

    void RHIDirectX12::DeleteSamplerState(std::shared_ptr<RHI::RHISamplerState>& samplerState) 
    {
        m_SamplerHeapAllocator.Free(samplerState->GetBindlessHandle().GetIndex());
        samplerState.reset();
    }

    std::shared_ptr<RHIBuffer> RHIDirectX12::CreateBuffer(const BufferDesc& desc)
    {
        D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(ToD3D12HeapType(desc.HeapType));
        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(desc.SizeInBytes, D3D12_RESOURCE_FLAG_NONE);

        auto InitBufferData = [&](ComPtr<ID3D12Resource>& pResource){
            void* pData;
            ThrowIfFailed(pResource->Map(0, nullptr, &pData));
            memcpy(pData, desc.InitialData, desc.SizeInBytes);
            pResource->Unmap(0, nullptr);
        };  

        ComPtr<ID3D12Resource> pResource;
        ThrowIfFailed(m_pDevice->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,                 // 无标志 不优化
            &resourceDesc,
            ToD3D12InitialState(desc.HeapType),
            nullptr,
            IID_PPV_ARGS(&pResource)));

        std::shared_ptr<BufferDirectX12> buffer = std::make_shared<BufferDirectX12>(pResource.Get(), desc, m_pDevice.Get());

        if(desc.HeapType == BufferHeapType::Default && desc.InitialData != nullptr){ // 需要Copy数据到Default堆
            // create Upload heap
            D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            ComPtr<ID3D12Resource> pUploadBuffer;
            
            ThrowIfFailed(m_pDevice->CreateCommittedResource(
                &uploadHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&pUploadBuffer)));
            
            // initial data to Upload heap
            InitBufferData(pUploadBuffer);
            std::shared_ptr<BufferDirectX12> uploadBuffer = std::make_shared<BufferDirectX12>(pUploadBuffer.Get(), desc, m_pDevice.Get());
            
            auto cmdList = CreateCommandList(RHICmdListType::Copy);
            cmdList->BeginRecording();
            
            // Default 资源初始状态是 COMMON，需要转换到 COPY_DEST
            RHI::BarrierDesc barrier = {};
            barrier.Type = RHI::ResourceBarrierType::Transition;
            barrier.Flags = RHI::ResourceBarrierFlags::None;
            barrier.Transition.pResource = buffer.get();
            barrier.Transition.Subresource = ~0u;  // 所有子资源
            barrier.Transition.StateBefore = static_cast<uint64_t>(D3D12_RESOURCE_STATE_COMMON);
            barrier.Transition.StateAfter = static_cast<uint64_t>(D3D12_RESOURCE_STATE_COPY_DEST);

            cmdList->ResourceBarrier(1, &barrier);
            
            cmdList->CopyBufferRegion(
                buffer.get(),         // 目标：Default 堆缓冲区
                0,                     // 目标偏移
                uploadBuffer.get(),   // 源：Upload 堆缓冲区
                0,                     // 源偏移
                desc.SizeInBytes        // 复制大小
            );

            RHI::BarrierDesc finalBarrier = {};
            finalBarrier.Type = RHI::ResourceBarrierType::Transition;
            finalBarrier.Flags = RHI::ResourceBarrierFlags::None;
            finalBarrier.Transition.pResource = buffer.get();
            finalBarrier.Transition.Subresource = ~0u;
            finalBarrier.Transition.StateBefore = static_cast<uint64_t>(D3D12_RESOURCE_STATE_COPY_DEST);
            finalBarrier.Transition.StateAfter = static_cast<uint64_t>(D3D12_RESOURCE_STATE_COMMON);
            cmdList->ResourceBarrier(1, &finalBarrier);
            
            cmdList->EndRecording();
            
            // run command list
            m_CopyQueue.get()->ExecuteCommandLists({cmdList});
            m_CopyQueue.get()->WaitForIdle();
            
            // return buffer
            return buffer;
        }
#if RHI_ENABLE_RESOURCE_INFO
        else if(desc.HeapType == BufferHeapType::Default && desc.InitialData == nullptr) 
            ThrowErrorMessage("Creating D3D12_HEAP_TYPE_DEFAULT requires providing heap data");
#endif

        if(desc.HeapType != BufferHeapType::Default && desc.InitialData != nullptr){
            // initial data to Upload heap
            InitBufferData(pResource);
        }

        return buffer;
    }

    void RHIDirectX12::DeleteBuffer(std::shared_ptr<RHI::RHIBuffer>& buffer)
    {
        buffer.reset();
    }

    void CommandListDirectX12::BeginRecording()
    {
        ThrowIfFailed(m_pCommandAllocator->Reset());
        ThrowIfFailed(m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr));
    }

    void CommandListDirectX12::EndRecording()
    {
        ThrowIfFailed(m_pCommandList->Close());
    }

    void CommandQueueDirectX12::ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists)
    {
        std::vector<ID3D12CommandList*> d3dCmdLists;
        d3dCmdLists.reserve(cmdLists.size());
        
        for (const auto& cmdList : cmdLists)
        {
            auto dx12CmdList = std::static_pointer_cast<CommandListDirectX12>(cmdList);
            d3dCmdLists.push_back(dx12CmdList->GetCommandList());
        }
        
        m_pCommandQueue->ExecuteCommandLists((UINT)d3dCmdLists.size(), d3dCmdLists.data());
    }

    void CommandQueueDirectX12::WaitForIdle()
    {
        UINT64 fenceValue = ++m_FenceValue;
        ThrowIfFailed(m_pCommandQueue->Signal(m_Fence.Get(), fenceValue));
        
        if (m_Fence->GetCompletedValue() < fenceValue)
        {
            HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
            ThrowIfFailed(m_Fence->SetEventOnCompletion(fenceValue, eventHandle));
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }
    }

    uint64_t CommandQueueDirectX12::Signal()
    {
        if (!m_Fence)
        {
            ThrowIfFailed(GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
        }
        
        UINT64 fenceValue = ++m_FenceValue;
        ThrowIfFailed(m_pCommandQueue->Signal(m_Fence.Get(), fenceValue));
        return fenceValue;
    }

    bool CommandQueueDirectX12::GetTimestampFrequency(uint64_t* frequency)
    {
        return SUCCEEDED(m_pCommandQueue->GetTimestampFrequency(frequency));
    }

    bool CommandQueueDirectX12::SetEventOnCompletion(uint64_t fenceValue, void* hEvent)
    {
        if (!m_Fence)
        {
            ThrowIfFailed(GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
        }
        return SUCCEEDED(m_Fence->SetEventOnCompletion(fenceValue, (HANDLE)hEvent));
    }

    uint64_t CommandQueueDirectX12::GetCompletedValue() const
    {
        if (m_Fence)
        {
            return m_Fence->GetCompletedValue();
        }
        return 0;
    }
}
