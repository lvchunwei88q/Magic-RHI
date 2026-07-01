#pragma once

#include <vulkan.h>
#include <RHIResource.h>
#include <vector>
#include <memory>

namespace RHI
{
    struct DescriptorData
    {
        DescriptorRangeType Type;
        uint64_t BufferAddress = 0;
        VkImageView ImageView = VK_NULL_HANDLE;
        VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkSampler Sampler = VK_NULL_HANDLE;
        uint32_t BufferRange = 0;

        void Reset()
        {
            Type = DescriptorRangeType::CBV;
            BufferAddress = 0;
            ImageView = VK_NULL_HANDLE;
            ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            Sampler = VK_NULL_HANDLE;
            BufferRange = 0;
        }
    };

    class DescriptorHeapVulKan : public RHIDescriptorHeap
    {
    public:
        DescriptorHeapVulKan(
            VkDevice* device,
            RHIDescriptorHeapType type,
            uint32_t capacity,
            const std::vector<VkDescriptorPoolSize>& poolSizes
        );
        ~DescriptorHeapVulKan() override;

        RHIDescriptorHandle Allocate() override;
        void Free(RHIDescriptorHandle handle) override;
        RHIResource* GetDescriptorHeapView(RHIDescriptorHandle handle) const override;
        bool IsFull() const override;

        bool SetCBV(RHIDescriptorHandle handle, uint64_t bufferAddress, uint32_t range = VK_WHOLE_SIZE);
        bool SetSRV(RHIDescriptorHandle handle, uint64_t bufferAddress, uint32_t range = VK_WHOLE_SIZE);
        bool SetSRV(RHIDescriptorHandle handle, VkImageView imageView, VkImageLayout layout);
        bool SetUAV(RHIDescriptorHandle handle, uint64_t bufferAddress, uint32_t range = VK_WHOLE_SIZE);
        bool SetUAV(RHIDescriptorHandle handle, VkImageView imageView, VkImageLayout layout);
        bool SetSampler(RHIDescriptorHandle handle, VkSampler sampler);

        const DescriptorData* GetDescriptor(RHIDescriptorHandle handle) const;
        const VkDevice GetDevice() const;

    private:
        bool AllocateDescriptorSet(VkDescriptorType type, VkDescriptorSet& outSet);

        // Here we just store device handle references
        const VkDevice* m_Device;
        VkDescriptorPool m_DescriptorPool;
        const uint32_t m_DescriptorSetCapacity;

        std::vector<uint32_t> m_FreeList;
        std::vector<DescriptorData> m_Descriptors;
        std::vector<VkDescriptorSet> m_DescriptorSets;
        std::vector<VkDescriptorType> m_DescriptorTypes;
    };
}
