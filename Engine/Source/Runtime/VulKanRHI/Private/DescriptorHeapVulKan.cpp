#include <Common/Check.h>
#include "DescriptorHeapVulKan.h"
#include "Common/RHITypes.h"

#define DESCRIPTOR_SET_CHECK()if (!handle.IsValid() || handle.GetType() != HeapType)return false;   \
        uint32_t index = handle.GetIndex();                                                         \
        if (index >= Capacity)return false;

namespace RHI
{
    DescriptorHeapVulKan::DescriptorHeapVulKan(
        const VkDevice* device,
        RHIDescriptorHeapType type,
        const VkDescriptorPoolCreateInfo& poolInfo
    )
        : RHIDescriptorHeap(type, poolInfo.maxSets)
        , m_Device(device)
        , m_DescriptorPool(VK_NULL_HANDLE)
        , m_DescriptorSetCapacity(poolInfo.maxSets)
    {
        // Get the capacity from the pool info
        uint32_t capacity = poolInfo.maxSets;
        if (capacity == 0)
        {
            ThrowErrorMessage("DescriptorHeapVulKan: capacity cannot be 0");
            return;
        }

        VkResult result = vkCreateDescriptorPool(GetDevice(), &poolInfo, nullptr, &m_DescriptorPool);
        if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("DescriptorHeapVulKan: Failed to create descriptor pool");
            return;
        }

        m_FreeList.reserve(capacity);
        m_Descriptors.reserve(capacity);
        m_DescriptorBindings.reserve(capacity);
        m_DescriptorTypes.reserve(capacity);

        for (uint32_t i = 0; i < capacity; ++i)
        {
            m_FreeList.push_back(i);
            m_Descriptors.push_back({});
            m_DescriptorBindings.push_back({});
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

        // Release the previously allocated descriptor sets (they were allocated from the same descriptor pool and will be automatically released with reset/free,
        // But here, since we use per-handle sets, we manage them uniformly by releasing the pool, so here we just set the resources to null.
        m_Descriptors[index].Release();
        // TODO: Release the descriptor set layout as well.
        m_DescriptorBindings[index].Release();
        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_MAX_ENUM;
        m_FreeList.push_back(index);
        CurrentIndex = Capacity - static_cast<uint32_t>(m_FreeList.size());
    }

    RHIResource* DescriptorHeapVulKan::GetDescriptorHeapView(RHIDescriptorHandle handle) const
    {
        if (!handle.IsValid() || handle.GetType() != HeapType)
        {
            ThrowErrorMessage("Error GetDescriptorHeapView Unknown index range");
            return nullptr;
        }
        uint32_t index = handle.GetIndex();
        auto& descriptor = m_Descriptors[index];
        if (descriptor.ViewType == RHIResourceType::RRT_None)
        {
            ThrowErrorMessage("Error GetDescriptorHeapView Unknown index range");
            return nullptr;
        }
        return descriptor.ViewType == RHIResourceType::RRT_ConstantBufferView
            ? SafeCast<RHIResource>(descriptor.pCBV.get())
            : descriptor.ViewType == RHIResourceType::RRT_ShaderResourceView
            ? SafeCast<RHIResource>(descriptor.pSRV.get())
            : descriptor.ViewType == RHIResourceType::RRT_UnorderedAccessView
            ? SafeCast<RHIResource>(descriptor.pUAV.get())
            : descriptor.ViewType == RHIResourceType::RRT_RenderTargetView
            ? SafeCast<RHIResource>(descriptor.pRTV.get())
            : descriptor.ViewType == RHIResourceType::RRT_DepthStencilView
            ? SafeCast<RHIResource>(descriptor.pDSV.get())
            : descriptor.ViewType == RHIResourceType::RRT_SamplerState
            ? SafeCast<RHIResource>(descriptor.pSampler.get())
            : nullptr;
    }

    bool DescriptorHeapVulKan::IsFull() const
    {
        return CurrentIndex >= Capacity;
    }

