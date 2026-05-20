#include "DirectX11/RHIDirectX11.h"
#include "DirectX11/RHIResourceDirectX11.h"

namespace RHI
{
    namespace
    {
        // D3D11 转换
        D3D11_USAGE ToD3D11Usage(BufferHeapType type)
        {
            switch (type)
            {
            case BufferHeapType::Default:  return D3D11_USAGE_DEFAULT;
            case BufferHeapType::Upload:   return D3D11_USAGE_DYNAMIC;
            case BufferHeapType::Readback: return D3D11_USAGE_STAGING;
            default:                       return D3D11_USAGE_DEFAULT;
            }
        }

        UINT ToD3D11CPUAccessFlags(BufferHeapType type)
        {
            switch (type)
            {
            case BufferHeapType::Default:  return 0;
            case BufferHeapType::Upload:   return D3D11_CPU_ACCESS_WRITE;
            case BufferHeapType::Readback: return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
            default:                       return 0;
            }
        }

        // 将 RHI 绑定标志转换为 D3D11 绑定标志
        UINT ToD3D11BindFlags(BufferBindFlag flags)
        {
            UINT d3dFlags = 0;
            
            d3dFlags |= EnumHasAnyFlags(flags, BufferBindFlag::VertexBuffer)   ? D3D11_BIND_VERTEX_BUFFER   : 0;
            d3dFlags |= EnumHasAnyFlags(flags, BufferBindFlag::IndexBuffer)    ? D3D11_BIND_INDEX_BUFFER    : 0;
            d3dFlags |= EnumHasAnyFlags(flags, BufferBindFlag::ConstantBuffer) ? D3D11_BIND_CONSTANT_BUFFER : 0;
            d3dFlags |= EnumHasAnyFlags(flags, BufferBindFlag::ShaderResource) ? D3D11_BIND_SHADER_RESOURCE : 0;
            d3dFlags |= EnumHasAnyFlags(flags, BufferBindFlag::UnorderedAccess)? D3D11_BIND_UNORDERED_ACCESS: 0;
            d3dFlags |= EnumHasAnyFlags(flags, BufferBindFlag::RenderTarget)   ? D3D11_BIND_RENDER_TARGET   : 0;
            d3dFlags |= EnumHasAnyFlags(flags, BufferBindFlag::DepthStencil)   ? D3D11_BIND_DEPTH_STENCIL   : 0;
            d3dFlags |= EnumHasAnyFlags(flags, BufferBindFlag::StreamOutput)   ? D3D11_BIND_STREAM_OUTPUT   : 0;
            
            return d3dFlags;
        }

        D3D11_FILTER ConvertFilter(SamplerFilter filter)
        {
            switch (filter)
            {
            case SamplerFilter::Point:
                return D3D11_FILTER_MIN_MAG_MIP_POINT;
            case SamplerFilter::Bilinear:
                return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            case SamplerFilter::Trilinear:
                return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            case SamplerFilter::Anisotropic:
                return D3D11_FILTER_ANISOTROPIC;
            default:
                return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            }
        }

        D3D11_TEXTURE_ADDRESS_MODE ConvertAddressMode(SamplerAddressMode mode)
        {
            switch (mode)
            {
            case SamplerAddressMode::Wrap:
                return D3D11_TEXTURE_ADDRESS_WRAP;
            case SamplerAddressMode::Mirror:
                return D3D11_TEXTURE_ADDRESS_MIRROR;
            case SamplerAddressMode::Clamp:
                return D3D11_TEXTURE_ADDRESS_CLAMP;
            case SamplerAddressMode::Border:
                return D3D11_TEXTURE_ADDRESS_BORDER;
            case SamplerAddressMode::MirrorOnce:
                return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
            default:
                return D3D11_TEXTURE_ADDRESS_WRAP;
            }
        }

        D3D11_COMPARISON_FUNC ConvertComparisonFunc(SamplerComparisonFunc func)
        {
            switch (func)
            {
            case SamplerComparisonFunc::Never:
                return D3D11_COMPARISON_NEVER;
            case SamplerComparisonFunc::Less:
                return D3D11_COMPARISON_LESS;
            case SamplerComparisonFunc::Equal:
                return D3D11_COMPARISON_EQUAL;
            case SamplerComparisonFunc::LessEqual:
                return D3D11_COMPARISON_LESS_EQUAL;
            case SamplerComparisonFunc::Greater:
                return D3D11_COMPARISON_GREATER;
            case SamplerComparisonFunc::NotEqual:
                return D3D11_COMPARISON_NOT_EQUAL;
            case SamplerComparisonFunc::GreaterEqual:
                return D3D11_COMPARISON_GREATER_EQUAL;
            case SamplerComparisonFunc::Always:
                return D3D11_COMPARISON_ALWAYS;
            default:
                return D3D11_COMPARISON_NEVER;
            }
        }

