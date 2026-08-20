#include "RHIPipelineStateVulkan.h"
#include "RHIRootSignatureVulKan.h"
#include "RHIVulKan.h"

namespace RHI
{
    namespace
    {

    }

    RHIPipelineStateVulkan::RHIPipelineStateVulkan()
    {
    }
    RHIPipelineStateVulkan::~RHIPipelineStateVulkan()
    {
        Shutdown();
    }

    bool RHIPipelineStateVulkan::Initialize(Device* device, const GraphicsPipelineStateDesc& desc)
    {
        // check params
        if (!device || !desc.pRootSignature || !desc.pVertexShader || !desc.pPixelShader)
        {
            ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: Device, RootSignature, VertexShader, or PixelShader is null");
            return false;
        }

        // check vulkan device
        auto vulkanDevice = SafeCast<DeviceVulKan>(device);
        auto VKDevice = vulkanDevice->GetDevice();
        if (!VKDevice)
        {
            ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: Device is null");
            return false;
        }

        // check root signature
        auto rootSig = SafeCast<RHIRootSignatureVulKan>(desc.pRootSignature);
        if (!rootSig->IsValid())
        {
            ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: RootSignature is not a RHIRootSignatureVulKan");
            return false;
        }

        // check vertex shader
        auto vsShader = SafeCast<VertexShaderVulKan>(desc.pVertexShader);
        if (!vsShader)
        {
            ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: VertexShader is not a VertexShaderVulKan");
            return false;
        }

        // check vertex shader module
        VkShaderModule vertModule = vsShader->GetShaderModule();
        if (!vertModule)
        {
            ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: VertexShader module is null");
            return false;
        }

        VkPipelineShaderStageCreateInfo vertStageInfo{};
        vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertStageInfo.module = vertModule;
        vertStageInfo.pName = vsShader->GetEntryPoint().c_str(); 
        vertStageInfo.pSpecializationInfo = nullptr;

        // Create shader stages
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {vertStageInfo};

        auto psShader = SafeCast<PixelShaderVulKan>(desc.pPixelShader);
        if (psShader)
        {
            // check fragment shader module
            VkShaderModule fragModule = psShader->GetShaderModule();
            if (!fragModule)
            {
                ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: PixelShader module is null");
                return false;
            }
            VkPipelineShaderStageCreateInfo fragStageInfo{};
            fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragStageInfo.module = fragModule;
            fragStageInfo.pName = psShader->GetEntryPoint().c_str(); 
            fragStageInfo.pSpecializationInfo = nullptr;

            // add fragment shader stage to shader stages
            shaderStages.push_back(fragStageInfo);
        }

        auto gsShader = SafeCast<GeometryShaderVulKan>(desc.pGeometryShader);
        if (gsShader)
        {
            // check geometry shader module
            VkShaderModule gsModule = gsShader->GetShaderModule();
            if (!gsModule)
            {
                ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: GeometryShader module is null");
                return false;
            }
            VkPipelineShaderStageCreateInfo gsStageInfo{};
            gsStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            gsStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            gsStageInfo.module = gsModule;
            gsStageInfo.pName = gsShader->GetEntryPoint().c_str(); 
            gsStageInfo.pSpecializationInfo = nullptr;

            // add geometry shader stage to shader stages
            shaderStages.push_back(gsStageInfo);
        }

        auto hsShader = SafeCast<HullShaderVulKan>(desc.pHullShader);
        if (hsShader)
        {
            // check hull shader module
            VkShaderModule hsModule = hsShader->GetShaderModule();
            if (!hsModule)
            {
                ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: HullShader module is null");
                return false;
            }
            VkPipelineShaderStageCreateInfo hsStageInfo{};
            hsStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            hsStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            hsStageInfo.module = hsModule;
            hsStageInfo.pName = hsShader->GetEntryPoint().c_str(); 
            hsStageInfo.pSpecializationInfo = nullptr;

            // add hull shader stage to shader stages
            shaderStages.push_back(hsStageInfo);
        }

        auto dsShader = SafeCast<DomainShaderVulKan>(desc.pDomainShader);
        if (dsShader)
        {
            // check domain shader module
            VkShaderModule dsModule = dsShader->GetShaderModule();
            if (!dsModule)
            {
                ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: DomainShader module is null");
                return false;
            }
            VkPipelineShaderStageCreateInfo dsStageInfo{};
            dsStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            dsStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            dsStageInfo.module = dsModule;
            dsStageInfo.pName = dsShader->GetEntryPoint().c_str(); 
            dsStageInfo.pSpecializationInfo = nullptr;

            // add depth stencil shader stage to shader stages
            shaderStages.push_back(dsStageInfo);
        }

        Type = PipelineStateType::Graphics;
        m_Device = VKDevice;
        return false;
    }

