#include "RHICommandListVulKan.h"
#include "DescriptorHeapVulKan.h"

#define ROOT_SIGNATURE_SET(pRootSignature,targetRS)                                                        \
    if (pRootSignature == nullptr)                                                                         \
    {targetRS = nullptr;return;}                                                                           \
    auto* pVKRS = SafeCast<RHIRootSignatureVulKan>(pRootSignature);                                        \
    if (pVKRS == nullptr || !pVKRS->IsValid())                                                             \
    {ThrowErrorMessage("CommandListVulKan::Set" #targetRS " - invalid root signature");return;}

#define CHECK_BUFFER_TYPE(RootSignature,rootParameterIndex,targetType)                                 \
        if (RootSignature && rootParameterIndex < RootSignature->GetParamCount())                      \
        {                                                                                              \
            const RootParameterSlotVK& slot = RootSignature->GetParamSlot(rootParameterIndex);         \
            if (slot.Type == targetType)                                                               \
            {                                                                                          \
                Core::WarningCapture::Capture(                                                         \
                    "Root parameter type " #targetType " not yet implemented in Vulkan RHI. "          \
                    "Root parameter index = " + std::to_string(rootParameterIndex));                   \
                return;                                                                                \
            }                                                                                          \
        }

#define CHECK_PARAM_INDEX(rootParameterIndex,targetRS)                                                                      \
        if (rootParameterIndex >= targetRS->GetParamCount()){                                                               \
            Core::WarningCapture::Capture("Root parameter index " + std::to_string(rootParameterIndex) + " out of range");  \
            return;                                                                                                         \
        }

// NOTE: Vulkan doesn't natively support D3D12's "root-level CBV/SRV/UAV" (a descriptor
// carried inside the root signature, with a GPU virtual address).
// The equivalent would be one of:
//   - VK_KHR_push_descriptors (update directly into cmd buffer without a pool)
//   - A small dedicated descriptor set per root descriptor, allocated each frame
//   - Rewriting the CBV/SRV/UAV slot of an existing "root params" descriptor set
// For now: leave a warning to make the missing behavior obvious at debug time.

namespace RHI
{
    // ====================================================================
    // Root signature & descriptor heap set
    // ====================================================================
    void CommandListVulKan::SetGraphicsRootSignature(RHIRootSignature* pRootSignature)
    {
        ROOT_SIGNATURE_SET(pRootSignature,m_pCurrentGraphicsRS);

        // Vulkan has no explicit 'bind root signature' command.
        // We just cache the pointer so subsequent SetRoot* calls know the layout.
        m_pCurrentGraphicsRS = pVKRS;
    }

    void CommandListVulKan::SetComputeRootSignature(RHIRootSignature* pRootSignature)
    {
        ROOT_SIGNATURE_SET(pRootSignature,m_pCurrentComputeRS);

        // Vulkan has no explicit 'bind root signature' command.
        // We just cache the pointer so subsequent SetRoot* calls know the layout.
        m_pCurrentComputeRS = pVKRS;
    }

    void CommandListVulKan::SetDescriptorHeaps(uint32_t /*numHeaps*/, RHIDescriptorHeap* const* /*ppHeaps*/)
    {
        // Vulkan doesn't need to bind descriptor heaps to a command list.
        // Descriptor sets (allocated from pools) are bound directly in
        // SetGraphics/ComputeRootDescriptorTable via vkCmdBindDescriptorSets.
    }

    // ====================================================================
    // Graphics pipeline binding — DescriptorTable / RootDescriptors / Constants
    // ====================================================================
    void CommandListVulKan::SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex,RHIDescriptorHeap* pDescriptorHeap,uint32_t offsetInDescriptorsFromTableStart)
    {
        if (m_pCurrentGraphicsRS == nullptr || pDescriptorHeap == nullptr) return;
        if (m_CommandBuffer == VK_NULL_HANDLE) return;

        DescriptorHeapVulKan* pHeap = SafeCast<DescriptorHeapVulKan>(pDescriptorHeap);
        if (pHeap == nullptr) return;
        // Check root parameter index
        CHECK_PARAM_INDEX(rootParameterIndex,m_pCurrentGraphicsRS);

        const RootParameterSlotVK& slot = m_pCurrentGraphicsRS->GetParamSlot(rootParameterIndex);
        if (slot.Type != VKParamSlotType::DescriptorSet){
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("CommandListVulKan::SetGraphicsRootDescriptorTable - invalid root parameter slot type");
#endif
            return;
        }

        const DescriptorSetLayoutInfo& layoutInfo = m_pCurrentGraphicsRS->GetDescriptorSetLayout(slot.DescriptorSetIndex);
        const RootParameterDesc& tableParam = m_pCurrentGraphicsRS->GetRootParameterDesc(rootParameterIndex);

        // TODO : Implement this method
        VkDescriptorSet set = pHeap->GetDescriptorSet({RHIDescriptorHeapType::Standard,0});
        if (set == VK_NULL_HANDLE) return;

        const uint32_t firstSet = slot.DescriptorSetIndex;
        vkCmdBindDescriptorSets(
            m_CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pCurrentGraphicsRS->GetPipelineLayout(),
            firstSet,
            1, &set,
            0, nullptr);
    }

    void CommandListVulKan::SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress)
    {
#if RHI_ENABLE_DEBUG_INFO
        CHECK_BUFFER_TYPE(m_pCurrentGraphicsRS,rootParameterIndex,VKParamSlotType::RootDescriptor);
#endif
        // TODO: Implement this method
        (void)rootParameterIndex; (void)gpuVirtualAddress;
    }

    void CommandListVulKan::SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress)
    {
#if RHI_ENABLE_DEBUG_INFO
        CHECK_BUFFER_TYPE(m_pCurrentGraphicsRS,rootParameterIndex,VKParamSlotType::RootDescriptor);
#endif
        // TODO: Implement this method
        (void)rootParameterIndex; (void)gpuVirtualAddress;
    }

    void CommandListVulKan::SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress)
    {
#if RHI_ENABLE_DEBUG_INFO
        CHECK_BUFFER_TYPE(m_pCurrentGraphicsRS,rootParameterIndex,VKParamSlotType::RootDescriptor);
#endif
        // TODO: Implement this method
        (void)rootParameterIndex; (void)gpuVirtualAddress;
    }

    void CommandListVulKan::SetGraphicsRoot32BitConstant(uint32_t rootParameterIndex,uint32_t value,uint32_t destOffsetIn32BitValues)
    {
        SetGraphicsRoot32BitConstants(rootParameterIndex, 1u, &value, destOffsetIn32BitValues);
    }

    void CommandListVulKan::SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex,uint32_t num32BitValues,const void* pSrcData,uint32_t destOffsetIn32BitValues)
    {
        if (m_pCurrentGraphicsRS == nullptr || pSrcData == nullptr || num32BitValues == 0) return;
        if (m_CommandBuffer == VK_NULL_HANDLE) return;
        // Check root parameter index
        CHECK_PARAM_INDEX(rootParameterIndex,m_pCurrentGraphicsRS);

        const RootParameterSlotVK& slot = m_pCurrentGraphicsRS->GetParamSlot(rootParameterIndex);
        if (slot.Type != VKParamSlotType::PushConstant) return;

        const VkPushConstantRange& range = m_pCurrentGraphicsRS->GetPushConstantRange(slot.PushRangeIndex);

        const uint32_t offsetInRange32 = slot.PushConstantBaseOffset32 + destOffsetIn32BitValues;
        if (offsetInRange32 + num32BitValues > slot.PushConstantCount32)
        {
#if RHI_ENABLE_DEBUG_INFO
            Core::WarningCapture::Capture(
                "SetGraphicsRoot32BitConstants: write outside push-constant range of parameter "
                + std::to_string(rootParameterIndex));
#endif
            return;
        }

        const uint32_t byteOffset = range.offset + offsetInRange32 * 4u;
        const uint32_t byteSize   = num32BitValues * 4u;

        vkCmdPushConstants(
            m_CommandBuffer,
            m_pCurrentGraphicsRS->GetPipelineLayout(),
            range.stageFlags,
            byteOffset, byteSize, pSrcData);
    }

    // ====================================================================
    // Compute pipeline binding — mirrors graphics but with PIPELINE_BIND_POINT_COMPUTE
    // ====================================================================
    void CommandListVulKan::SetComputeRootDescriptorTable(uint32_t rootParameterIndex,RHIDescriptorHeap* pDescriptorHeap,uint32_t offsetInDescriptorsFromTableStart)
    {
        if (m_pCurrentComputeRS == nullptr || pDescriptorHeap == nullptr) return;
        if (m_CommandBuffer == VK_NULL_HANDLE) return;

        DescriptorHeapVulKan* pHeap = SafeCast<DescriptorHeapVulKan>(pDescriptorHeap);
        if (pHeap == nullptr) return;
        // Check root parameter index
        CHECK_PARAM_INDEX(rootParameterIndex,m_pCurrentComputeRS);

        const RootParameterSlotVK& slot = m_pCurrentComputeRS->GetParamSlot(rootParameterIndex);
        if (slot.Type != VKParamSlotType::DescriptorSet){
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("CommandListVulKan::SetComputeRootDescriptorTable - invalid root parameter slot type");
#endif
            return;
        }

        const DescriptorSetLayoutInfo& layoutInfo = m_pCurrentComputeRS->GetDescriptorSetLayout(slot.DescriptorSetIndex);
        const RootParameterDesc& tableParam = m_pCurrentComputeRS->GetRootParameterDesc(rootParameterIndex);

        // TODO : Implement this method
        VkDescriptorSet set = pHeap->GetDescriptorSet({RHIDescriptorHeapType::Standard,0});
        if (set == VK_NULL_HANDLE) return;

        const uint32_t firstSet = slot.DescriptorSetIndex;
        vkCmdBindDescriptorSets(
            m_CommandBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            m_pCurrentComputeRS->GetPipelineLayout(),
            firstSet,
            1, &set,
            0, nullptr);
    }

    void CommandListVulKan::SetComputeRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress)
    {
#if RHI_ENABLE_DEBUG_INFO
        CHECK_BUFFER_TYPE(m_pCurrentComputeRS,rootParameterIndex,VKParamSlotType::RootDescriptor);
#endif
        // TODO: Implement ConstantBufferView
        (void)rootParameterIndex; (void)gpuVirtualAddress;
    }

    void CommandListVulKan::SetComputeRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress)
    {
#if RHI_ENABLE_DEBUG_INFO
        CHECK_BUFFER_TYPE(m_pCurrentComputeRS,rootParameterIndex,VKParamSlotType::RootDescriptor);
#endif
        // TODO: Implement ShaderResourceView
        (void)rootParameterIndex; (void)gpuVirtualAddress;
    }

    void CommandListVulKan::SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress)
    {
#if RHI_ENABLE_DEBUG_INFO
        CHECK_BUFFER_TYPE(m_pCurrentComputeRS,rootParameterIndex,VKParamSlotType::RootDescriptor);
#endif
        // TODO: Implement UnorderedAccessView
        (void)rootParameterIndex; (void)gpuVirtualAddress;
    }

    void CommandListVulKan::SetComputeRoot32BitConstant(uint32_t rootParameterIndex,uint32_t value,uint32_t destOffsetIn32BitValues)
    {
        SetComputeRoot32BitConstants(rootParameterIndex, 1u, &value, destOffsetIn32BitValues);
    }

    void CommandListVulKan::SetComputeRoot32BitConstants(uint32_t rootParameterIndex,uint32_t num32BitValues,const void* pSrcData,uint32_t destOffsetIn32BitValues)
    {
        if (m_pCurrentComputeRS == nullptr || pSrcData == nullptr || num32BitValues == 0) return;
        if (m_CommandBuffer == VK_NULL_HANDLE) return;
        // Check root parameter index
        CHECK_PARAM_INDEX(rootParameterIndex,m_pCurrentComputeRS);

        const RootParameterSlotVK& slot = m_pCurrentComputeRS->GetParamSlot(rootParameterIndex);
        if (slot.Type != VKParamSlotType::PushConstant) return;

        const VkPushConstantRange& range = m_pCurrentComputeRS->GetPushConstantRange(slot.PushRangeIndex);

        const uint32_t offsetInRange32 = slot.PushConstantBaseOffset32 + destOffsetIn32BitValues;
        if (offsetInRange32 + num32BitValues > slot.PushConstantCount32)
        {
#if RHI_ENABLE_DEBUG_INFO
            Core::WarningCapture::Capture(
                "SetComputeRoot32BitConstants: write outside push-constant range of parameter "
                + std::to_string(rootParameterIndex));
#endif
            return;
        }

        const uint32_t byteOffset = range.offset + offsetInRange32 * 4u;
        const uint32_t byteSize   = num32BitValues * 4u;

        vkCmdPushConstants(
            m_CommandBuffer,
            m_pCurrentComputeRS->GetPipelineLayout(),
            range.stageFlags,
            byteOffset, byteSize, pSrcData);
    }
}