        D3D11_PRIMITIVE_TOPOLOGY ConvertPrimitiveTopology(RHIPrimitiveTopology topology)
        {
            switch (topology)
            {
            case RHIPrimitiveTopology::PointList: return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            case RHIPrimitiveTopology::LineList: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            case RHIPrimitiveTopology::LineStrip: return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case RHIPrimitiveTopology::TriangleList: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case RHIPrimitiveTopology::TriangleStrip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            case RHIPrimitiveTopology::LineListAdj: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
            case RHIPrimitiveTopology::LineStripAdj: return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
            case RHIPrimitiveTopology::TriangleListAdj: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
            case RHIPrimitiveTopology::TriangleStripAdj: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
            default: return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
            }
        }

        D3D11_PRIMITIVE_TOPOLOGY ConvertControlPointPatchList(uint32_t controlPointCount)
        {
            switch (controlPointCount)
            {
            case 1: return D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
            case 2: return D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
            case 3: return D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
            case 4: return D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
            case 5: return D3D11_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST;
            case 6: return D3D11_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST;
            case 7: return D3D11_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST;
            case 8: return D3D11_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST;
            case 9: return D3D11_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST;
            case 10: return D3D11_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST;
            case 11: return D3D11_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST;
            case 12: return D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST;
            case 13: return D3D11_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST;
            case 14: return D3D11_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST;
            case 15: return D3D11_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST;
            case 16: return D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
            case 17: return D3D11_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST;
            case 18: return D3D11_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST;
            case 19: return D3D11_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST;
            case 20: return D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
            case 21: return D3D11_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST;
            case 22: return D3D11_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST;
            case 23: return D3D11_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST;
            case 24: return D3D11_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST;
            case 25: return D3D11_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
            case 26: return D3D11_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST;
            case 27: return D3D11_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST;
            case 28: return D3D11_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST;
            case 29: return D3D11_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST;
            case 30: return D3D11_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST;
            case 31: return D3D11_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST;
            case 32: return D3D11_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST;
            default: return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
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
    }

    std::shared_ptr<RHISamplerState> RHIDirectX11::CreateSamplerState(const SamplerStateDesc& desc)
    {
        D3D11_SAMPLER_DESC samplerDesc = {};
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

        ComPtr<ID3D11SamplerState> pSamplerState;
        ThrowIfFailed(m_pDevice->CreateSamplerState(&samplerDesc, pSamplerState.GetAddressOf()));

        return std::make_shared<SamplerStateDirectX11>(pSamplerState.Get());
    }

    void RHIDirectX11::DeleteSamplerState(std::shared_ptr<RHI::RHISamplerState>& samplerState)
    {
        samplerState.reset();
    }

    std::shared_ptr<RHIBuffer> RHIDirectX11::CreateBuffer(const BufferDesc& desc)
    {
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = static_cast<UINT>(desc.SizeInBytes);
        bufferDesc.MiscFlags = 0;
        bufferDesc.BindFlags = ToD3D11BindFlags(desc.BindFlags);
        bufferDesc.StructureByteStride = desc.Stride;
        bufferDesc.Usage = ToD3D11Usage(desc.HeapType);
        bufferDesc.CPUAccessFlags = ToD3D11CPUAccessFlags(desc.HeapType);

        if(bufferDesc.BindFlags == 0)
        {
#if RHI_ENABLE_RESOURCE_INFO
        ThrowIfFailed("Please set the type for your buffer, otherwise it will fail to be created."); // 直接抛出异常，不支持动态创建
#else
        bufferDesc.BindFlags |= D3D11_BIND_CONSTANT_BUFFER; // 选择一个常用的
#endif
        }

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = desc.InitialData;

        ComPtr<ID3D11Buffer> pBuffer;
        if (desc.InitialData != nullptr)
        {
            ThrowIfFailed(m_pDevice->CreateBuffer(&bufferDesc, &initData, pBuffer.GetAddressOf()));
        }
#if RHI_ENABLE_RESOURCE_INFO
        else if(desc.InitialData == nullptr && desc.HeapType == RHI::BufferHeapType::Default)
            ThrowIfFailed("Creating D3D11_HEAP_TYPE_DEFAULT requires providing heap data");
#endif
        else
        {
            ThrowIfFailed(m_pDevice->CreateBuffer(&bufferDesc, nullptr, pBuffer.GetAddressOf()));
        }

        auto buffer = std::make_shared<BufferDirectX11>(pBuffer.Get(), desc);
        buffer->SetDeviceContext(m_pDeviceContext.Get());

        return buffer;
    }

    void RHIDirectX11::DeleteBuffer(std::shared_ptr<RHI::RHIBuffer>& buffer)
    {
        buffer.reset();
    }

    // CommandListDirectX11 实现
    void CommandListDirectX11::IASetPrimitiveTopology(RHIPrimitiveTopology topology, uint32_t controlPointCount)
    {
        if (topology == RHIPrimitiveTopology::ControlPointPatchList)
        {
            m_pDeviceContext->IASetPrimitiveTopology(ConvertControlPointPatchList(controlPointCount));
        }
        else
        {
            m_pDeviceContext->IASetPrimitiveTopology(ConvertPrimitiveTopology(topology));
        }
    }

    void CommandListDirectX11::IASetVertexBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers, const uint64_t* pOffsets)
    {
        std::vector<ID3D11Buffer*> buffers;
        std::vector<UINT> strides;
        std::vector<UINT> offsets;

        buffers.reserve(numBuffers);
        strides.reserve(numBuffers);
        offsets.reserve(numBuffers);

        for (uint32_t i = 0; i < numBuffers; ++i)
        {
            BufferDirectX11* dxBuffer = static_cast<BufferDirectX11*>(ppBuffers[i]);
            buffers.push_back(SafeCast<ID3D11Buffer>(dxBuffer->GetResource()));
            strides.push_back(dxBuffer->GetStride());
            offsets.push_back(pOffsets ? (UINT)pOffsets[i] : 0);
        }

        m_pDeviceContext->IASetVertexBuffers(startSlot, numBuffers, buffers.data(), strides.data(), offsets.data());
    }

