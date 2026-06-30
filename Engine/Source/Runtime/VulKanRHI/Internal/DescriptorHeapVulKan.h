#pragma once

#include <vulkan.h>
#include <RHIResource.h>

namespace RHI
{
    class DescriptorHeapVulKan : public RHIDescriptorHeap
    {
    public:
        DescriptorHeapVulKan(VkDescriptorPool descriptorPool, RHIDescriptorHeapType type, uint32_t capacity, VkDevice device)
            : RHIDescriptorHeap(type, capacity)
            , m_DescriptorPool(descriptorPool)
            , m_Device(device) {}
        ~DescriptorHeapVulKan() override = default;

        RHIDescriptorHandle Allocate() override;
        void Free(RHIDescriptorHandle handle) override;
        bool IsFull() const override;

        VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }
        VkDevice GetDevice() const { return m_Device; }

    private:
        VkDescriptorPool m_DescriptorPool;
        VkDevice m_Device;
    };
}
