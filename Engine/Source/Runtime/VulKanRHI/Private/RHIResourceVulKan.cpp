#include <Common/Check.h>
#include <cstring>
#include "RHICommandListVulKan.h"
#include "RHIVulKan.h"

namespace RHI
{
    namespace
    {
        // Convert BufferHeapType to Vulkan memory property flags
        VkMemoryPropertyFlags ToVulkanMemoryPropertyFlags(BufferHeapType type)
        {
            switch (type)
            {
            case BufferHeapType::Default:
                return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            case BufferHeapType::Upload:
                return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            case BufferHeapType::Readback:
                return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            default:
                return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            }
        }

        // Convert BufferBindFlag to Vulkan buffer usage flags
        VkBufferUsageFlags ToVulkanBufferUsageFlags(BufferBindFlag bindFlags)
        {
            VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            if (EnumHasAnyFlags(bindFlags, BufferBindFlag::VertexBuffer))
                usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            if (EnumHasAnyFlags(bindFlags, BufferBindFlag::IndexBuffer))
                usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            if (EnumHasAnyFlags(bindFlags, BufferBindFlag::ConstantBuffer))
                usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            if (EnumHasAnyFlags(bindFlags, BufferBindFlag::ShaderResource))
                usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
            if (EnumHasAnyFlags(bindFlags, BufferBindFlag::UnorderedAccess))
                usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

            return usage;
        }

        // Find a suitable Vulkan memory type index
        uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
        {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
            {
                if ((typeFilter & (1u << i)) &&
                    (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    return i;
                }
            }

            ThrowErrorMessage("BufferVulKan: Failed to find suitable memory type");
            return UINT32_MAX;
        }

        // Helper: create a VkBuffer + VkDeviceMemory pair
        bool CreateBufferAndMemory(
            VkDevice device,
            VkPhysicalDevice physicalDevice,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& outBuffer,
            VkDeviceMemory& outMemory)
        {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(device, &bufferInfo, nullptr, &outBuffer) != VK_SUCCESS)
            {
                ThrowErrorMessage("BufferVulKan: Failed to create buffer");
                return false;
            }

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, outBuffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

            if (vkAllocateMemory(device, &allocInfo, nullptr, &outMemory) != VK_SUCCESS)
            {
                ThrowErrorMessage("BufferVulKan: Failed to allocate buffer memory");
                vkDestroyBuffer(device, outBuffer, nullptr);
                outBuffer = VK_NULL_HANDLE;
                return false;
            }

            vkBindBufferMemory(device, outBuffer, outMemory, 0);
            return true;
        }
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
        // Constant buffers need 256-byte alignment on Vulkan (minUniformBufferOffsetAlignment typically)
        auto isConstantBuffer = [](BufferBindFlag flag) -> bool {
            return EnumHasAnyFlags(flag, BufferBindFlag::ConstantBuffer);
        };

        desc.SizeInBytes = isConstantBuffer(desc.BindFlags) ? AlignUp(desc.SizeInBytes, 256) : desc.SizeInBytes;

        VkDevice device = *GetDevice();
        VkPhysicalDevice physicalDevice = *GetPhysicalDevice();

        VkBufferUsageFlags usage = ToVulkanBufferUsageFlags(desc.BindFlags);
        VkMemoryPropertyFlags memProps = ToVulkanMemoryPropertyFlags(desc.HeapType);

        VkBuffer bufferHandle = VK_NULL_HANDLE;
        VkDeviceMemory memoryHandle = VK_NULL_HANDLE;

        if (!CreateBufferAndMemory(device, physicalDevice, desc.SizeInBytes, usage, memProps, bufferHandle, memoryHandle))
        {
            return nullptr;
        }

        auto InitBufferData = [&](void* mappedData) {
            if (mappedData != nullptr) {
                memcpy(mappedData, desc.InitialData, desc.SizeInBytes);
            }else{
                ThrowErrorMessage("BufferVulKan: Failed to map buffer memory");
            }
        };

        std::shared_ptr<BufferVulKan> buffer = std::make_shared<BufferVulKan>(bufferHandle, memoryHandle, desc, GetDevice());

