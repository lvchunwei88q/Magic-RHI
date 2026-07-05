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

        m_Descriptors[index].Release();
        m_DescriptorSets[index] = VK_NULL_HANDLE;
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

    bool DescriptorHeapVulKan::AllocateDescriptorSet(VkDescriptorType type, VkDescriptorSet& outSet)
    {
        /*
        * Here, we use a descriptor set to bind just one resource in order to align with other APIs.
        */
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

    bool DescriptorHeapVulKan::SetCBVDescriptor(RHIDescriptorHandle handle, ConstantBufferViewVulKan* CBV)
    {
        // Check descriptor handle
        DESCRIPTOR_SET_CHECK();

        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        if (!AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_DescriptorSets[index]))
            ThrowErrorMessage("DescriptorHeapVulKan: Failed to allocate descriptor set");
        
        m_Descriptors[index].ViewType = RHIResourceType::RRT_ConstantBufferView;
        m_Descriptors[index].pCBV = std::unique_ptr<ConstantBufferViewVulKan>(CBV);

        return true;
    }

    bool DescriptorHeapVulKan::SetSRVDescriptor(RHIDescriptorHandle handle, ShaderResourceViewVulKan* SRV)
    {
        // Check descriptor handle
        DESCRIPTOR_SET_CHECK();

        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        if (!AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, m_DescriptorSets[index]))
            ThrowErrorMessage("DescriptorHeapVulKan: Failed to allocate descriptor set");
        
        m_Descriptors[index].ViewType = RHIResourceType::RRT_ShaderResourceView;
        m_Descriptors[index].pSRV = std::unique_ptr<ShaderResourceViewVulKan>(SRV);

        return true;
    }

    bool DescriptorHeapVulKan::SetUAVDescriptor(RHIDescriptorHandle handle, UnorderedAccessViewVulKan* UAV)
    {
        // Check descriptor handle
        DESCRIPTOR_SET_CHECK();

        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        if (!AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, m_DescriptorSets[index]))
            ThrowErrorMessage("DescriptorHeapVulKan: Failed to allocate descriptor set");
        
        m_Descriptors[index].ViewType = RHIResourceType::RRT_UnorderedAccessView;
        m_Descriptors[index].pUAV = std::unique_ptr<UnorderedAccessViewVulKan>(UAV);

        return true;
    }

    bool DescriptorHeapVulKan::SetRTVDescriptor(RHIDescriptorHandle handle, RenderTargetViewVulKan* RTV)
    {
        // Check descriptor handle
        DESCRIPTOR_SET_CHECK();

        // TODO: RTV descriptor type
        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_MAX_ENUM;
        if (!AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_MAX_ENUM, m_DescriptorSets[index]))
            ThrowErrorMessage("DescriptorHeapVulKan: Failed to allocate descriptor set");
        
        m_Descriptors[index].ViewType = RHIResourceType::RRT_RenderTargetView;
        m_Descriptors[index].pRTV = std::unique_ptr<RenderTargetViewVulKan>(RTV);

        return true;
    }

    bool DescriptorHeapVulKan::SetDSVDescriptor(RHIDescriptorHandle handle, DepthStencilViewVulKan* DSV)
    {
        // Check descriptor handle
        DESCRIPTOR_SET_CHECK();

        // TODO: DSV descriptor type
        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_MAX_ENUM;
        if (!AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_MAX_ENUM, m_DescriptorSets[index]))
            ThrowErrorMessage("DescriptorHeapVulKan: Failed to allocate descriptor set");
        
        m_Descriptors[index].ViewType = RHIResourceType::RRT_DepthStencilView;
        m_Descriptors[index].pDSV = std::unique_ptr<DepthStencilViewVulKan>(DSV);

        return true;
    }

    bool DescriptorHeapVulKan::SetSamplerDescriptor(RHIDescriptorHandle handle, SamplerStateVulKan* sampler)
    {
        // Check descriptor handle
        DESCRIPTOR_SET_CHECK();

        m_DescriptorTypes[index] = VK_DESCRIPTOR_TYPE_SAMPLER;
        if (!AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_SAMPLER, m_DescriptorSets[index]))
            ThrowErrorMessage("DescriptorHeapVulKan: Failed to allocate descriptor set");
        
        m_Descriptors[index].ViewType = RHIResourceType::RRT_SamplerState;
        m_Descriptors[index].pSampler = std::unique_ptr<SamplerStateVulKan>(sampler);

        return true;
    }

    const DescriptorData* DescriptorHeapVulKan::GetDescriptor(RHIDescriptorHandle handle) const
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
