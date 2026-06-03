#include "RHICommandListDirectX11.h"

namespace RHI
{
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

        CommandAllocatorDirectX11* pAllocator = SafeCast<CommandAllocatorDirectX11>(m_pAllocator);
        pAllocator->GetDeviceContext()->RSSetViewports(numViewports, d3dViewports.data());
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

        CommandAllocatorDirectX11* pAllocator = SafeCast<CommandAllocatorDirectX11>(m_pAllocator);
        pAllocator->GetDeviceContext()->RSSetScissorRects(numRects, d3dRects.data());
    }
}
