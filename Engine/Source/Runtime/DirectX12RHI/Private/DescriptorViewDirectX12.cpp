#include "RHIDirectX12.h"
#include "RHIResourceDirectX12.h"

namespace RHI
{
    RHIDescriptorHandle RHIDirectX12::CreateStandardHeapDescriptorView(RHIBuffer* Buffer,DescriptorRangeType Type)
    {
        auto dx12Buffer = SafeCast<BufferDirectX12>(Buffer);
        if (!dx12Buffer || !dx12Buffer->GetResource())
        {
            ThrowErrorMessage("Invalid buffer for descriptor creation");
            return RHIDescriptorHandle();
        }
        
        // 使用标准堆创建描述符
        RHIDescriptorHeapType heapType = RHIDescriptorHeapType::Standard;
        RHIDescriptorHandle handle = m_pStandardHeap->Allocate();
        if (!handle.IsValid())
        {
#if RHI_ENABLE_RESOURCE_INFO
            ThrowErrorMessage("Failed to allocate descriptor");
#endif
            return RHIDescriptorHandle();
        }
    
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_pStandardHeap->GetCPUHandle(handle.GetIndex());
        
        // 根据类型创建描述符
        /* 唯一特殊的描述符类型是 CBV，因为API必须要以256字节对齐，所以你会看到CreateBuffer会针对他们进行特殊处理，
        * 当然SRV则需要设置Shader4ComponentMapping，表示在Shader中如何读取这些值一般来说默认是D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING，
        * 而UAV则需要从Default堆中创建（CBV也是）如果你需要一些示例请你查看https://github.com/lvchunwei88q/DirectXRHI 仓库中的Main.cpp哪里有关于使用RHI的样板代码
        **/
        switch (Type)
        {
        case DescriptorRangeType::CBV:
        {
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
            cbvDesc.BufferLocation = dx12Buffer->GetResource()->GetGPUVirtualAddress();
            cbvDesc.SizeInBytes = (UINT)dx12Buffer->GetSize();
            m_pDevice->CreateConstantBufferView(&cbvDesc, cpuHandle);
            break;
        }
        
        case DescriptorRangeType::SRV:
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Buffer.FirstElement = 0;
            srvDesc.Buffer.NumElements = (UINT)(dx12Buffer->GetSize() / dx12Buffer->GetStride());
            srvDesc.Buffer.StructureByteStride = dx12Buffer->GetStride();
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; 
            m_pDevice->CreateShaderResourceView(dx12Buffer->GetResource(), &srvDesc, cpuHandle);
            break;
        }
        
        case DescriptorRangeType::UAV:
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_UNKNOWN;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.FirstElement = 0;
            uavDesc.Buffer.NumElements = (UINT)(dx12Buffer->GetSize() / dx12Buffer->GetStride());
            uavDesc.Buffer.StructureByteStride = dx12Buffer->GetStride();
            uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
            m_pDevice->CreateUnorderedAccessView(dx12Buffer->GetResource(), nullptr, &uavDesc, cpuHandle);
            break;
        }
        
        default:
            ThrowErrorMessage("Unsupported descriptor type for buffer");
            return RHIDescriptorHandle();
        }
        
        Buffer->SetBindlessHandle(handle);
        return handle;
    }

    RHIDescriptorHandle RHIDirectX12::CreateStandardHeapDescriptorView(RHITexture* Texture,DescriptorRangeType Type)
    {
        if (!m_pSamplerHeap || m_pSamplerHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pSamplerHeap->Allocate();
    }

    RHIDescriptorHandle RHIDirectX12::CreateSamplerHeapDescriptorView(const SamplerStateDesc& /*desc*/)
    {
        if (!m_pSamplerHeap || m_pSamplerHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pSamplerHeap->Allocate();
    }

    RHIDescriptorHandle RHIDirectX12::CreateRTVHeapDescriptorView(RHIRenderTargetView* /*InView*/)
    {
        if (!m_pRTVHeap || m_pRTVHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pRTVHeap->Allocate();
    }

    RHIDescriptorHandle RHIDirectX12::CreateDSVHeapDescriptorView(RHIDepthStencilView* /*InView*/)
    {
        if (!m_pDSVHeap || m_pDSVHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pDSVHeap->Allocate();
    }
}