    bool DescriptorHeapVulKan::GetDescriptorSetBinding(RHIDescriptorHandle handle, DescriptorSetBinding& outBinding) const
    {
        if (!handle.IsValid() || handle.GetType() != HeapType) return false;
        uint32_t index = handle.GetIndex();
        if (index >= Capacity) return false;
        outBinding = m_DescriptorBindings[index];
        return true;
    }

    VkDescriptorType DescriptorHeapVulKan::GetDescriptorType(RHIDescriptorHandle handle) const
    {
        if (!handle.IsValid() || handle.GetType() != HeapType) return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        uint32_t index = handle.GetIndex();
        if (index >= Capacity) return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        return m_DescriptorTypes[index];
    }

    bool DescriptorHeapVulKan::AllocateDescriptorSet(VkDescriptorType type, VkDescriptorSet& outSet, VkDescriptorSetLayout& outLayout)
    {
        /*
          * Allocate a set from the pool using a 1-binding descriptor set layout.
          * This way, each resource has its own VkDescriptorSet and layout,
          * making it easier to bind them flexibly later in the CommandList (SetGraphicsRootDescriptorTable).
         */
        //VkDescriptorSetLayout layout = VK_NULL_HANDLE;

        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = 0;
        binding.descriptorType = type;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_ALL;
        binding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &binding;

        VkResult result = vkCreateDescriptorSetLayout(GetDevice(), &layoutInfo, nullptr, &outLayout);
        if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("DescriptorHeapVulKan: Failed to create descriptor set layout");
            return false;
        }

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &outLayout;

        result = vkAllocateDescriptorSets(GetDevice(), &allocInfo, &outSet);