    bool RHIPipelineStateVulkan::Initialize(Device* device, const ComputePipelineStateDesc& desc)
    {
        // check params
        if (!device || !desc.pRootSignature || !desc.pComputeShader)
        {
            ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: Device, RootSignature, or ComputeShader is null");
            return false;
        }

        // check vulkan device
        auto vulkanDevice = SafeCast<DeviceVulKan>(device);
        auto VKDevice = vulkanDevice->GetDevice();
        if (!VKDevice)
        {
            ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: Device is null");
            return false;
        }

        // check root signature
        auto rootSig = SafeCast<RHIRootSignatureVulKan>(desc.pRootSignature);
        if (!rootSig->IsValid())
        {
            ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: RootSignature is not a RHIRootSignatureVulKan");
            return false;
        }

        // check compute shader
        auto computeShader = SafeCast<ComputeShaderVulKan>(desc.pComputeShader);
        if (!computeShader)
        {
            ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: ComputeShader is not a ComputeShaderVulKan");
            return false;
        }

        // check compute shader module
        VkShaderModule computeModule = computeShader->GetShaderModule();
        if (!computeModule)
        {
            ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: ComputeShader module is null");
            return false;
        }

        VkPipelineShaderStageCreateInfo computeStageInfo{};
        computeStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        computeStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computeStageInfo.module = computeModule;
        computeStageInfo.pName = computeShader->GetEntryPoint().c_str(); 
        computeStageInfo.pSpecializationInfo = nullptr;

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 0;           // TODO: no descriptor sets
        layoutInfo.pushConstantRangeCount = 0;   // TODO: no push constants


        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.stage = computeStageInfo;
        pipelineInfo.layout = rootSig->GetPipelineLayout();    // pipeline Layout
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        VkPipeline computePipeline;
        if (vkCreateComputePipelines(
                *VKDevice, 
                nullptr,   // If it's nullptr, don't use the pipeline cache
                1,
                &pipelineInfo, 
                nullptr, 
                &computePipeline) != VK_SUCCESS) {
            // failed to create compute pipeline
            ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: Failed to create compute pipeline");
            return false;
        }

        m_Pipeline = computePipeline;
        Type = PipelineStateType::Compute;
        m_Device = VKDevice;
        return true;
    }

    void RHIPipelineStateVulkan::Shutdown()
    {
        if (m_Pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(*m_Device, m_Pipeline, nullptr);
            m_Pipeline = VK_NULL_HANDLE;
        }
        m_Device = nullptr;
        
        Type = PipelineStateType::Unknown;
    }

    bool RHIPipelineStateVulkan::IsValid() const
    {
        return m_Pipeline != VK_NULL_HANDLE && Type != PipelineStateType::Unknown;
    }

    PipelineStateType RHIPipelineStateVulkan::GetType() const
    {
        return Type;
    }

    // ============== DeviceVulKan ==============
    std::shared_ptr<RHIPipelineState> DeviceVulKan::CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
    {
        auto pipelineState = std::make_shared<RHIPipelineStateVulkan>();
        if (pipelineState->Initialize(this, desc))
        {
            return pipelineState;
        }
        return nullptr;
    }

    std::shared_ptr<RHIPipelineState> DeviceVulKan::CreateComputePipelineState(const ComputePipelineStateDesc& desc)
    {
        auto pipelineState = std::make_shared<RHIPipelineStateVulkan>();
        if (pipelineState->Initialize(this, desc))
        {
            return pipelineState;
        }
        return nullptr;
    }

    void DeviceVulKan::DeletePipelineState(std::shared_ptr<RHIPipelineState>& pipelineState)
    {
        if (pipelineState)
        {
            auto vulkanPipelineState = SafeCast<RHIPipelineStateVulkan>(pipelineState.get());
            if (vulkanPipelineState)
            {
                vulkanPipelineState->Shutdown();
            }
            pipelineState.reset();
        }
    }

} // namespace RHI
