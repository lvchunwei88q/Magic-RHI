/*
 * 因为使用到了前向声明所以需要先引入声明定义
 */
 #include <Common/Check.h>
#include "RHICommandListD3D12.h"
#include "RHID3D12.h"

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

        inline D3D12_RESOURCE_FLAGS ConvertBindFlagsToResourceFlags(BufferBindFlag BindFlags)
        {
            D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;
            
            // UAV
            if (EnumHasAnyFlags(BindFlags, BufferBindFlag::UnorderedAccess))
            {
                Flags = static_cast<D3D12_RESOURCE_FLAGS>(
                    static_cast<uint32_t>(Flags) | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
                );
            }
            
            // RenderTarget
            if (EnumHasAnyFlags(BindFlags, BufferBindFlag::RenderTarget))
            {
                Flags = static_cast<D3D12_RESOURCE_FLAGS>(
                    static_cast<uint32_t>(Flags) | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
                );
            }
            
            // DepthStencilTarget
            if (EnumHasAnyFlags(BindFlags, BufferBindFlag::DepthStencil))
            {
                Flags = static_cast<D3D12_RESOURCE_FLAGS>(
                    static_cast<uint32_t>(Flags) | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
                );
            }
            
            if (!EnumHasAnyFlags(BindFlags, BufferBindFlag::ShaderResource) &&
                !EnumHasAnyFlags(BindFlags, BufferBindFlag::ConstantBuffer))
            {
                // 如果不是作为 SRV 使用，可以禁止着色器资源访问
                Flags = static_cast<D3D12_RESOURCE_FLAGS>(
                    static_cast<uint32_t>(Flags) | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
                );
            }
            
            return Flags;
        }
    }

    std::shared_ptr<RHISamplerState> RHID3D12::CreateSamplerState(const SamplerStateDesc& desc)
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

        uint32_t index = m_pSamplerHeap->Allocate().GetIndex();
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_pSamplerHeap->GetCPUHandle(index);

        m_pDevice->CreateSampler(&samplerDesc, cpuHandle);

        RHIDescriptorHandle handle = {(uint8_t)RHIDescriptorHeapType::Sampler, index};

        return std::make_shared<SamplerStateD3D12>(samplerDesc, handle);
    }

    void RHID3D12::DeleteSamplerState(std::shared_ptr<RHI::RHISamplerState>& samplerState) 
    {
        m_pSamplerHeap->Free(samplerState->GetBindlessHandle());
        samplerState.reset();
    }

    std::shared_ptr<RHIBuffer> RHID3D12::CreateBuffer(BufferDesc& desc)
    {
        auto isConstantBuffer = [](BufferBindFlag flag) -> bool {
            return flag == BufferBindFlag::ConstantBuffer;
        };
        
        desc.SizeInBytes = isConstantBuffer(desc.BindFlags) ? AlignUp(desc.SizeInBytes, 256) : desc.SizeInBytes;

        D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(ToD3D12HeapType(desc.HeapType));
        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
            desc.SizeInBytes, ConvertBindFlagsToResourceFlags(desc.BindFlags));

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

        std::shared_ptr<BufferD3D12> buffer = std::make_shared<BufferD3D12>(pResource.Get(), desc, m_pDevice.Get());

        if(desc.HeapType == BufferHeapType::Default && desc.InitialData != nullptr){ // 需要Copy数据到Default堆
            // create Upload heap
            D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            ComPtr<ID3D12Resource> pUploadBuffer;
            
            D3D12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(
            desc.SizeInBytes, 
                D3D12_RESOURCE_FLAG_NONE  // 强制无任何标志
            );
            
            ThrowIfFailed(m_pDevice->CreateCommittedResource(
                &uploadHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &uploadDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&pUploadBuffer)));
            
            // initial data to Upload heap
            InitBufferData(pUploadBuffer);
            std::shared_ptr<BufferD3D12> uploadBuffer = std::make_shared<BufferD3D12>(pUploadBuffer.Get(), desc, m_pDevice.Get());
            
            auto cmdAllocator = CreateCommandAllocator(RHICmdType::Copy);
            auto cmdList = CreateCommandList(cmdAllocator);
            m_CopyQueue.get()->BeginFrame();
            cmdList->BeginRecording();
            
            // Default 资源初始状态是 COMMON，需要转换到 COPY_DEST
            RHI::BarrierDesc barrier = {};
            barrier.Type                        = RHI::ResourceBarrierType::Transition;
            barrier.ResourceType                = ResourceType::Buffer;
            barrier.Flags                       = RHI::ResourceBarrierFlags::None;
            barrier.Transition.pResource        = buffer.get();
            barrier.Transition.Subresource      = ~0u;  // 所有子资源
            barrier.Transition.StateBefore      = RHIResourceState::Common;
            barrier.Transition.StateAfter       = RHIResourceState::CopyDest;

            cmdList->ResourceBarrier(1, &barrier);
            
            cmdList->CopyBufferRegion(
                buffer.get(),         // 目标：Default 堆缓冲区
                0,                     // 目标偏移
                uploadBuffer.get(),   // 源：Upload 堆缓冲区
                0,                     // 源偏移
                desc.SizeInBytes        // 复制大小
            );

            RHI::BarrierDesc finalBarrier = {};
            finalBarrier.Type                        = RHI::ResourceBarrierType::Transition;
            finalBarrier.ResourceType                = ResourceType::Buffer;
            finalBarrier.Flags                       = RHI::ResourceBarrierFlags::None;
            finalBarrier.Transition.pResource        = buffer.get();
            finalBarrier.Transition.Subresource      = ~0u;
            finalBarrier.Transition.StateBefore      = RHIResourceState::CopyDest;
            finalBarrier.Transition.StateAfter       = RHIResourceState::Common;
            cmdList->ResourceBarrier(1, &finalBarrier);
            
            cmdList->EndRecording();
            
            // run command list
            m_CopyQueue.get()->ExecuteCommandLists({cmdList});
            m_CopyQueue.get()->EndFrame();
            m_CopyQueue.get()->WaitForGPU();
            
            // return buffer
            return buffer;
        }
