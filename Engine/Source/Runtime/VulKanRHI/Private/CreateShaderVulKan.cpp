#include "RHIVulKan.h"

namespace RHI
{
    namespace
    {
    }

    /**
     * ======================================================================
     * Create a shader using the given shader bytecode descriptor
     */
    std::unique_ptr<RHIVertexShader> DeviceVulKan::CreateVertexShader(const CreateShaderDesc& desc)
    {
        return nullptr;
    }

    std::unique_ptr<RHIPixelShader> DeviceVulKan::CreatePixelShader(const CreateShaderDesc& desc)
    {
        return nullptr;
    }

    std::unique_ptr<RHIGeometryShader> DeviceVulKan::CreateGeometryShader(const CreateShaderDesc& desc)
    {
        return nullptr;
    }

    std::unique_ptr<RHIHullShader> DeviceVulKan::CreateHullShader(const CreateShaderDesc& desc)
    {
        return nullptr;
    }

    std::unique_ptr<RHIDomainShader> DeviceVulKan::CreateDomainShader(const CreateShaderDesc& desc)
    {
        return nullptr;
    }

    std::unique_ptr<RHIComputeShader> DeviceVulKan::CreateComputeShader(const CreateShaderDesc& desc)
    {
        return nullptr;
    }
}
