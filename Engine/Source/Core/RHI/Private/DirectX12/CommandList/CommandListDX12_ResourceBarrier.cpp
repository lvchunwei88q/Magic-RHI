#include "DirectX12/RHIResourceDirectX12.h"

namespace RHI
{
    namespace
    {
        D3D12_RESOURCE_BARRIER_FLAGS ConvertResourceBarrierFlags(ResourceBarrierFlags flags)
        {
            D3D12_RESOURCE_BARRIER_FLAGS d3d12Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            
            if ((flags & ResourceBarrierFlags::BeginOnly) != ResourceBarrierFlags::None)
                d3d12Flags |= D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
            
            if ((flags & ResourceBarrierFlags::EndOnly) != ResourceBarrierFlags::None)
                d3d12Flags |= D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
            
            return d3d12Flags;
        }
    }

    void CommandListDirectX12::ResourceBarrier(uint32_t numBarriers, const BarrierDesc* pBarriers)
    {
        if (numBarriers == 0 || pBarriers == nullptr)
            return;
        
        // 转换 RHI 屏障描述到 D3D12 屏障
        std::vector<D3D12_RESOURCE_BARRIER> d3d12Barriers;
        d3d12Barriers.reserve(numBarriers);
        
        for (uint32_t i = 0; i < numBarriers; ++i)
        {
            const auto& rhiBarrier = pBarriers[i];
            D3D12_RESOURCE_BARRIER d3d12Barrier = {};
            
            switch (rhiBarrier.Type)
            {
            case ResourceBarrierType::Transition:
            {
                d3d12Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                d3d12Barrier.Flags = ConvertResourceBarrierFlags(rhiBarrier.Flags);
                
                // 获取 D3D12 资源指针
                auto* buffer = static_cast<BufferDirectX12*>(rhiBarrier.Transition.pResource);
                d3d12Barrier.Transition.pResource = buffer ? SafeCast<ID3D12Resource>(buffer->GetResource()) : nullptr;
                d3d12Barrier.Transition.Subresource = rhiBarrier.Transition.Subresource;
                d3d12Barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(rhiBarrier.Transition.StateBefore);
                d3d12Barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(rhiBarrier.Transition.StateAfter);
                break;
            }
            case ResourceBarrierType::Aliasing:
            {
                d3d12Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
                d3d12Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                
                auto* pResourceBefore = static_cast<BufferDirectX12*>(rhiBarrier.Aliasing.pResourceBefore);
                auto* pResourceAfter = static_cast<BufferDirectX12*>(rhiBarrier.Aliasing.pResourceAfter);
                d3d12Barrier.Aliasing.pResourceBefore = pResourceBefore ? SafeCast<ID3D12Resource>(pResourceBefore->GetResource()) : nullptr;
                d3d12Barrier.Aliasing.pResourceAfter = pResourceAfter ? SafeCast<ID3D12Resource>(pResourceAfter->GetResource()) : nullptr;
                break;
            }
            case ResourceBarrierType::UAV:
            {
                d3d12Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                d3d12Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                
                auto* pResource = static_cast<BufferDirectX12*>(rhiBarrier.UAV.pResource);
                d3d12Barrier.UAV.pResource = pResource ? SafeCast<ID3D12Resource>(pResource->GetResource()) : nullptr;
                break;
            }
            default:
#if RHI_ENABLE_RESOURCE_INFO
                ThrowIfFailed("Unknown barrier type");
#endif
                Core::ErrorCapture::Capture("Unknown barrier type");
                continue;
            }
            
            d3d12Barriers.push_back(d3d12Barrier);
        }
        
        // 调用 D3D12 命令列表的 ResourceBarrier
        if (!d3d12Barriers.empty())
        {
            m_pCommandList->ResourceBarrier(
                static_cast<UINT>(d3d12Barriers.size()),
                d3d12Barriers.data()
            );
        }
    }
}
