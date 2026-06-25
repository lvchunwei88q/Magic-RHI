#include <Common/Check.h>
#include "RHID3D11.h"
#include "RHIResourceD3D11.h"

namespace RHI
{
    RHIDescriptorHandle RHID3D11::CreateStandardHeapDescriptorView(RHIBuffer* Buffer,DescriptorRangeType Type)
    {
        if (!m_pStandardHeap || m_pStandardHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        auto dx11Buffer = SafeCast<BufferD3D11>(Buffer);
        RHIDescriptorHandle handle = m_pStandardHeap->Allocate();
        if (!handle.IsValid())
        {
#if RHI_ENABLE_RESOURCE_INFO
            ThrowErrorMessage("Failed to allocate descriptor");
#endif
            return RHIDescriptorHandle();
        }

        switch (Type)
        {
            case DescriptorRangeType::CBV:
            {
                // DX11 中常量缓冲区不需要单独创建 View
                if (!dx11Buffer)
                    ThrowErrorMessage("Invalid buffer for CBV");
                
                ConstantBufferViewD3D11* pDX11CBV = new ConstantBufferViewD3D11(dx11Buffer->GetResource());
                m_pStandardHeap->SetDescriptor(handle, pDX11CBV);
                break;
            }

            case DescriptorRangeType::SRV:
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = DXGI_FORMAT_UNKNOWN;               // 使用 Buffer 格式，不指定具体格式
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
                srvDesc.Buffer.FirstElement = 0;
                srvDesc.Buffer.NumElements = (UINT)(dx11Buffer->GetSize() / dx11Buffer->GetStride());
                
                ID3D11ShaderResourceView* pSRV = nullptr;
                HRESULT hr = m_pDevice->CreateShaderResourceView(dx11Buffer->GetResource(), &srvDesc, &pSRV);
                if (FAILED(hr))
                    ThrowIfFailed(hr);
                
                ShaderResourceViewD3D11* pDX11SRV = new ShaderResourceViewD3D11(pSRV);
                m_pStandardHeap->SetDescriptor(handle, pDX11SRV);
                break;
            }

            case DescriptorRangeType::UAV:
            {
                D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.Format = DXGI_FORMAT_UNKNOWN;
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
                uavDesc.Buffer.FirstElement = 0;
                uavDesc.Buffer.NumElements = (UINT)(dx11Buffer->GetSize() / dx11Buffer->GetStride());
                
                ID3D11UnorderedAccessView* pUAV = nullptr;
                HRESULT hr = m_pDevice->CreateUnorderedAccessView(dx11Buffer->GetResource(), &uavDesc, &pUAV);
                if (FAILED(hr))
                    ThrowIfFailed(hr);

                UnorderedAccessViewD3D11* pDX11UAV = new UnorderedAccessViewD3D11(pUAV);
                m_pStandardHeap->SetDescriptor(handle, pDX11UAV);
                break;
            }
            default:
                ThrowErrorMessage("Unsupported descriptor type for buffer");
                return RHIDescriptorHandle();
        }
        
        dx11Buffer->SetBindlessHandle(handle);
        return handle;
    }

    RHIDescriptorHandle RHID3D11::CreateStandardHeapDescriptorView(RHITexture* Texture,DescriptorRangeType Type)
    {
        if (!m_pStandardHeap || m_pStandardHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pStandardHeap->Allocate();
    }

    RHIDescriptorHandle RHID3D11::CreateSamplerHeapDescriptorView(const SamplerStateDesc& /*desc*/)
    {
        if (!m_pSamplerHeap || m_pSamplerHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pSamplerHeap->Allocate();
    }

    RHIDescriptorHandle RHID3D11::CreateRTVHeapDescriptorView(RHIRenderTargetView* /*InView*/)
    {
        if (!m_pRTVHeap || m_pRTVHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pRTVHeap->Allocate();
    }

    RHIDescriptorHandle RHID3D11::CreateDSVHeapDescriptorView(RHIDepthStencilView* /*InView*/)
    {
        if (!m_pDSVHeap || m_pDSVHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pDSVHeap->Allocate();
    }
}