    void CommandListDirectX11::IASetIndexBuffer(RHIBuffer* pIndexBuffer, RHIIndexFormat format, uint64_t offset)
    {
        if (pIndexBuffer)
        {
            BufferDirectX11* dxBuffer = static_cast<BufferDirectX11*>(pIndexBuffer);
            m_pDeviceContext->IASetIndexBuffer(SafeCast<ID3D11Buffer>(dxBuffer->GetResource()),
             ConvertIndexFormat(format), (UINT)offset);
        }
        else
        {
            m_pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
        }
    }

    // 着色器 - 占位符实现
    void CommandListDirectX11::VSSetShader(RHIVertexShader* pShader) {}
    void CommandListDirectX11::PSSetShader(RHIPixelShader* pShader) {}
    void CommandListDirectX11::GSSetShader(RHIGeometryShader* pShader) {}
    void CommandListDirectX11::HSSetShader(RHIHullShader* pShader) {}
    void CommandListDirectX11::DSSetShader(RHIDomainShader* pShader) {}
    void CommandListDirectX11::CSSetShader(RHIComputeShader* pShader) {}

    // 着色器资源 - 占位符实现
    void CommandListDirectX11::VSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) {}
    void CommandListDirectX11::PSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) {}
    void CommandListDirectX11::GSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) {}
    void CommandListDirectX11::HSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) {}
    void CommandListDirectX11::DSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) {}
    void CommandListDirectX11::CSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) {}

    void CommandListDirectX11::VSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) {}
    void CommandListDirectX11::PSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) {}
    void CommandListDirectX11::GSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) {}
    void CommandListDirectX11::HSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) {}
    void CommandListDirectX11::DSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) {}
    void CommandListDirectX11::CSSetShaderResources(uint32_t startSlot, uint32_t numViews, RHIShaderResourceView* const* ppViews) {}

    void CommandListDirectX11::VSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) {}
    void CommandListDirectX11::PSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) {}
    void CommandListDirectX11::GSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) {}
    void CommandListDirectX11::HSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) {}
    void CommandListDirectX11::DSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) {}
    void CommandListDirectX11::CSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) {}

    // 光栅器
    void CommandListDirectX11::RSSetViewports(uint32_t numViewports, const RHIViewport* pViewports)
    {
        std::vector<D3D11_VIEWPORT> d3dViewports;
        d3dViewports.reserve(numViewports);
        for (uint32_t i = 0; i < numViewports; ++i)
        {
            D3D11_VIEWPORT vp = {};
            vp.TopLeftX = pViewports[i].topLeftX;
            vp.TopLeftY = pViewports[i].topLeftY;
            vp.Width = pViewports[i].width;
            vp.Height = pViewports[i].height;
            vp.MinDepth = pViewports[i].minDepth;
            vp.MaxDepth = pViewports[i].maxDepth;
            d3dViewports.push_back(vp);
        }
        m_pDeviceContext->RSSetViewports(numViewports, d3dViewports.data());
    }

    void CommandListDirectX11::RSSetScissorRects(uint32_t numRects, const RHIRect* pRects)
    {
        std::vector<D3D11_RECT> d3dRects;
        d3dRects.reserve(numRects);
        for (uint32_t i = 0; i < numRects; ++i)
        {
            D3D11_RECT rect = {};
            rect.left = (LONG)pRects[i].left;
            rect.top = (LONG)pRects[i].top;
            rect.right = (LONG)pRects[i].right;
            rect.bottom = (LONG)pRects[i].bottom;
            d3dRects.push_back(rect);
        }
        m_pDeviceContext->RSSetScissorRects(numRects, d3dRects.data());
    }

    void CommandListDirectX11::RSSetState(RHIRasterizerState* pState) {}

    // 输出合并器 - 占位符实现
    void CommandListDirectX11::OMSetRenderTargets(uint32_t numRenderTargets, RHIRenderTargetView* const* ppViews, RHIDepthStencilView* pDepthStencilView) {}
    void CommandListDirectX11::OMSetBlendState(RHIBlendState* pState, const float* blendFactor, uint32_t sampleMask) {}
    void CommandListDirectX11::OMSetDepthStencilState(RHIDepthStencilState* pState, uint32_t stencilRef) {}

    // 绘制
    void CommandListDirectX11::Draw(uint32_t vertexCount, uint32_t startVertexLocation)
    {
        m_pDeviceContext->Draw(vertexCount, startVertexLocation);
    }

    void CommandListDirectX11::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
    {
        m_pDeviceContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void CommandListDirectX11::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
    {
        m_pDeviceContext->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
    }

    void CommandListDirectX11::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
    {
        m_pDeviceContext->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    void CommandListDirectX11::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
    {
        m_pDeviceContext->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
    }

    // 清除 - 占位符实现
    void CommandListDirectX11::ClearRenderTargetView(RHIRenderTargetView* pView, const float* colorRGBA) {}
    void CommandListDirectX11::ClearDepthStencilView(RHIDepthStencilView* pView, RHIClearFlags clearFlags, float depth, uint8_t stencil) {}

    // 资源操作
    void CommandListDirectX11::CopyResource(RHIResource* pDstResource, RHIResource* pSrcResource)
    {
        if (pDstResource && pSrcResource)
        {
            BufferDirectX11* dstBuffer = static_cast<BufferDirectX11*>(pDstResource);
            BufferDirectX11* srcBuffer = static_cast<BufferDirectX11*>(pSrcResource);
            m_pDeviceContext->CopyResource(SafeCast<ID3D11Buffer>(dstBuffer->GetResource()),
             SafeCast<ID3D11Buffer>(srcBuffer->GetResource()));
        }
    }

    void CommandListDirectX11::CopyBufferRegion(RHIBuffer* pDstBuffer, uint64_t dstOffset, RHIBuffer* pSrcBuffer, uint64_t srcOffset, uint64_t numBytes)
    {
        if (pDstBuffer && pSrcBuffer)
        {
            BufferDirectX11* dxDstBuffer = static_cast<BufferDirectX11*>(pDstBuffer);
            BufferDirectX11* dxSrcBuffer = static_cast<BufferDirectX11*>(pSrcBuffer);
            
            D3D11_BOX srcBox;
            srcBox.left = (UINT)srcOffset;
            srcBox.top = 0;
            srcBox.front = 0;
            srcBox.right = (UINT)srcOffset + (UINT)numBytes;
            srcBox.bottom = 1;
            srcBox.back = 1;
            
            m_pDeviceContext->CopySubresourceRegion(
                SafeCast<ID3D11Buffer>(dxDstBuffer->GetResource()), 0, (UINT)dstOffset, 0, 0,
                SafeCast<ID3D11Buffer>(dxSrcBuffer->GetResource()), 0, &srcBox);
        }
    }

    void CommandListDirectX11::ResourceBarrier(uint32_t numBarriers, const BarrierDesc* pBarriers)
    {
        // NOT
    }
}
