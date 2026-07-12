#pragma once

#include "Common/VULKANRHI_API.h"
#include <RHIRootSignature.h>
#include <vulkan.h>
#include <vector>

namespace RHI
{
    struct DescriptorSetLayoutInfo
    {
        VkDescriptorSetLayout Layout = VK_NULL_HANDLE;
        uint32_t ParameterIndex = 0;  // Corresponding index of RootParameterDesc in RootSignatureDesc.Parameters
    };

    class RHIRootSignatureVulKan : public RHIRootSignature
    {
    public:
        RHIRootSignatureVulKan();
        ~RHIRootSignatureVulKan() override;

        bool Initialize(Device* device, const RootSignatureDesc& desc) override;
        void Shutdown() override;

        bool IsValid() const override;

        VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }
        uint32_t GetDescriptorSetLayoutCount() const { return static_cast<uint32_t>(m_SetLayouts.size()); }
        const DescriptorSetLayoutInfo& GetDescriptorSetLayout(uint32_t index) const { return m_SetLayouts[index]; }

        const VkDevice GetDevice() const;

    private:
        // Here we just store device handle references
        const VkDevice* m_Device;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        std::vector<DescriptorSetLayoutInfo> m_SetLayouts;      // A SetLayout for each DescriptorTable parameter
    };
}