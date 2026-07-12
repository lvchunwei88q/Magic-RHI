#include "RHIRootSignatureVulKan.h"
#include "CoreMinimal.h"
#include "RHIVulKan.h"

#include <Common/Check.h>
#include <map>

namespace RHI
{
    namespace
    {
        // ========== Translate helpers ==========
        VkShaderStageFlags ShaderVisibilityToStageFlags(ShaderVisibility vis)
        {
            VkShaderStageFlags flags = 0;
            switch (vis)
            {
            case ShaderVisibility::None:          flags = 0;                                              break;
            case ShaderVisibility::All:           flags = VK_SHADER_STAGE_ALL;                            break;
            case ShaderVisibility::Vertex:        flags = VK_SHADER_STAGE_VERTEX_BIT;                     break;
            case ShaderVisibility::Pixel:         flags = VK_SHADER_STAGE_FRAGMENT_BIT;                   break;
            case ShaderVisibility::Hull:          flags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;       break;
            case ShaderVisibility::Domain:        flags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;    break;
            case ShaderVisibility::Geometry:      flags = VK_SHADER_STAGE_GEOMETRY_BIT;                   break;
            case ShaderVisibility::Amplification: flags = VK_SHADER_STAGE_TASK_BIT_NV;                    break;
            case ShaderVisibility::Mesh:          flags = VK_SHADER_STAGE_MESH_BIT_NV;                    break;
            case ShaderVisibility::Compute:       flags = VK_SHADER_STAGE_COMPUTE_BIT;                    break;
            default:                              flags = VK_SHADER_STAGE_ALL;                            break;
            }
            return flags;
        }

        // Translate DescriptorRangeType to VkDescriptorType
        VkDescriptorType DescriptorRangeTypeToVk(DescriptorRangeType type)
        {
            switch (type)
            {
            case DescriptorRangeType::SRV:     return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case DescriptorRangeType::UAV:     return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            case DescriptorRangeType::CBV:     return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case DescriptorRangeType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
            default:                           return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            }
        }

        // Translate RootParameterType to VkDescriptorType
        VkDescriptorType RootParamToVkDescType(RootParameterType type)
        {
            switch (type)
            {
            case RootParameterType::CBV: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case RootParameterType::SRV: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case RootParameterType::UAV: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            default:                     return VK_DESCRIPTOR_TYPE_MAX_ENUM;
            }
        }

        // Organize and merge PushConstant Ranges
        std::vector<VkPushConstantRange> MergePushConstantRanges(
            const std::vector<VkPushConstantRange>& inputRanges,
            uint32_t alignment = 4)
        {
            if (inputRanges.empty()) {
                return {};
            }

            // ============================================================
            // Merge sizes by stageFlags
            // Use map to store max size for each stageFlags combination
            // ============================================================
            std::map<VkShaderStageFlags, uint32_t> mergedSizes;

            for (const auto& range : inputRanges) {
                // If this stageFlags combination already exists, take the larger size
                auto it = mergedSizes.find(range.stageFlags);
                if (it != mergedSizes.end()) {
                    it->second = max(it->second, range.size);
                } else
                    mergedSizes[range.stageFlags] = range.size;
            }

            // ============================================================
            // Merge overlapping stageFlags
            // Check for conflicts between overlapping stageFlags
            // ============================================================
            std::map<VkShaderStageFlags, uint32_t> finalMerged;
            std::vector<VkShaderStageFlags> keys;
            for (const auto& pair : mergedSizes) {
                keys.push_back(pair.first);
            }

            // Greedy merge: merge whenever there's an overlap
            std::vector<bool> used(keys.size(), false);
            
            for (size_t i = 0; i < keys.size(); ++i) {
                if (used[i]) continue;

                VkShaderStageFlags combinedFlags = keys[i];
                uint32_t combinedSize = mergedSizes[keys[i]];

                for (size_t j = i + 1; j < keys.size(); ++j) {
                    if (used[j]) continue;

                    // Check if there's an overlap between combinedFlags and keys[j]
                    if (combinedFlags & keys[j]) {
                        // There's an overlap, merge
                        combinedFlags |= keys[j];
                        combinedSize = max(combinedSize, mergedSizes[keys[j]]);
                        used[j] = true;
                    }
                }

                finalMerged[combinedFlags] = combinedSize;
                used[i] = true;
            }

            // ============================================================
            // Recalculate offsets
            // ============================================================
            std::vector<VkPushConstantRange> result;
            uint32_t currentOffset = 0;

            for (const auto& pair : finalMerged) {
                VkPushConstantRange range;
                range.stageFlags = pair.first;
                range.size = pair.second;

                // Align to alignment
                currentOffset = (currentOffset + alignment - 1) & ~(alignment - 1);
                range.offset = currentOffset;currentOffset += range.size;

                result.push_back(range);
            }

            return result;
        }
    }

    // ====================================================
    // DeviceVulKan: RootSignature Interface
    // ====================================================
    std::shared_ptr<RHIRootSignature> DeviceVulKan::CreateRootSignature(const RootSignatureDesc& desc)
    {
        auto rootSignature = std::make_shared<RHIRootSignatureVulKan>();
        if (rootSignature->Initialize(this, desc))
        {
            return rootSignature;
        }
        return nullptr;
    }

