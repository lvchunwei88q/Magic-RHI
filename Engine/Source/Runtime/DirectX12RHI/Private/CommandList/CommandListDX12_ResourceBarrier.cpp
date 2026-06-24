#include <CoreMinimal.h>
#include "RHICommandListD3D12.h"

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

        D3D12_RESOURCE_STATES ConvertToD3D12State(RHIResourceState state)
        {
            // 这两个状态特殊处理
            if (state == RHIResourceState::Common)
                return D3D12_RESOURCE_STATE_COMMON;
            if (state == RHIResourceState::Present)
                return D3D12_RESOURCE_STATE_PRESENT;

            switch (state)
            {
                // 基础状态
                case RHIResourceState::VertexBuffer:                return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                case RHIResourceState::IndexBuffer:                 return D3D12_RESOURCE_STATE_INDEX_BUFFER;
                case RHIResourceState::RenderTarget:                return D3D12_RESOURCE_STATE_RENDER_TARGET;
                case RHIResourceState::UnorderedAccess:             return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
                case RHIResourceState::DepthWrite:                  return D3D12_RESOURCE_STATE_DEPTH_WRITE;
                case RHIResourceState::DepthRead:                   return D3D12_RESOURCE_STATE_DEPTH_READ;
                case RHIResourceState::NonPixelShaderResource:      return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
                case RHIResourceState::PixelShaderResource:         return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
                case RHIResourceState::StreamOut:                   return D3D12_RESOURCE_STATE_STREAM_OUT;
                case RHIResourceState::IndirectArgument:            return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
                case RHIResourceState::CopyDest:                    return D3D12_RESOURCE_STATE_COPY_DEST;
                case RHIResourceState::CopySource:                  return D3D12_RESOURCE_STATE_COPY_SOURCE;
                case RHIResourceState::ResolveDest:                 return D3D12_RESOURCE_STATE_RESOLVE_DEST;
                case RHIResourceState::ResolveSource:               return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
                // 视频相关
                case RHIResourceState::VideoDecodeRead:             return D3D12_RESOURCE_STATE_VIDEO_DECODE_READ;
                case RHIResourceState::VideoDecodeWrite:            return D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE;
                case RHIResourceState::VideoProcessRead:            return D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ;
                case RHIResourceState::VideoProcessWrite:           return D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE;
                case RHIResourceState::VideoEncodeRead:             return D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ;
                case RHIResourceState::VideoEncodeWrite:            return D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE;
                // 光线追踪
                case RHIResourceState::RaytracingAccelerationStructure: return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
                // 着色率
                case RHIResourceState::ShadingRateSource:           return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
                // 组合状态
                case RHIResourceState::GenericRead:                 return D3D12_RESOURCE_STATE_GENERIC_READ;
                case RHIResourceState::AllShaderResource:           return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
                default:
                    return D3D12_RESOURCE_STATE_COMMON;
            }
        }

        ID3D12Resource* GetD3D12Resource(RHIResource* pResource, ResourceType type)
        {
            switch(type){
                case ResourceType::Texture:
                {
                    return SafeCast<TextureD3D12>(pResource)->GetResource();
                }
                case ResourceType::Buffer:
                {
                    return SafeCast<BufferD3D12>(pResource)->GetResource();
                }
                default:
                {
                    ThrowErrorMessage("Invalid resource type for resource barrier");
                    return nullptr;
                }
            }
        }
    }

    void CommandListD3D12::ResourceBarrier(uint32_t numBarriers, const BarrierDesc* pBarriers)
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
                d3d12Barrier.Type                       = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                d3d12Barrier.Flags                      = ConvertResourceBarrierFlags(rhiBarrier.Flags);
                
                // 获取 D3D12 资源指针
                d3d12Barrier.Transition.pResource       = GetD3D12Resource(rhiBarrier.Transition.pResource, rhiBarrier.ResourceType);
                d3d12Barrier.Transition.Subresource     = rhiBarrier.Transition.Subresource;
                d3d12Barrier.Transition.StateBefore     = ConvertToD3D12State(rhiBarrier.Transition.StateBefore);
                d3d12Barrier.Transition.StateAfter      = ConvertToD3D12State(rhiBarrier.Transition.StateAfter);
                break;
            }
            case ResourceBarrierType::Aliasing:
            {
                d3d12Barrier.Type                       = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
                d3d12Barrier.Flags                      = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                
                d3d12Barrier.Aliasing.pResourceBefore   = GetD3D12Resource(rhiBarrier.Aliasing.pResourceBefore, rhiBarrier.ResourceType);
                d3d12Barrier.Aliasing.pResourceAfter    = GetD3D12Resource(rhiBarrier.Aliasing.pResourceAfter, rhiBarrier.ResourceType);
                break;
            }
            case ResourceBarrierType::UAV:
            {
                d3d12Barrier.Type                       = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                d3d12Barrier.Flags                      = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                
                d3d12Barrier.UAV.pResource              = GetD3D12Resource(rhiBarrier.UAV.pResource, rhiBarrier.ResourceType);
                break;
            }
            default:
#if RHI_ENABLE_RESOURCE_INFO
                ThrowErrorMessage("Unknown barrier type");
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
