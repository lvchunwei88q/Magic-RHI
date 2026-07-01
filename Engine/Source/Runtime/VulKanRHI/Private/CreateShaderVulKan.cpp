#include "RHIVulKan.h"
#include "RHIResourceVulKan.h"

namespace RHI
{
    namespace
    {
        ShaderModelVersion GetHighestSupportedShaderModel(VkPhysicalDevice physicalDevice)
        {
            // Get device properties
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(physicalDevice, &props);
            
            uint32_t apiVersion = props.apiVersion;
            uint32_t major = VK_API_VERSION_MAJOR(apiVersion);
            uint32_t minor = VK_API_VERSION_MINOR(apiVersion);

            VkPhysicalDeviceVulkan12Features features12 = {};
            features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
            
            VkPhysicalDeviceVulkan13Features features13 = {};
            features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
            features13.pNext = &features12;  // 1.3 → 1.2
            
            VkPhysicalDeviceFeatures2 features2 = {};
            features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            features2.pNext = &features13;   // 2 → 1.3 → 1.2
            
            vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);
            
            // Determine the Shader Model based on the API version and features
            if (major >= 1 && minor >= 4)
            {
                // Vulkan 1.4 Base = SM 6.8
                return ShaderModelVersion::SM_6_8;
            }
            else if (major >= 1 && minor >= 3)
            {
                if (features13.shaderDemoteToHelperInvocation)
                    return ShaderModelVersion::SM_6_7;
                
                if (features12.shaderFloat16 && features12.shaderInt8)
                    return ShaderModelVersion::SM_6_0;
                
                return ShaderModelVersion::SM_5_1;
            }
            else if (major >= 1 && minor >= 2)
            {
                if (features12.shaderFloat16 && features12.shaderInt8)
                    return ShaderModelVersion::SM_6_0;
                
                return ShaderModelVersion::SM_5_1;
            }
            else if (major >= 1 && minor >= 1)
                return ShaderModelVersion::SM_5_1;
            
            return ShaderModelVersion::SM_5_0;
        }

        template<typename ShaderType>
        std::unique_ptr<ShaderType> CompileShaderInternal(const CreateShaderDesc& desc)
        {
            // Check if the shader byte code is empty
            if(desc.GetUINT32ByteCode().size() == 0){
#if RHI_ENABLE_DEBUG_INFO
                ThrowErrorMessage("Shader byte code is empty");
#endif 
                return nullptr;
            }
            return std::make_unique<ShaderType>(desc.GetUINT32ByteCode());
        }
    }

    /**
     * ======================================================================
     * Create a shader using the given shader bytecode descriptor
     */
    std::unique_ptr<RHIVertexShader> DeviceVulKan::CreateVertexShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<VertexShaderVulKan>(desc);
    }

    std::unique_ptr<RHIPixelShader> DeviceVulKan::CreatePixelShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<PixelShaderVulKan>(desc);
    }

    std::unique_ptr<RHIGeometryShader> DeviceVulKan::CreateGeometryShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<GeometryShaderVulKan>(desc);
    }

    std::unique_ptr<RHIHullShader> DeviceVulKan::CreateHullShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<HullShaderVulKan>(desc);
    }

    std::unique_ptr<RHIDomainShader> DeviceVulKan::CreateDomainShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<DomainShaderVulKan>(desc);
    }

    std::unique_ptr<RHIComputeShader> DeviceVulKan::CreateComputeShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<ComputeShaderVulKan>(desc);
    }

    ShaderModelVersion DeviceVulKan::GetShaderModelVersion() const
    {
        return GetHighestSupportedShaderModel(m_PhysicalDevice);
    }
}
