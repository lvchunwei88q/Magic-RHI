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

        D3D12_PRIMITIVE_TOPOLOGY ConvertPrimitiveTopology(RHIPrimitiveTopology topology)
        {
            switch (topology)
            {
            case RHIPrimitiveTopology::PointList: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            case RHIPrimitiveTopology::LineList: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            case RHIPrimitiveTopology::LineStrip: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case RHIPrimitiveTopology::TriangleList: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case RHIPrimitiveTopology::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            case RHIPrimitiveTopology::LineListAdj: return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
            case RHIPrimitiveTopology::LineStripAdj: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
            case RHIPrimitiveTopology::TriangleListAdj: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
            case RHIPrimitiveTopology::TriangleStripAdj: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
            default: return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
            }
        }

        D3D12_PRIMITIVE_TOPOLOGY ConvertControlPointPatchList(uint32_t controlPointCount)
        {
            switch (controlPointCount)
            {
            case 1: return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
            case 2: return D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
            case 3: return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
            case 4: return D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
            case 5: return D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST;
            case 6: return D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST;
            case 7: return D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST;
            case 8: return D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST;
            case 9: return D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST;
            case 10: return D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST;
            case 11: return D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST;
            case 12: return D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST;
            case 13: return D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST;
            case 14: return D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST;
            case 15: return D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST;
            case 16: return D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
            case 17: return D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST;
            case 18: return D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST;
            case 19: return D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST;
            case 20: return D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
            case 21: return D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST;
            case 22: return D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST;
            case 23: return D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST;
            case 24: return D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST;
            case 25: return D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
            case 26: return D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST;
            case 27: return D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST;
            case 28: return D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST;
            case 29: return D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST;
            case 30: return D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST;
            case 31: return D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST;
            case 32: return D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST;
            default: return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
            }
        }

        DXGI_FORMAT ConvertIndexFormat(RHIIndexFormat format)
        {
            switch (format)
            {
            case RHIIndexFormat::Uint16: return DXGI_FORMAT_R16_UINT;
            case RHIIndexFormat::Uint32: return DXGI_FORMAT_R32_UINT;
            default: return DXGI_FORMAT_UNKNOWN;
            }
        }

        D3D12_CLEAR_FLAGS ConvertClearFlags(RHIClearFlags flags)
        {
            int d3dFlags = 0;
            
            if ((flags & RHIClearFlags::Depth) != RHIClearFlags::None)
                d3dFlags |= D3D12_CLEAR_FLAG_DEPTH;
            
            if ((flags & RHIClearFlags::Stencil) != RHIClearFlags::None)
                d3dFlags |= D3D12_CLEAR_FLAG_STENCIL;
            
            return static_cast<D3D12_CLEAR_FLAGS>(d3dFlags);
        }

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
            ThrowIfFailed("Creating D3D12_HEAP_TYPE_DEFAULT requires providing heap data");
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

    // 输入装配器
    void CommandListDirectX12::IASetPrimitiveTopology(RHIPrimitiveTopology topology, uint32_t controlPointCount)
    {
        if (topology == RHIPrimitiveTopology::ControlPointPatchList)
        {
            m_pCommandList->IASetPrimitiveTopology(ConvertControlPointPatchList(controlPointCount));
        }
        else
        {
            m_pCommandList->IASetPrimitiveTopology(ConvertPrimitiveTopology(topology));
        }
    }

    void CommandListDirectX12::IASetVertexBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers, const uint64_t* pOffsets)
    {
        std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews;
        vertexBufferViews.reserve(numBuffers);

        for (uint32_t i = 0; i < numBuffers; ++i)
        {
            BufferDirectX12* dxBuffer = static_cast<BufferDirectX12*>(ppBuffers[i]);
            D3D12_VERTEX_BUFFER_VIEW view = {};
            view.BufferLocation = SafeCast<ID3D12Resource>(dxBuffer->GetResource())->GetGPUVirtualAddress();
            view.StrideInBytes = dxBuffer->GetStride();
            view.SizeInBytes = (UINT)dxBuffer->GetSize();
            vertexBufferViews.push_back(view);
        }

        m_pCommandList->IASetVertexBuffers(startSlot, numBuffers, vertexBufferViews.data());
    }

    void CommandListDirectX12::IASetIndexBuffer(RHIBuffer* pIndexBuffer, RHIIndexFormat format, uint64_t offset)
    {
        if (pIndexBuffer)
        {
            BufferDirectX12* dxBuffer = static_cast<BufferDirectX12*>(pIndexBuffer);
            D3D12_INDEX_BUFFER_VIEW view = {};  
            view.BufferLocation = SafeCast<ID3D12Resource>(dxBuffer->GetResource())->GetGPUVirtualAddress() + offset;
            view.Format = ConvertIndexFormat(format);
            view.SizeInBytes = (UINT)dxBuffer->GetSize() - (UINT)offset;
            m_pCommandList->IASetIndexBuffer(&view);
        }
        else
        {
            m_pCommandList->IASetIndexBuffer(nullptr);
        }
    }

    // 着色器 - 占位符实现
    void CommandListDirectX12::VSSetShader(RHIVertexShader* pShader) {}
    void CommandListDirectX12::PSSetShader(RHIPixelShader* pShader) {}
    void CommandListDirectX12::GSSetShader(RHIGeometryShader* pShader) {}
    void CommandListDirectX12::HSSetShader(RHIHullShader* pShader) {}
    void CommandListDirectX12::DSSetShader(RHIDomainShader* pShader) {}
    void CommandListDirectX12::CSSetShader(RHIComputeShader* pShader) {}

    // 着色器资源 - 占位符实现
    void CommandListDirectX12::VSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) {}
    void CommandListDirectX12::PSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) {}
    void CommandListDirectX12::GSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) {}
    void CommandListDirectX12::HSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) {}
    void CommandListDirectX12::DSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) {}
    void CommandListDirectX12::CSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) {}

    void CommandListDirectX12::VSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) {}
    void CommandListDirectX12::PSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) {}
    void CommandListDirectX12::GSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) {}
    void CommandListDirectX12::HSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) {}
    void CommandListDirectX12::DSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) {}
    void CommandListDirectX12::CSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) {}

    void CommandListDirectX12::VSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) {}
    void CommandListDirectX12::PSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) {}
    void CommandListDirectX12::GSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) {}
    void CommandListDirectX12::HSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) {}
    void CommandListDirectX12::DSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) {}
    void CommandListDirectX12::CSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) {}

    // 光栅器
    void CommandListDirectX12::RSSetViewports(uint32_t numViewports, const RHIViewport* pViewports)
    {
        std::vector<D3D12_VIEWPORT> d3dViewports;
        d3dViewports.reserve(numViewports);
        for (uint32_t i = 0; i < numViewports; ++i)
        {
            D3D12_VIEWPORT vp = {};
            vp.TopLeftX = pViewports[i].topLeftX;
            vp.TopLeftY = pViewports[i].topLeftY;
            vp.Width = pViewports[i].width;
            vp.Height = pViewports[i].height;
            vp.MinDepth = pViewports[i].minDepth;
            vp.MaxDepth = pViewports[i].maxDepth;
            d3dViewports.push_back(vp);
        }
        m_pCommandList->RSSetViewports(numViewports, d3dViewports.data());
    }

    void CommandListDirectX12::RSSetScissorRects(uint32_t numRects, const RHIRect* pRects)
    {
        std::vector<D3D12_RECT> d3dRects;
        d3dRects.reserve(numRects);
        for (uint32_t i = 0; i < numRects; ++i)
        {
            D3D12_RECT rect = {};
            rect.left = (LONG)pRects[i].left;
            rect.top = (LONG)pRects[i].top;
            rect.right = (LONG)pRects[i].right;
            rect.bottom = (LONG)pRects[i].bottom;
            d3dRects.push_back(rect);
        }
        m_pCommandList->RSSetScissorRects(numRects, d3dRects.data());
    }

    void CommandListDirectX12::RSSetState(RHIRasterizerState* pState) {}

    // 输出合并器 - 占位符实现
    void CommandListDirectX12::OMSetRenderTargets(uint32_t numRenderTargets, RHIRenderTargetView* const* ppViews, RHIDepthStencilView* pDepthStencilView) {}
    void CommandListDirectX12::OMSetBlendState(RHIBlendState* pState, const float* blendFactor, uint32_t sampleMask) {}
    void CommandListDirectX12::OMSetDepthStencilState(RHIDepthStencilState* pState, uint32_t stencilRef) {}

    // 绘制
    void CommandListDirectX12::Draw(uint32_t vertexCount, uint32_t startVertexLocation)
    {
        m_pCommandList->DrawInstanced(vertexCount, 1, startVertexLocation, 0);
    }

    void CommandListDirectX12::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
    {
        m_pCommandList->DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, 0);
    }

    void CommandListDirectX12::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
    {
        m_pCommandList->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
    }

    void CommandListDirectX12::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
    {
        m_pCommandList->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    void CommandListDirectX12::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
    {
        m_pCommandList->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
    }

    // 清除 - 占位符实现
    void CommandListDirectX12::ClearRenderTargetView(RHIRenderTargetView* pView, const float* colorRGBA) {}
    void CommandListDirectX12::ClearDepthStencilView(RHIDepthStencilView* pView, RHIClearFlags clearFlags, float depth, uint8_t stencil) {}

    // 资源操作
    void CommandListDirectX12::CopyResource(RHIResource* pDstResource, RHIResource* pSrcResource)
    {
        if (pDstResource && pSrcResource)
        {
            BufferDirectX12* dstBuffer = static_cast<BufferDirectX12*>(pDstResource);
            BufferDirectX12* srcBuffer = static_cast<BufferDirectX12*>(pSrcResource);
            m_pCommandList->CopyResource(SafeCast<ID3D12Resource>(dstBuffer->GetResource()),
             SafeCast<ID3D12Resource>(srcBuffer->GetResource()));
        }
    }

    void CommandListDirectX12::CopyBufferRegion(RHIBuffer* pDstBuffer, uint64_t dstOffset, RHIBuffer* pSrcBuffer, uint64_t srcOffset, uint64_t numBytes)
    {
        if (pDstBuffer && pSrcBuffer)
        {
            BufferDirectX12* dxDstBuffer = static_cast<BufferDirectX12*>(pDstBuffer);
            BufferDirectX12* dxSrcBuffer = static_cast<BufferDirectX12*>(pSrcBuffer);
            m_pCommandList->CopyBufferRegion(
                SafeCast<ID3D12Resource>(dxDstBuffer->GetResource()),
                dstOffset,
                SafeCast<ID3D12Resource>(dxSrcBuffer->GetResource()),
                srcOffset,
                numBytes);
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
