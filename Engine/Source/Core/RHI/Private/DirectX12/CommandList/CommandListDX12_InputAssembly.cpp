#include "DirectX12/RHIResourceDirectX12.h"
#include "DirectXHelper.h"

namespace RHI
{
    namespace
    {
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
    }

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
}
