#include <Common/Check.h>
#include "RHIResourceVulKan.h"
#include "RHIVulKan.h"

namespace RHI
{
    namespace
    {
    }

    // =====================================================================
    // device methods
    std::shared_ptr<RHISamplerState> DeviceVulKan::CreateSamplerState(const SamplerStateDesc& desc)
    {
        return nullptr;
    }

    void DeviceVulKan::DeleteSamplerState(std::shared_ptr<RHISamplerState>& samplerState)
    {
    }

    std::shared_ptr<RHIBuffer> DeviceVulKan::CreateBuffer(BufferDesc& desc)
    {
        return nullptr;
    }

    void DeviceVulKan::DeleteBuffer(std::shared_ptr<RHIBuffer>& buffer)
    {
    }

    // =====================================================================

    BufferVulKan::~BufferVulKan()
    {
        if (m_MappedData)
        {
            Unmap();
        }

        if (m_Memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(GetDevice(), m_Memory, nullptr);
            m_Memory = VK_NULL_HANDLE;
        }

        if (m_Buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(GetDevice(), m_Buffer, nullptr);
            m_Buffer = VK_NULL_HANDLE;
        }
    }

    void* BufferVulKan::Map()
    {
        if (GetHeapType() != BufferHeapType::Default)
        {
            VkResult result = vkMapMemory(GetDevice(), m_Memory, 0, GetSize(), 0, &m_MappedData);
            if (result != VK_SUCCESS)
            {
                m_MappedData = nullptr;
                ThrowErrorMessage("BufferVulKan: Failed to map buffer memory");
                return nullptr;
            }
            return m_MappedData;
        }

        ThrowErrorMessage("BufferVulKan: Cannot map default buffer");
        return nullptr;
    }

    void BufferVulKan::Unmap()
    {
        if (m_MappedData && GetHeapType() != BufferHeapType::Default)
        {
            vkUnmapMemory(GetDevice(), m_Memory);
            m_MappedData = nullptr;
        }
    }

    uint64_t BufferVulKan::GetGPUVirtualAddress() const
    {
        return 0;
    }

    TextureVulKan::~TextureVulKan()
    {
        if (m_Memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(GetDevice(), m_Memory, nullptr);
            m_Memory = VK_NULL_HANDLE;
        }

        if (m_Image != VK_NULL_HANDLE)
        {
            vkDestroyImage(GetDevice(), m_Image, nullptr);
            m_Image = VK_NULL_HANDLE;
        }
    }

    uint64_t TextureVulKan::GetSize() const
    {
        return 0;
    }

    uint32_t TextureVulKan::GetWidth() const
    {
        return GetDesc().Width;
    }

    uint32_t TextureVulKan::GetHeight() const
    {
        return GetDesc().Height;
    }

    uint64_t ConstantBufferViewVulKan::GetGPUVirtualAddress() const
    {
        return m_Offset;
    }

    ShaderResourceViewVulKan::~ShaderResourceViewVulKan()
    {
        if (m_ImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(GetDevice(), m_ImageView, nullptr);
            m_ImageView = VK_NULL_HANDLE;
        }
    }

    uint64_t ShaderResourceViewVulKan::GetGPUVirtualAddress() const
    {
        if (m_Buffer != VK_NULL_HANDLE)
        {
            return m_Offset;
        }
        return (uint64_t)m_ImageView;
    }

    UnorderedAccessViewVulKan::~UnorderedAccessViewVulKan()
    {
        if (m_ImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(GetDevice(), m_ImageView, nullptr);
            m_ImageView = VK_NULL_HANDLE;
        }
    }

    uint64_t UnorderedAccessViewVulKan::GetGPUVirtualAddress() const
    {
        if (m_Buffer != VK_NULL_HANDLE)
        {
            return m_Offset;
        }
        return (uint64_t)m_ImageView;
    }

    RenderTargetViewVulKan::~RenderTargetViewVulKan()
    {
        vkDestroyImageView(GetDevice(), m_ImageView, nullptr);
    }

    DepthStencilViewVulKan::~DepthStencilViewVulKan()
    {
        vkDestroyImageView(GetDevice(), m_ImageView, nullptr);
    }

    SamplerStateVulKan::~SamplerStateVulKan()
    {
        vkDestroySampler(GetDevice(), m_Sampler, nullptr);
    }
}