#if RHI_ENABLE_RESOURCE_DEBUG_INFO
        else if(desc.HeapType == BufferHeapType::Default && desc.InitialData == nullptr) 
            ThrowErrorMessage("Creating D3D12_HEAP_TYPE_DEFAULT requires providing heap data");
#endif

        if(desc.HeapType != BufferHeapType::Default && desc.InitialData != nullptr){
            // initial data to Upload heap
            InitBufferData(pResource);
        }

        return buffer;
    }

    void RHID3D12::DeleteBuffer(std::shared_ptr<RHI::RHIBuffer>& buffer)
    {
        // 只有在有描述符句柄时才需要释放
        if(buffer->HasDescriptorHandle())
            m_pStandardHeap->Free(buffer->GetBindlessHandle());
        buffer.reset();
    }

    void CommandListD3D12::BeginRecording()
    {
        if(m_pAllocator == nullptr){
#ifdef RHI_ENABLE_RESOURCE_DEBUG_INFO
            ThrowErrorMessage("CommandAllocatorD3D12 is nullptr");
#endif
            return;
        }
        CommandAllocatorD3D12* dx12CmdAllocator = SafeCast<CommandAllocatorD3D12>(m_pAllocator);

        if(dx12CmdAllocator == nullptr){
#ifdef RHI_ENABLE_RESOURCE_DEBUG_INFO
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

    bool CommandQueueD3D12::SetEventOnCompletion(uint64_t fenceValue, void* hEvent)
    {
        return SUCCEEDED(m_Fence->SetEventOnCompletion(fenceValue, (HANDLE)hEvent));
    }

    uint64_t CommandQueueD3D12::GetFrameIndex() const
    {
        return m_currentFrame;
    }
}
