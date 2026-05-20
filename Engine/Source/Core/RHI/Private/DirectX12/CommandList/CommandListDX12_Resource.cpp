#include "DirectX12/RHIResourceDirectX12.h"

namespace RHI
{
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
}
