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

        VkShaderModule CreateShaderModule (VkDevice device, const std::vector<uint32_t>& code) {
            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size () * sizeof (uint32_t);
            createInfo.pCode = code.data ();

            VkShaderModule shaderModule;
            if (vkCreateShaderModule (device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                throw std::runtime_error ("Failed to create shader module!");
            }
            return shaderModule;
        }

        template<typename ShaderType>
        std::unique_ptr<ShaderType> CompileShaderInternal(DeviceVulKan* device,const CreateShaderDesc& desc)
        {
            const VkDevice* Vk_device = device->GetDevice();
            // Check Vk_device
            if (!Vk_device) {
                ThrowErrorMessage ("CompileShaderInternal:Failed to get VK device!");
            }

            // Check if the shader byte code is empty
            if(desc.GetUINT32ByteCode().size() == 0){
#if RHI_ENABLE_DEBUG_INFO
                ThrowErrorMessage("Shader byte code is empty");
#endif 
                return nullptr;
            }

            VkShaderModule ShaderModule = CreateShaderModule(*Vk_device,desc.GetUINT32ByteCode());
            return std::make_unique<ShaderType>(Vk_device, ShaderModule,desc.GetUINT32ByteCode());
        }
    }

    /**
     * ======================================================================
     * Create a shader using the given shader bytecode descriptor
     */
    std::unique_ptr<RHIVertexShader> DeviceVulKan::CreateVertexShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<VertexShaderVulKan>(this, desc);
    }

    std::unique_ptr<RHIPixelShader> DeviceVulKan::CreatePixelShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<PixelShaderVulKan>(this, desc);
    }

    std::unique_ptr<RHIGeometryShader> DeviceVulKan::CreateGeometryShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<GeometryShaderVulKan>(this, desc);
    }

    std::unique_ptr<RHIHullShader> DeviceVulKan::CreateHullShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<HullShaderVulKan>(this, desc);
    }

    std::unique_ptr<RHIDomainShader> DeviceVulKan::CreateDomainShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<DomainShaderVulKan>(this, desc);
    }

    std::unique_ptr<RHIComputeShader> DeviceVulKan::CreateComputeShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<ComputeShaderVulKan>(this, desc);
    }

    ShaderModelVersion DeviceVulKan::GetShaderModelVersion() const
    {
        return GetHighestSupportedShaderModel(m_PhysicalDevice);
    }
}
