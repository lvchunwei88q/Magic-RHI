#pragma once

#include <vulkan.h>
#include "RHIResourceVulKan.h"
#include <vector>
#include <memory>

namespace RHI
{
    struct DescriptorView
    {
        std::unique_ptr<ConstantBufferViewVulKan> pCBV;
        std::unique_ptr<ShaderResourceViewVulKan> pSRV;
        std::unique_ptr<UnorderedAccessViewVulKan> pUAV;
        std::unique_ptr<RenderTargetViewVulKan> pRTV;
        std::unique_ptr<DepthStencilViewVulKan> pDSV;
        std::unique_ptr<SamplerStateVulKan> pSampler;
        RHIResourceType ViewType;

        void Release(){
            pCBV.reset();pSRV.reset();
            pUAV.reset();pRTV.reset();
            pDSV.reset();pSampler.reset();
            ViewType = RHIResourceType::RRT_None;
        }
    };

    struct DescriptorSetBinding
    {
        VkDescriptorSet set;
        VkDescriptorSetLayout layout;

        void Release(){
            layout = VK_NULL_HANDLE;
            set = VK_NULL_HANDLE;
        }

        DescriptorSetBinding() : layout(VK_NULL_HANDLE), set(VK_NULL_HANDLE) {}
        DescriptorSetBinding(VkDescriptorSet set, VkDescriptorSetLayout layout) : layout(layout), set(set) {}
        ~DescriptorSetBinding() {}
    };

    class DescriptorHeapVulKan : public RHIDescriptorHeap
    {
    public:
        DescriptorHeapVulKan(
            const VkDevice* device,
            RHIDescriptorHeapType type,
            const VkDescriptorPoolCreateInfo& poolInfo
        );
        ~DescriptorHeapVulKan() override;

        RHIDescriptorHandle Allocate() override;
        void Free(RHIDescriptorHandle handle) override;
        RHIResource* GetDescriptorHeapView(RHIDescriptorHandle handle) const override;
        bool IsFull() const override;

        // Set descriptor view
        bool SetCBVDescriptor(RHIDescriptorHandle handle, ConstantBufferViewVulKan* CBV);
        bool SetSRVDescriptor(RHIDescriptorHandle handle, ShaderResourceViewVulKan* SRV);
        bool SetUAVDescriptor(RHIDescriptorHandle handle, UnorderedAccessViewVulKan* UAV);
        bool SetRTVDescriptor(RHIDescriptorHandle handle, RenderTargetViewVulKan* RTV);
        bool SetDSVDescriptor(RHIDescriptorHandle handle, DepthStencilViewVulKan* DSV);
        bool SetSamplerDescriptor(RHIDescriptorHandle handle, SamplerStateVulKan* sampler);

        const DescriptorView* GetDescriptorView(RHIDescriptorHandle handle) const;
        const VkDevice GetDevice() const;

        bool GetDescriptorSetBinding(RHIDescriptorHandle handle, DescriptorSetBinding& outBinding) const;
        VkDescriptorType GetDescriptorType(RHIDescriptorHandle handle) const;

    private:
        // Allocate a descriptor set for a single resource
        bool AllocateDescriptorSet(VkDescriptorType type, VkDescriptorSet& outSet, VkDescriptorSetLayout& outLayout);

        // Actually write descriptor data into the allocated descriptor set
        void WriteCBVToSet(VkDescriptorSet set, ConstantBufferViewVulKan* CBV);
        void WriteSRVToSet(VkDescriptorSet set, ShaderResourceViewVulKan* SRV);
        void WriteUAVToSet(VkDescriptorSet set, UnorderedAccessViewVulKan* UAV);
        void WriteSamplerToSet(VkDescriptorSet set, SamplerStateVulKan* sampler);
        /*
         * Note: In Vk, there isn’t a direct equivalent of RTV and DSV in the descriptions, so that’s something to keep in mind.
         * void WriteRTVToSet(VkDescriptorSet set, RenderTargetViewVulKan* RTV);
         * void WriteDSVToSet(VkDescriptorSet set, DepthStencilViewVulKan* DSV);
         */
         
        // Here we just store device handle references
        const VkDevice* m_Device;
        VkDescriptorPool m_DescriptorPool;
        const uint32_t m_DescriptorSetCapacity;

        std::vector<uint32_t> m_FreeList;
        std::vector<DescriptorView> m_Descriptors;
        std::vector<DescriptorSetBinding> m_DescriptorBindings;
        std::vector<VkDescriptorType> m_DescriptorTypes;
    };
}
