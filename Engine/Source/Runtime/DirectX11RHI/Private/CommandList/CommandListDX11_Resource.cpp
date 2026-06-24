#include "RHICommandListD3D11.h"

namespace RHI
{
    void CommandListD3D11::CopyResource(RHIResource* pDstResource, RHIResource* pSrcResource)
    {
        if (pDstResource && pSrcResource)
        {
            BufferD3D11* dstBuffer = static_cast<BufferD3D11*>(pDstResource);
            BufferD3D11* srcBuffer = static_cast<BufferD3D11*>(pSrcResource);

            CommandAllocatorD3D11* pAllocator = SafeCast<CommandAllocatorD3D11>(m_pAllocator);
            pAllocator->GetDeviceContext()->CopyResource(SafeCast<ID3D11Buffer>(dstBuffer->GetResource()),
             SafeCast<ID3D11Buffer>(srcBuffer->GetResource()));
        }
    }

    void CommandListD3D11::CopyBufferRegion(RHIBuffer* pDstBuffer, uint64_t dstOffset, RHIBuffer* pSrcBuffer, uint64_t srcOffset, uint64_t numBytes)
    {
        if (pDstBuffer && pSrcBuffer)
        {
            BufferD3D11* dxDstBuffer = static_cast<BufferD3D11*>(pDstBuffer);
            BufferD3D11* dxSrcBuffer = static_cast<BufferD3D11*>(pSrcBuffer);
            
            D3D11_BOX srcBox;
            srcBox.left = (UINT)srcOffset;
            srcBox.top = 0;
            srcBox.front = 0;
            srcBox.right = (UINT)srcOffset + (UINT)numBytes;
            srcBox.bottom = 1;
            srcBox.back = 1;
            
            CommandAllocatorD3D11* pAllocator = SafeCast<CommandAllocatorD3D11>(m_pAllocator);
            pAllocator->GetDeviceContext()->CopySubresourceRegion(
                SafeCast<ID3D11Buffer>(dxDstBuffer->GetResource()), 0, (UINT)dstOffset, 0, 0,
                SafeCast<ID3D11Buffer>(dxSrcBuffer->GetResource()), 0, &srcBox);
        }
    }
}
