#include <Common/Check.h>
#include "DescriptorHeapVulKan.h"

namespace RHI
{
    DescriptorHeapVulKan::DescriptorHeapVulKan(
        VkDevice* device,
        RHIDescriptorHeapType type,
        uint32_t capacity,
        const std::vector<VkDescriptorPoolSize>& poolSizes
    )
        : RHIDescriptorHeap(type, capacity)
        , m_Device(device)
        , m_DescriptorPool(VK_NULL_HANDLE)
        , m_DescriptorSetCapacity(capacity)
    {
        if (poolSizes.empty())
        {
            ThrowErrorMessage("DescriptorHeapVulKan: poolSizes cannot be empty");
            return;
        }

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = capacity;

        VkResult result = vkCreateDescriptorPool(GetDevice(), &poolInfo, nullptr, &m_DescriptorPool);
        if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("DescriptorHeapVulKan: Failed to create descriptor pool");
            return;
        }

        m_FreeList.reserve(capacity);
        m_Descriptors.reserve(capacity);
        m_DescriptorSets.reserve(capacity);
        m_DescriptorTypes.reserve(capacity);

        for (uint32_t i = 0; i < capacity; ++i)
        {
            m_FreeList.push_back(i);
            m_Descriptors.push_back({});
            m_DescriptorSets.push_back(VK_NULL_HANDLE);
            m_DescriptorTypes.push_back(VK_DESCRIPTOR_TYPE_MAX_ENUM);
        }
    }

    DescriptorHeapVulKan::~DescriptorHeapVulKan()
    {   
        if (m_DescriptorPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(GetDevice(), m_DescriptorPool, nullptr);
            m_DescriptorPool = VK_NULL_HANDLE;
        }
    }

    const VkDevice DescriptorHeapVulKan::GetDevice() const
    {
        if (!m_Device)
        {
            ThrowErrorMessage("DescriptorHeapVulKan: Device is null");
            return VK_NULL_HANDLE;
        }
        return *m_Device;
    }

    RHIDescriptorHandle DescriptorHeapVulKan::Allocate()
    {
        if (IsFull())
        {
            ThrowErrorMessage("DescriptorHeapVulKan: Heap is full");
            return RHIDescriptorHandle();
        }

        uint32_t index = m_FreeList.back();
        m_FreeList.pop_back();

        CurrentIndex = Capacity - static_cast<uint32_t>(m_FreeList.size());

        return RHIDescriptorHandle(HeapType, index);
    }

    void DescriptorHeapVulKan::Free(RHIDescriptorHandle handle)
    {
        if (!handle.IsValid() || handle.GetType() != HeapType)
        {
            return;
        }

        uint32_t index = handle.GetIndex();

        if (index >= Capacity)
        {
            ThrowErrorMessage("DescriptorHeapVulKan: Unknown index range");
            return;
        }

#if _DEBUG
        auto it = std::find(m_FreeList.begin(), m_FreeList.end(), index);
        if (it != m_FreeList.end())
        {
            ThrowErrorMessage("DescriptorHeapVulKan: Double free detected");
            return;
        }
#endif

        m_Descriptors[index].Reset();
        m_DescriptorSets[index] = VK_NULL_HANDLE;
        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_MAX_ENUM;
        m_FreeList.push_back(index);
        CurrentIndex = Capacity - static_cast<uint32_t>(m_FreeList.size());
    }

    RHIResource* DescriptorHeapVulKan::GetDescriptorHeapView(RHIDescriptorHandle handle) const
    {
        if (!handle.IsValid() || handle.GetType() != HeapType)
        {
            ThrowErrorMessage("DescriptorHeapVulKan: Invalid handle");
            return nullptr;
        }

        uint32_t index = handle.GetIndex();
        if (index >= Capacity)
        {
            ThrowErrorMessage("DescriptorHeapVulKan: Index out of range");
            return nullptr;
        }

        const auto& descriptor = m_Descriptors[index];
        if (descriptor.Type == DescriptorRangeType::CBV)
        {
            return nullptr;
        }

        return nullptr;
    }

    bool DescriptorHeapVulKan::IsFull() const
    {
        return CurrentIndex >= Capacity;
    }

    bool DescriptorHeapVulKan::AllocateDescriptorSet(VkDescriptorType type, VkDescriptorSet& outSet)
    {
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;

        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = 0;
        binding.descriptorType = type;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_ALL;

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &binding;

        VkResult result = vkCreateDescriptorSetLayout(GetDevice(), &layoutInfo, nullptr, &layout);
        if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("DescriptorHeapVulKan: Failed to create descriptor set layout");
            return false;
        }

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        result = vkAllocateDescriptorSets(GetDevice(), &allocInfo, &outSet);
        vkDestroyDescriptorSetLayout(GetDevice(), layout, nullptr);

        if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("DescriptorHeapVulKan: Failed to allocate descriptor set");
            return false;
        }

        return true;
    }

    bool DescriptorHeapVulKan::SetCBV(RHIDescriptorHandle handle, uint64_t bufferAddress, uint32_t range)
    {
        if (!handle.IsValid() || handle.GetType() != HeapType)
        {
            return false;
        }

        uint32_t index = handle.GetIndex();
        if (index >= Capacity)
        {
            return false;
        }

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        if (!AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorSet))
        {
            return false;
        }

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = reinterpret_cast<VkBuffer>(bufferAddress);
        bufferInfo.offset = 0;
        bufferInfo.range = range;       // buffer size

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(GetDevice(), 1, &descriptorWrite, 0, nullptr);

        m_DescriptorSets[index] = descriptorSet;
        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        m_Descriptors[index].Type = DescriptorRangeType::CBV;
        m_Descriptors[index].BufferAddress = bufferAddress;
        m_Descriptors[index].BufferRange = range;

        return true;
    }

    bool DescriptorHeapVulKan::SetSRV(RHIDescriptorHandle handle, uint64_t bufferAddress, uint32_t range)
    {
        if (!handle.IsValid() || handle.GetType() != HeapType)
        {
            return false;
        }

        uint32_t index = handle.GetIndex();
        if (index >= Capacity)
        {
            return false;
        }

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        if (!AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptorSet))
        {
            return false;
        }

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = reinterpret_cast<VkBuffer>(bufferAddress);
        bufferInfo.offset = 0;
        bufferInfo.range = range;

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(GetDevice(), 1, &descriptorWrite, 0, nullptr);

        m_DescriptorSets[index] = descriptorSet;
        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        m_Descriptors[index].Type = DescriptorRangeType::SRV;
        m_Descriptors[index].BufferAddress = bufferAddress;
        m_Descriptors[index].BufferRange = range;

        return true;
    }

    bool DescriptorHeapVulKan::SetSRV(RHIDescriptorHandle handle, VkImageView imageView, VkImageLayout layout)
    {
        if (!handle.IsValid() || handle.GetType() != HeapType)
        {
            return false;
        }

        uint32_t index = handle.GetIndex();
        if (index >= Capacity)
        {
            return false;
        }

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        if (!AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, descriptorSet))
        {
            return false;
        }

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageView = imageView;
        imageInfo.imageLayout = layout;
        imageInfo.sampler = VK_NULL_HANDLE;

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(GetDevice(), 1, &descriptorWrite, 0, nullptr);

        m_DescriptorSets[index] = descriptorSet;
        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        m_Descriptors[index].Type = DescriptorRangeType::SRV;
        m_Descriptors[index].ImageView = imageView;
        m_Descriptors[index].ImageLayout = layout;

        return true;
    }

    bool DescriptorHeapVulKan::SetUAV(RHIDescriptorHandle handle, uint64_t bufferAddress, uint32_t range)
    {
        if (!handle.IsValid() || handle.GetType() != HeapType)
        {
            return false;
        }

        uint32_t index = handle.GetIndex();
        if (index >= Capacity)
        {
            return false;
        }

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        if (!AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptorSet))
        {
            return false;
        }

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = reinterpret_cast<VkBuffer>(bufferAddress);
        bufferInfo.offset = 0;
        bufferInfo.range = range;

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(GetDevice(), 1, &descriptorWrite, 0, nullptr);

        m_DescriptorSets[index] = descriptorSet;
        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        m_Descriptors[index].Type = DescriptorRangeType::UAV;
        m_Descriptors[index].BufferAddress = bufferAddress;
        m_Descriptors[index].BufferRange = range;

        return true;
    }

    bool DescriptorHeapVulKan::SetUAV(RHIDescriptorHandle handle, VkImageView imageView, VkImageLayout layout)
    {
        if (!handle.IsValid() || handle.GetType() != HeapType)
        {
            return false;
        }

        uint32_t index = handle.GetIndex();
        if (index >= Capacity)
        {
            return false;
        }

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        if (!AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptorSet))
        {
            return false;
        }

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageView = imageView;
        imageInfo.imageLayout = layout;
        imageInfo.sampler = VK_NULL_HANDLE;

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(GetDevice(), 1, &descriptorWrite, 0, nullptr);

        m_DescriptorSets[index] = descriptorSet;
        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        m_Descriptors[index].Type = DescriptorRangeType::UAV;
        m_Descriptors[index].ImageView = imageView;
        m_Descriptors[index].ImageLayout = layout;

        return true;
    }

    bool DescriptorHeapVulKan::SetSampler(RHIDescriptorHandle handle, VkSampler sampler)
    {
        if (!handle.IsValid() || handle.GetType() != HeapType)
        {
            return false;
        }

        uint32_t index = handle.GetIndex();
        if (index >= Capacity)
        {
            return false;
        }

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        if (!AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_SAMPLER, descriptorSet))
        {
            return false;
        }

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler = sampler;

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(GetDevice(), 1, &descriptorWrite, 0, nullptr);

        m_DescriptorSets[index] = descriptorSet;
        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_SAMPLER;
        m_Descriptors[index].Type = DescriptorRangeType::Sampler;
        m_Descriptors[index].Sampler = sampler;

        return true;
    }

    const DescriptorData* DescriptorHeapVulKan::GetDescriptor(RHIDescriptorHandle handle) const
    {
        if (!handle.IsValid() || handle.GetType() != HeapType)
        {
            return nullptr;
        }

        uint32_t index = handle.GetIndex();
        if (index >= Capacity)
        {
            return nullptr;
        }

        return &m_Descriptors[index];
    }
}
