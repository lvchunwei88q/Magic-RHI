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

    // Mapping type: one Vulkan-slot description per root parameter
    enum class VKParamSlotType : uint8_t
    {
        None = 0,            // Parameter type not yet implemented / ignored
        DescriptorSet,       // DescriptorTable -> bind to one VkDescriptorSet (set# stored)
        PushConstant,        // Constants -> one or part of a VkPushConstantRange
        RootDescriptor       // CBV / SRV / UAV at root level (handled by command list dynamically)
    };

    struct RootParameterSlotVK
    {
        VKParamSlotType Type = VKParamSlotType::None;

        // For Type == DescriptorSet:
        // Index into m_SetLayouts, i.e. the 'set#' used in vkCmdBindDescriptorSets firstSet
        uint32_t DescriptorSetIndex = 0;

        // For Type == PushConstant:
        // Index into m_PushConstantRanges
        uint32_t PushRangeIndex = 0;
        // Base offset *within* this push constant range, in 32-bit values (mul 4 for bytes)
        uint32_t PushConstantBaseOffset32 = 0;
        // Total size of this param's constants, in 32-bit values
        uint32_t PushConstantCount32 = 0;

        // For Type == RootDescriptor:
        VkDescriptorType RootDescType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
        uint32_t RootDescShaderRegister = 0;
        uint32_t RootDescRegisterSpace = 0;
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

        // Param-slot mapping accessors
        uint32_t GetParamCount() const { return static_cast<uint32_t>(m_ParamSlots.size()); }
        const RootParameterSlotVK& GetParamSlot(uint32_t rootParamIndex) const { return m_ParamSlots[rootParamIndex]; }
        // Full root-parameter desc (deep-copied from the creation-time RootSignatureDesc)
        const RootParameterDesc& GetRootParameterDesc(uint32_t rootParamIndex) const { return m_Desc.Parameters[rootParamIndex]; }
        const RootSignatureDesc& GetDesc() const { return m_Desc; }

        // Push constant ranges (used by vkCmdPushConstants stageFlags/offset)
        uint32_t GetPushConstantRangeCount() const { return static_cast<uint32_t>(m_PushConstantRanges.size()); }
        const VkPushConstantRange& GetPushConstantRange(uint32_t idx) const { return m_PushConstantRanges[idx]; }

        const VkDevice GetDevice() const;

    private:
        // Here we just store device handle references
        const VkDevice* m_Device;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

        // Deep copy of the creation-time RootSignatureDesc so we can re-read
        // table ranges / parameter types at bind-time.
        RootSignatureDesc m_Desc;

        // A SetLayout for each DescriptorTable parameter (in the order they appear in RootSignatureDesc.Parameters)
        std::vector<DescriptorSetLayoutInfo> m_SetLayouts;

        // Slot mapping for ALL parameters, size == RootSignatureDesc.Parameters.size()
        std::vector<RootParameterSlotVK> m_ParamSlots;

        // Merged (non-overlapping) push constant ranges referenced by ParamSlots
        std::vector<VkPushConstantRange> m_PushConstantRanges;
    };
}