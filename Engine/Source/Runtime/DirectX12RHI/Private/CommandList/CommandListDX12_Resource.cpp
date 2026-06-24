#include "RHICommandListD3D12.h"

namespace RHI
{
    void CommandListD3D12::CopyResource(RHIResource* pDstResource, RHIResource* pSrcResource)
    {
        if (pDstResource && pSrcResource)
        {
            BufferD3D12* dstBuffer = static_cast<BufferD3D12*>(pDstResource);
            BufferD3D12* srcBuffer = static_cast<BufferD3D12*>(pSrcResource);
            m_pCommandList->CopyResource(SafeCast<ID3D12Resource>(dstBuffer->GetResource()),
             SafeCast<ID3D12Resource>(srcBuffer->GetResource()));
        }
    }

    void CommandListD3D12::CopyBufferRegion(RHIBuffer* pDstBuffer, uint64_t dstOffset, RHIBuffer* pSrcBuffer, uint64_t srcOffset, uint64_t numBytes)
    {
        if (pDstBuffer && pSrcBuffer)
        {
            BufferD3D12* dxDstBuffer = static_cast<BufferD3D12*>(pDstBuffer);
            BufferD3D12* dxSrcBuffer = static_cast<BufferD3D12*>(pSrcBuffer);
            m_pCommandList->CopyBufferRegion(
                SafeCast<ID3D12Resource>(dxDstBuffer->GetResource()),
                dstOffset,
                SafeCast<ID3D12Resource>(dxSrcBuffer->GetResource()),
                srcOffset,
                numBytes);
        }
    }
}