        // Default heap with initial data: need to stage through an upload buffer
        if (desc.HeapType == BufferHeapType::Default && desc.InitialData != nullptr)
        {
            VkBuffer uploadBufferHandle = VK_NULL_HANDLE;
            VkDeviceMemory uploadMemoryHandle = VK_NULL_HANDLE;

            if (!CreateBufferAndMemory(
                device, physicalDevice,
                desc.SizeInBytes,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                uploadBufferHandle, uploadMemoryHandle))
            {
                return nullptr;
            }

            // Map and copy initial data to upload heap
            void* pMapped = nullptr;
            if (vkMapMemory(device, uploadMemoryHandle, 0, desc.SizeInBytes, 0, &pMapped) != VK_SUCCESS)
            {
                ThrowErrorMessage("BufferVulKan: Failed to map upload buffer memory");
                vkFreeMemory(device, uploadMemoryHandle, nullptr);
                vkDestroyBuffer(device, uploadBufferHandle, nullptr);
                return nullptr;
            }
            InitBufferData(pMapped);
            vkUnmapMemory(device, uploadMemoryHandle);

            BufferDesc uploadDesc = desc;
            uploadDesc.HeapType = BufferHeapType::Upload;
            std::shared_ptr<BufferVulKan> uploadBuffer = std::make_shared<BufferVulKan>(
                uploadBufferHandle, uploadMemoryHandle, uploadDesc, GetDevice());

            // Use the copy queue to copy from upload to default heap
            auto cmdAllocator = CreateCommandAllocator(RHICmdType::Copy);
            auto cmdList = CreateCommandList(cmdAllocator);
            GetCommandQueue(RHICmdType::Copy)->BeginFrame();
            cmdList->BeginRecording();

            cmdList->CopyBufferRegion(
                buffer.get(),       // Target: Default heap buffer
                0,                   // Target offset
                uploadBuffer.get(), // Source: Upload heap buffer
                0,                   // Source offset
                desc.SizeInBytes      // Copy size
            );

            cmdList->EndRecording();

            // Execute and wait for the copy to finish
            GetCommandQueue(RHICmdType::Copy)->ExecuteCommandLists({cmdList});
            GetCommandQueue(RHICmdType::Copy)->EndFrame();
            GetCommandQueue(RHICmdType::Copy)->WaitForGPU();

            // Upload buffer will be released automatically when it goes out of scope
            return buffer;
        }

#if RHI_ENABLE_DEBUG_INFO
        else if (desc.HeapType == BufferHeapType::Default && desc.InitialData == nullptr)
            ThrowErrorMessage("Creating Default heap buffer without InitialData");
#endif

        // Upload / Readback heap with initial data: map and copy directly
        if (desc.HeapType != BufferHeapType::Default && desc.InitialData != nullptr)
        {
            void* pMapped = nullptr;
            if (vkMapMemory(device, memoryHandle, 0, desc.SizeInBytes, 0, &pMapped) != VK_SUCCESS)
            {
                ThrowErrorMessage("BufferVulKan: Failed to map buffer memory for initial data");
            }
            else
            {
                InitBufferData(pMapped);
                vkUnmapMemory(device, memoryHandle);
            }
        }

        return buffer;
    }

    void DeviceVulKan::DeleteBuffer(std::shared_ptr<RHIBuffer>& buffer)
    {
        if (!buffer)
            return;

        // Release descriptor heap slot if the buffer has a bindless handle registered
        if (buffer->HasDescriptorHandle())
            m_pStandardHeap->Free(buffer->GetBindlessHandle());

        // Release the buffer resource (BufferVulKan destructor handles vkDestroyBuffer/vkFreeMemory)
        buffer.reset();
    }

    // =====================================================================

    BufferVulKan::~BufferVulKan()
    {
        if (m_MappedData)
        {
            Unmap();
        }

        // Must destroy the buffer FIRST (breaks its binding to device memory),
        // then free the underlying VkDeviceMemory. Otherwise Vulkan validation
        // reports VUID-vkFreeMemory-memory-00677 (memory still in use by buffer).
        if (m_Buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(GetDevice(), m_Buffer, nullptr);
            m_Buffer = VK_NULL_HANDLE;
        }

        if (m_Memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(GetDevice(), m_Memory, nullptr);
            m_Memory = VK_NULL_HANDLE;
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
        // Must destroy the image FIRST (breaks its binding to device memory),
        // then free the underlying VkDeviceMemory. Same VUID-vkFreeMemory-memory-00677.
        if (m_Image != VK_NULL_HANDLE)
        {
            vkDestroyImage(GetDevice(), m_Image, nullptr);
            m_Image = VK_NULL_HANDLE;
        }

        if (m_Memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(GetDevice(), m_Memory, nullptr);
            m_Memory = VK_NULL_HANDLE;
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
