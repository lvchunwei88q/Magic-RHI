#include "RHIVulKan.h"
#include "RHIResourceVulKan.h"

namespace RHI
{
    namespace
    {
    }
    
    RHIDescriptorHandle DeviceVulKan::CreateStandardHeapDescriptorView(RHIBuffer* Buffer, DescriptorRangeType Type)
    {
        auto vkBuffer = SafeCast<BufferVulKan>(Buffer);
        if (!vkBuffer || vkBuffer->GetBuffer() == VK_NULL_HANDLE)
        {
            ThrowErrorMessage("Invalid buffer for descriptor creation");
            return RHIDescriptorHandle();
        }

        if (!m_pStandardHeap || m_pStandardHeap->IsFull())
        {
#if RHI_ENABLE_RESOURCE_INFO
            ThrowErrorMessage("Standard descriptor heap is unavailable or full");
#endif
            return RHIDescriptorHandle();
        }

        RHIDescriptorHandle handle = m_pStandardHeap->Allocate();
        if (!handle.IsValid())
        {
#if RHI_ENABLE_RESOURCE_INFO
            ThrowErrorMessage("Failed to allocate descriptor from standard heap");
#endif
            return RHIDescriptorHandle();
        }

        VkBuffer     bufferHandle = vkBuffer->GetBuffer();
        const uint64_t size       = vkBuffer->GetSize();
        const VkDevice* device    = GetDevice();

        switch (Type)
        {
            case DescriptorRangeType::CBV:
            {
                // Maps to D3D12 ConstantBufferView → Vulkan VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
                auto* pCBV = new ConstantBufferViewVulKan(bufferHandle, /*offset*/ 0, /*range*/ size);
                if (!m_pStandardHeap->SetCBVDescriptor(handle, pCBV))
                {
                    m_pStandardHeap->Free(handle);
                    return RHIDescriptorHandle();
                }
                break;
            }

            case DescriptorRangeType::SRV:
            {
                // Maps to D3D12 ShaderResourceView (BUFFER dimension)
                // On Vulkan, buffer SRV uses VK_DESCRIPTOR_TYPE_STORAGE_BUFFER (see WriteSRVToSet)
                auto* pSRV = new ShaderResourceViewVulKan(bufferHandle, /*offset*/ 0, /*range*/ size, device);
                if (!m_pStandardHeap->SetSRVDescriptor(handle, pSRV))
                {
                    m_pStandardHeap->Free(handle);
                    return RHIDescriptorHandle();
                }
                break;
            }

            case DescriptorRangeType::UAV:
            {
                // Maps to D3D12 UnorderedAccessView (BUFFER dimension)
                auto* pUAV = new UnorderedAccessViewVulKan(bufferHandle, /*offset*/ 0, /*range*/ size, device);
                if (!m_pStandardHeap->SetUAVDescriptor(handle, pUAV))
                {
                    m_pStandardHeap->Free(handle);
                    return RHIDescriptorHandle();
                }
                break;
            }

            case DescriptorRangeType::Sampler:
            default:
                ThrowErrorMessage("Unsupported descriptor type for buffer (expected CBV/SRV/UAV)");
                m_pStandardHeap->Free(handle);
                return RHIDescriptorHandle();
        }

        Buffer->SetBindlessHandle(handle);
        return handle;
    }

    RHIDescriptorHandle DeviceVulKan::CreateStandardHeapDescriptorView(RHITexture* Texture, DescriptorRangeType Type)
    {
        if (!m_pStandardHeap || m_pStandardHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pStandardHeap->Allocate();
    }

    RHIDescriptorHandle DeviceVulKan::CreateSamplerHeapDescriptorView(const SamplerStateDesc& desc)
    {
        if (!m_pSamplerHeap || m_pSamplerHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pSamplerHeap->Allocate();
    }

    RHIDescriptorHandle DeviceVulKan::CreateRTVHeapDescriptorView(RHITexture* Texture)
    {
        if (!m_pRTVHeap || m_pRTVHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pRTVHeap->Allocate();
    }

    RHIDescriptorHandle DeviceVulKan::CreateDSVHeapDescriptorView(RHITexture* Texture)
    {
        if (!m_pDSVHeap || m_pDSVHeap->IsFull())
        {
            return RHIDescriptorHandle();
        }

        return m_pDSVHeap->Allocate();
    }
}
