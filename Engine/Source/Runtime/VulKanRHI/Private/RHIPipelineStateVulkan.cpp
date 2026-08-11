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

        // check vertex and pixel shader
        auto vsShader = SafeCast<VertexShaderVulKan>(desc.pVertexShader);
        auto psShader = SafeCast<PixelShaderVulKan>(desc.pPixelShader);
        if (!vsShader || !psShader)
        {
            ThrowErrorMessage("RHIPipelineStateVulkan::Initialize: VertexShader or PixelShader is not a VertexShaderVulKan or PixelShaderVulKan");
            return false;
        }

        Type = PipelineStateType::Graphics;
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

        Type = PipelineStateType::Compute;
        return false;
    }

    void RHIPipelineStateVulkan::Shutdown()
    {
    }

    bool RHIPipelineStateVulkan::IsValid() const
    {
        return false;
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