    void DeviceVulKan::DeleteRootSignature(std::shared_ptr<RHIRootSignature>& rootSignature)
    {
        if (rootSignature)
        {
            rootSignature->Shutdown();
            rootSignature.reset();
        }
    }

    // ====================================================
    // RHIRootSignatureVulKan achieve
    // ====================================================
    const VkDevice RHIRootSignatureVulKan::GetDevice() const
    {
        if (!m_Device)
        {
            ThrowErrorMessage("RHIRootSignatureVulKan: Device is null");
            return VK_NULL_HANDLE;
        }
        return *m_Device;
    }

    RHIRootSignatureVulKan::RHIRootSignatureVulKan()
    {
    }

    RHIRootSignatureVulKan::~RHIRootSignatureVulKan()
    {
        Shutdown();
    }

    bool RHIRootSignatureVulKan::Initialize(Device* device, const RootSignatureDesc& desc)
    {
        DeviceVulKan* vkDevice = static_cast<DeviceVulKan*>(device);
        if (!vkDevice || !vkDevice->GetDevice() || *vkDevice->GetDevice() == VK_NULL_HANDLE)
        {
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("RHIRootSignatureVulKan::Initialize - Device is null");
#endif
            return false;
        }
        m_Device = vkDevice->GetDevice();

        // Deep-copy the desc so bind-time code can access ranges / param types later.
        m_Desc = desc;
        // Initialize slots (one per root parameter, default to None)
        m_ParamSlots.resize(desc.Parameters.size());

        // Used for the subsequent PipelineLayout: one Set per DescriptorTable parameter, keeping the order consistent with the parameters in RootSignatureDesc.Parameters.
        std::vector<VkDescriptorSetLayout> layoutHandles;
        layoutHandles.reserve(desc.Parameters.size());
        // Constants parameters become PushConstantRanges (one per param; offsets computed in order to avoid overlap)
        std::vector<VkPushConstantRange> temp_pushConstantRanges;

        auto CreateDescriptorSetLayout = [this](const VkDescriptorSetLayoutCreateInfo& info, VkDescriptorSetLayout& outLayout) -> bool {
            if (vkCreateDescriptorSetLayout(GetDevice(), &info, nullptr, &outLayout) != VK_SUCCESS)
            {
                ThrowErrorMessage("RHIRootSignatureVulKan::Initialize - Failed to create descriptor set layout");
                Shutdown();return false;
            }
            return true;
        };

        // Scan All RootParameter
        for (uint32_t paramIdx = 0; paramIdx < static_cast<uint32_t>(desc.Parameters.size()); ++paramIdx)
        {
            const RootParameterDesc& param = desc.Parameters[paramIdx];
            RootParameterSlotVK& slot = m_ParamSlots[paramIdx];
            // Get Shader Stage Flags
            const VkShaderStageFlags stageFlags = ShaderVisibilityToStageFlags(param.Visibility);

            switch (param.Type)
            {
                // ============ Descriptor Table ============
                case RootParameterType::DescriptorTable:
                {
                    slot.Type = VKParamSlotType::DescriptorSet;

                    const uint32_t descriptorSetIndex = static_cast<uint32_t>(m_SetLayouts.size());

                    const uint32_t rangeCount = param.DescriptorTable.NumDescriptorRanges;
                    const DescriptorRangeDesc* pRanges = param.DescriptorTable.pDescriptorRanges;
                    if (rangeCount == 0 || pRanges == nullptr)
                    {
                        // Empty table, create an empty layout to keep set numbers aligned with DescriptorTable count
                        VkDescriptorSetLayoutCreateInfo emptyInfo = {};
                        emptyInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                        emptyInfo.bindingCount = 0;
                        emptyInfo.pBindings = nullptr;
                        VkDescriptorSetLayout emptyLayout = VK_NULL_HANDLE;
                        if (!CreateDescriptorSetLayout(emptyInfo, emptyLayout)) return false;

#if RHI_ENABLE_DEBUG_INFO
                        // if empty table, capture warning message
                        Core::WarningCapture::Capture("RHIRootSignatureVulKan::Initialize - DescriptorTable "
                            + std::to_string(paramIdx) + " is empty, create empty descriptor set layout");
#endif
                        layoutHandles.push_back(emptyLayout);
                        m_SetLayouts.push_back({ emptyLayout, paramIdx });

                        slot.DescriptorSetIndex = descriptorSetIndex;
                        break;
                    }

                    // Translate each Range to a VkDescriptorSetLayoutBinding
                    std::vector<VkDescriptorSetLayoutBinding> bindings;
                    bindings.reserve(rangeCount);

                    for (uint32_t i = 0; i < rangeCount; ++i)
                    {
                        const DescriptorRangeDesc& range = pRanges[i];

                        VkDescriptorSetLayoutBinding binding = {};
                        // binding number = ShaderRegister + RegisterSpace * REG_SPACE_BASE.
                        // For simplicity and compatibility with simple shaders that only use space0, we just use ShaderRegister.
                        // If you later need multi-space support, define REG_SPACE_BASE = 65536 or use separate sets per space.
                        binding.binding = range.ShaderRegister;
                        binding.descriptorType = DescriptorRangeTypeToVk(range.RangeType);
                        binding.descriptorCount = range.NumDescriptors;
                        binding.stageFlags = stageFlags;
                        binding.pImmutableSamplers = nullptr;
                        bindings.push_back(binding);

                        (void)range.OffsetInDescriptorsFromTableStart; // Used at bind-time (SetGraphicsRootDescriptorTable), not layout-creation time
                        (void)range.RegisterSpace; // See binding.binding comment above
                    }

                    // Fill DescriptorSetLayoutCreateInfo
                    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
                    layoutInfo.pBindings = bindings.data();

                    // Create DescriptorSetLayout
                    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
                    if (!CreateDescriptorSetLayout(layoutInfo, setLayout)) return false;

                    layoutHandles.push_back(setLayout);
                    m_SetLayouts.push_back({ setLayout, paramIdx });

                    slot.Type = VKParamSlotType::DescriptorSet;
                    slot.DescriptorSetIndex = descriptorSetIndex;
                    break;
                }

                // ============ Root CBV/SRV/UAV ============
                case RootParameterType::CBV:
                case RootParameterType::SRV:
                case RootParameterType::UAV:
                {
                    slot.Type = VKParamSlotType::RootDescriptor;

                    slot.RootDescType = RootParamToVkDescType(param.Type);
                    slot.RootDescShaderRegister = param.Descriptor.ShaderRegister;
                    slot.RootDescRegisterSpace = param.Descriptor.RegisterSpace;
                    break;
                }

                // ============ Constants: Push Constant ============
                case RootParameterType::Constants:
                {
                    slot.Type = VKParamSlotType::PushConstant;

                    uint32_t num32 = (param.Constants.Num32BitValues == 0) ? 1u : param.Constants.Num32BitValues;
                    uint32_t sizeBytes = num32 * 4;

                    VkPushConstantRange pushRange = {};
                    pushRange.stageFlags = (stageFlags == 0) ? VK_SHADER_STAGE_ALL : stageFlags;
                    //  Set it to 0 by default for now 
                    pushRange.offset = 0;
                    pushRange.size = sizeBytes;

                    const uint32_t pushRangeIndex = static_cast<uint32_t>(temp_pushConstantRanges.size());
                    temp_pushConstantRanges.push_back(pushRange);

                    slot.PushRangeIndex = pushRangeIndex;
                    // We never merge constants across params, so each param starts at 0 within its own range
                    slot.PushConstantBaseOffset32 = 0;  
                    slot.PushConstantCount32 = num32;
                    break;
                }

                default:
                    slot.Type = VKParamSlotType::None;
                    break;
            }
        }

        // Merge PushConstant ranges: Vulkan requires that ranges with the same stageFlags do not overlap
        // Note: Here we assume the device's maximum supported constant buffer size is 256.
        m_PushConstantRanges = MergePushConstantRanges(temp_pushConstantRanges);

        // ============ Fill Create PipelineLayout ============
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        // Set layout count and layout
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layoutHandles.size());
        pipelineLayoutInfo.pSetLayouts = layoutHandles.empty() ? nullptr : layoutHandles.data();
        /*
         * pPushConstantRanges indicates how many PushConstants there are.
         * Then Vk reads the values we set in PushConstantRange for the Shader.
         * But if there are two PushConstants whose VkShaderStageFlags overlap, it will cause an error.
        */
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(m_PushConstantRanges.size());
        pipelineLayoutInfo.pPushConstantRanges = m_PushConstantRanges.empty() ? nullptr : m_PushConstantRanges.data();

        // Create PipelineLayout
        if (vkCreatePipelineLayout(GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
        {
            ThrowErrorMessage("RHIRootSignatureVulKan::Initialize - Failed to create pipeline layout");
            Shutdown();return false;
        }

        return true;
    }

    void RHIRootSignatureVulKan::Shutdown()
    {
        if (GetDevice() != VK_NULL_HANDLE)
        {
            if (m_PipelineLayout != VK_NULL_HANDLE)
            {
                vkDestroyPipelineLayout(GetDevice(), m_PipelineLayout, nullptr);
                m_PipelineLayout = VK_NULL_HANDLE;
            }

            for (auto& info : m_SetLayouts)
            {
                if (info.Layout != VK_NULL_HANDLE)
                {
                    vkDestroyDescriptorSetLayout(GetDevice(), info.Layout, nullptr);
                    info.Layout = VK_NULL_HANDLE;
                }
            }
            m_SetLayouts.clear();m_PushConstantRanges.clear();
            m_ParamSlots.clear();m_Desc.Parameters.clear();
        }
    }

    bool RHIRootSignatureVulKan::IsValid() const
    {
        return GetDevice() != VK_NULL_HANDLE && m_PipelineLayout != VK_NULL_HANDLE;
    }
}
