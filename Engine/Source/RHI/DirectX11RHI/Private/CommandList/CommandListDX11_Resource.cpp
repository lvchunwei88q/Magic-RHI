#include "RHIResourceDirectX11.h"

namespace RHI
{
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
}
