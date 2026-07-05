#pragma once

#include <vulkan.h>
#include "RHIResourceVulKan.h"
#include <vector>
#include <memory>

namespace RHI
{
    struct DescriptorData
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