        if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("DescriptorHeapVulKan: Failed to allocate descriptor set");
            return false;
        }

        return true;
    }

    // ============ Write the view information into the descriptor set ============

    void DescriptorHeapVulKan::WriteCBVToSet(VkDescriptorSet set, ConstantBufferViewVulKan* CBV)
    {
        if (!CBV || set == VK_NULL_HANDLE) return;

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = CBV->GetBuffer();
        bufferInfo.offset = CBV->GetOffset();
        bufferInfo.range  = CBV->GetRange();

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set;
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(GetDevice(), 1, &write, 0, nullptr);
    }

    void DescriptorHeapVulKan::WriteSRVToSet(VkDescriptorSet set, ShaderResourceViewVulKan* SRV)
    {
        if (!SRV || set == VK_NULL_HANDLE) return;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set;
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;

        // ShaderResourceViewInfo
        union ShaderResourceViewInfo
        {
            VkDescriptorImageInfo imageInfo;
            VkDescriptorBufferInfo bufferInfo;
        };
        ShaderResourceViewInfo srvInfo = {};

        // if SRV is Image
        if (SRV->GetImageView() != VK_NULL_HANDLE)
        {
            // === Image SRV ===
            srvInfo.imageInfo.imageView = SRV->GetImageView();
            srvInfo.imageInfo.imageLayout = SRV->GetImageLayout();
            srvInfo.imageInfo.sampler = VK_NULL_HANDLE;

            write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            write.pImageInfo = &srvInfo.imageInfo;
        }
        else // if SRV is not Image        
        {
            // === Buffer SRV(ByteAddressBuffer/StructuredBuffer ...) ===
            srvInfo.bufferInfo.buffer = SRV->GetBuffer();
            srvInfo.bufferInfo.offset = SRV->GetOffset();
            srvInfo.bufferInfo.range  = SRV->GetRange();

            write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            write.pBufferInfo = &srvInfo.bufferInfo;
        }

        vkUpdateDescriptorSets(GetDevice(), 1, &write, 0, nullptr);
    }

    void DescriptorHeapVulKan::WriteUAVToSet(VkDescriptorSet set, UnorderedAccessViewVulKan* UAV)
    {
        if (!UAV || set == VK_NULL_HANDLE) return;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set;
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;

        // UnorderedAccessViewInfo
        union UnorderedAccessViewInfo
        {
            VkDescriptorImageInfo imageInfo;
            VkDescriptorBufferInfo bufferInfo;
        };
        UnorderedAccessViewInfo uavInfo = {};

        // if UAV is Image
        if (UAV->GetImageView() != VK_NULL_HANDLE)
        {
            // === Image UAV ===
            uavInfo.imageInfo.imageView = UAV->GetImageView();
            uavInfo.imageInfo.imageLayout = UAV->GetImageLayout();
            uavInfo.imageInfo.sampler = VK_NULL_HANDLE;

            write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            write.pImageInfo = &uavInfo.imageInfo;
        }
        else// if UAV is not Image
        {
            // === Buffer UAV ===
            uavInfo.bufferInfo.buffer = UAV->GetBuffer();
            uavInfo.bufferInfo.offset = UAV->GetOffset();
            uavInfo.bufferInfo.range  = UAV->GetRange();

            write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            write.pBufferInfo = &uavInfo.bufferInfo;
        }

        vkUpdateDescriptorSets(GetDevice(), 1, &write, 0, nullptr);
    }

    void DescriptorHeapVulKan::WriteSamplerToSet(VkDescriptorSet set, SamplerStateVulKan* sampler)
    {
        if (!sampler || set == VK_NULL_HANDLE) return;

        VkDescriptorImageInfo samplerInfo = {};
        samplerInfo.sampler = sampler->GetSampler();
        samplerInfo.imageView = VK_NULL_HANDLE;
        samplerInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set;
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        write.pImageInfo = &samplerInfo;

        vkUpdateDescriptorSets(GetDevice(), 1, &write, 0, nullptr);
    }

    // ============ SetXxxDescriptor Interface ============
    bool DescriptorHeapVulKan::SetCBVDescriptor(RHIDescriptorHandle handle, ConstantBufferViewVulKan* CBV)
    {
        if (!CBV) return false;
        DESCRIPTOR_SET_CHECK();

        DescriptorSetBinding& binding = m_DescriptorBindings[index];
        if (binding.set == VK_NULL_HANDLE)
        {
            m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            if (!AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding.set, binding.layout))
            {
                ThrowErrorMessage("DescriptorHeapVulKan::SetCBVDescriptor - AllocateDescriptorSet failed");return false;
            }
        }

        m_Descriptors[index].ViewType = RHIResourceType::RRT_ConstantBufferView;
        m_Descriptors[index].pCBV = std::unique_ptr<ConstantBufferViewVulKan>(CBV);

        WriteCBVToSet(binding.set, m_Descriptors[index].pCBV.get());

        return true;
    }

    bool DescriptorHeapVulKan::SetSRVDescriptor(RHIDescriptorHandle handle, ShaderResourceViewVulKan* SRV)
    {
        if (!SRV) return false;
        DESCRIPTOR_SET_CHECK();

        // Choose the correct descriptor type based on Image vs Buffer
        const VkDescriptorType descType = (SRV->GetImageView() != VK_NULL_HANDLE)
            ? VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
            : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

        DescriptorSetBinding& binding = m_DescriptorBindings[index];
        if (binding.set == VK_NULL_HANDLE)
        {
            m_DescriptorTypes[index] = descType;
            if (!AllocateDescriptorSet(descType, binding.set, binding.layout))
            {
                ThrowErrorMessage("DescriptorHeapVulKan::SetSRVDescriptor - AllocateDescriptorSet failed");return false;
            }
        }

        m_Descriptors[index].ViewType = RHIResourceType::RRT_ShaderResourceView;
        m_Descriptors[index].pSRV = std::unique_ptr<ShaderResourceViewVulKan>(SRV);

        WriteSRVToSet(binding.set, m_Descriptors[index].pSRV.get());

        return true;
    }

    bool DescriptorHeapVulKan::SetUAVDescriptor(RHIDescriptorHandle handle, UnorderedAccessViewVulKan* UAV)
    {
        if (!UAV) return false;
        DESCRIPTOR_SET_CHECK();

        // Choose the correct descriptor type based on Image vs Buffer
        const VkDescriptorType descType = (UAV->GetImageView() != VK_NULL_HANDLE)
            ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
            : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

        DescriptorSetBinding& binding = m_DescriptorBindings[index];
        if (binding.set == VK_NULL_HANDLE)
        {
            m_DescriptorTypes[index] = descType;
            if (!AllocateDescriptorSet(descType, binding.set, binding.layout))
            {
                ThrowErrorMessage("DescriptorHeapVulKan::SetUAVDescriptor - AllocateDescriptorSet failed");return false;
            }
        }

        m_Descriptors[index].ViewType = RHIResourceType::RRT_UnorderedAccessView;
        m_Descriptors[index].pUAV = std::unique_ptr<UnorderedAccessViewVulKan>(UAV);

        WriteUAVToSet(binding.set, m_Descriptors[index].pUAV.get());

        return true;
    }

    bool DescriptorHeapVulKan::SetRTVDescriptor(RHIDescriptorHandle handle, RenderTargetViewVulKan* RTV)
    {
        /*
        * Note: In Vulkan, a Render Target View (VkImageView as a color attachment) is not a descriptor.
        * It gets bound during rendering via VkFramebuffer / VkRenderPass.
        * Here, we don't allocate a descriptor set or call vkUpdateDescriptorSets;
        * we just save the pointer to the RTV for later use when creating a Framebuffer.
        */
        if (!RTV) return false;
        DESCRIPTOR_SET_CHECK();

        // Do not allocate descriptor set, m_DescriptorSets[index] remains VK_NULL_HANDLE
        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_MAX_ENUM;

        m_Descriptors[index].ViewType = RHIResourceType::RRT_RenderTargetView;
        m_Descriptors[index].pRTV = std::unique_ptr<RenderTargetViewVulKan>(RTV);

        return true;
    }

    bool DescriptorHeapVulKan::SetDSVDescriptor(RHIDescriptorHandle handle, DepthStencilViewVulKan* DSV)
    {
        /*
        * Similar to RTV: DSV in Vulkan is not a descriptor type, but a depth/stencil attachment of VkFramebuffer.
        * Here we only store the view data, without allocating a descriptor set.
        */
        if (!DSV) return false;
        DESCRIPTOR_SET_CHECK();

        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_MAX_ENUM;

        m_Descriptors[index].ViewType = RHIResourceType::RRT_DepthStencilView;
        m_Descriptors[index].pDSV = std::unique_ptr<DepthStencilViewVulKan>(DSV);

        return true;
    }

    bool DescriptorHeapVulKan::SetSamplerDescriptor(RHIDescriptorHandle handle, SamplerStateVulKan* sampler)
    {
        if (!sampler) return false;
        DESCRIPTOR_SET_CHECK();

        DescriptorSetBinding& binding = m_DescriptorBindings[index];
        if (binding.set == VK_NULL_HANDLE)
        {
            m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_SAMPLER;
            if (!AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_SAMPLER, binding.set, binding.layout))
            {
                ThrowErrorMessage("DescriptorHeapVulKan::SetSamplerDescriptor - AllocateDescriptorSet failed");return false;
            }
        }

        m_Descriptors[index].ViewType = RHIResourceType::RRT_SamplerState;
        m_Descriptors[index].pSampler = std::unique_ptr<SamplerStateVulKan>(sampler);

        WriteSamplerToSet(binding.set, m_Descriptors[index].pSampler.get());

        return true;
    }

    const DescriptorView* DescriptorHeapVulKan::GetDescriptorView(RHIDescriptorHandle handle) const
    {
        if (!handle.IsValid() || handle.GetType() != HeapType)
        {
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("Invalid descriptor handle type.");
#endif
            return nullptr;
        }

        uint32_t index = handle.GetIndex();
        if (index >= Capacity)
        {
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("Invalid descriptor handle index.");
#endif
            return nullptr;
        }

        return &m_Descriptors[index];
    }
}
