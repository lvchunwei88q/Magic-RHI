#include <Common/Check.h>
#include <Common/RHIFeatureLevel.h>
#include "RHIVulKan.h"
#include <set>

namespace RHI
{
    // ========== Local Required Features ==========
    struct LocalRequiredFeatures {
        VkPhysicalDeviceFeatures2 features2;
        VkPhysicalDeviceShaderFloat16Int8Features float16Int8Features;
        // You can add other extensions
    };
    
    namespace
    {
        bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers)
        {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const char* layerName : validationLayers)
            {
                bool layerFound = false;

                for (const auto& layerProperties : availableLayers)
                {
                    if (strcmp(layerName, layerProperties.layerName) == 0)
                    {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound)
                {
                    return false;
                }
            }

            return true;
        }

        std::vector<const char*> GetRequiredValidationLayers()
        {
#ifdef _DEBUG
            return {
                "VK_LAYER_KHRONOS_validation"
            };
#else
            return {};
#endif
        }

        std::vector<const char*> GetRequiredExtensions(bool enableValidation)
        {
            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> extensions(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

            std::vector<const char*> requiredExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };

            if (enableValidation)
            {
                requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return requiredExtensions;
        }

        LocalRequiredFeatures GetRequiredFeatures()
        {
            LocalRequiredFeatures result{};
            
            // Initialize the main structure
            result.features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            result.features2.pNext = &result.float16Int8Features;
            
            // Core features (members of VkPhysicalDeviceFeatures)
            result.features2.features.geometryShader = VK_TRUE;
            result.features2.features.tessellationShader = VK_TRUE;
            result.features2.features.shaderStorageImageMultisample = VK_TRUE;
            result.features2.features.shaderStorageImageReadWithoutFormat = VK_TRUE;
            result.features2.features.shaderStorageImageWriteWithoutFormat = VK_TRUE;
            
            // Extension features ... 
            result.float16Int8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES;
            result.float16Int8Features.shaderFloat16 = VK_TRUE;
            result.float16Int8Features.shaderInt8 = VK_TRUE;
            
            return result;
        }

        bool IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice, uint32_t& graphicsFamily, uint32_t& computeFamily, uint32_t& transferFamily)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

            if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {
                return false;
            }

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

            graphicsFamily = UINT32_MAX;
            computeFamily = UINT32_MAX;
            transferFamily = UINT32_MAX;

            for (uint32_t i = 0; i < queueFamilyCount; ++i)
            {
                const auto& queueFamily = queueFamilies[i];

                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    graphicsFamily = i;
                    if (computeFamily == UINT32_MAX) computeFamily = i;
                    if (transferFamily == UINT32_MAX) transferFamily = i;
                }

                if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) && !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
                {
                    computeFamily = i;
                }

                if ((queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) && 
                    !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && 
                    !(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
                {
                    transferFamily = i;
                }
            }

            return graphicsFamily != UINT32_MAX;
        }

        std::wstring GetPhysicalDeviceName(VkPhysicalDevice physicalDevice)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
            
            std::string name(deviceProperties.deviceName);
            return std::wstring(name.begin(), name.end());
        }

        FeatureLevel FromVulkanVersionLevel(uint32_t apiVersion)
        {
            uint32_t major = VK_API_VERSION_MAJOR(apiVersion);
            uint32_t minor = VK_API_VERSION_MINOR(apiVersion);

            if (major == 1)
            {
                switch (minor)
                {
                case 0: return FeatureLevel::Vulkan_1_0;
                case 1: return FeatureLevel::Vulkan_1_1;
                case 2: return FeatureLevel::Vulkan_1_2;
                case 3: return FeatureLevel::Vulkan_1_3;
                case 4: return FeatureLevel::Vulkan_1_4;
                default:
                    if (minor > 4) {
                        return FeatureLevel::Vulkan_1_4;
                    }
                    return FeatureLevel::Vulkan_1_0;  // default value
                }
            }else{
                // more ...
            }
            
            return FeatureLevel::Vulkan_1_0;  // default value
        }
    }

    DeviceVulKan::DeviceVulKan()
    {
    }
    
    DeviceVulKan::~DeviceVulKan()
    {
        Shutdown();
    }

    bool DeviceVulKan::Initialize()
    {
        if (!CreateInstance())
        {
            ThrowErrorMessage("Failed to create Vulkan instance");
            return false;
        }

        if (!PickPhysicalDevice())
        {
            ThrowErrorMessage("Failed to pick physical device");
            return false;
        }

        if (!CreateLogicalDevice())
        {
            ThrowErrorMessage("Failed to create logical device");
            return false;
        }

        CreateQueues();

        if (!CreateDescriptorPools())
        {
            ThrowErrorMessage("Failed to create descriptor pools");
            return false;
        }

        return true;
    }

    bool DeviceVulKan::CreateInstance()
    {
        bool enableValidation = false;
#ifdef _DEBUG
        enableValidation = true;
#endif

        if (enableValidation && !CheckValidationLayerSupport(GetRequiredValidationLayers()))
        {
            ThrowErrorMessage("Validation layers requested but not available");
            return false;
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "DirectXRHI Vulkan";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "DirectXRHI";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = GetRequiredExtensions(enableValidation);
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (enableValidation)
        {
            auto validationLayers = GetRequiredValidationLayers();
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }

        VkResult instance_result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
        if (instance_result != VK_SUCCESS)
        {
            ThrowErrorMessage("Failed to create Vulkan instance");
            return false;
        }

        uint32_t apiVersion = 0;
        VkResult api_version_result = vkEnumerateInstanceVersion(&apiVersion);
        if (api_version_result == VK_SUCCESS) {
            // apiVersion now includes the Vulkan version number supported by the instance
            ApiVersion = apiVersion;
        }else{
            Core::ErrorCapture::Capture("Failed to enumerate instance version");
        }

        return true;
    }

    bool DeviceVulKan::PickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            ThrowErrorMessage("No Vulkan compatible GPUs found");
            return false;
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

        for (VkPhysicalDevice device : devices)
        {
            uint32_t graphicsFamily, computeFamily, transferFamily;
            if (IsPhysicalDeviceSuitable(device, graphicsFamily, computeFamily, transferFamily))
            {
                m_PhysicalDevice = device;
                m_GraphicsQueueFamilyIndex = graphicsFamily;
                m_ComputeQueueFamilyIndex = computeFamily;
                m_CopyQueueFamilyIndex = transferFamily;
                m_AdapterName = GetPhysicalDeviceName(device);
                return true;
            }
        }

        ThrowErrorMessage("No suitable GPU found");
        return false;
    }

    bool DeviceVulKan::CreateLogicalDevice()
    {
        /*
         * "每个GPU对应的每个种类的队列簇是不同的，
         * 所以我们需要先获取每个队列的队列簇索引，然后在创建设备时设置 queueFamilyIndex 告诉驱动我要从哪个队列簇分配队列，
         * 最后通过 vkGetDeviceQueue 获取队列的句柄。"
        */
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            m_GraphicsQueueFamilyIndex,
            m_ComputeQueueFamilyIndex,
            m_CopyQueueFamilyIndex
        };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        LocalRequiredFeatures deviceFeatures = GetRequiredFeatures();

        std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &deviceFeatures.features2; 

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        bool enableValidation = false;
#ifdef _DEBUG
        enableValidation = true;
#endif

        if (enableValidation)
        {
            auto validationLayers = GetRequiredValidationLayers();
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }

        VkResult result = vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device);
        if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("Failed to create logical device");
            return false;
        }

        // vkGetDeviceQueue(m_Device, m_GraphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
        // vkGetDeviceQueue(m_Device, m_ComputeQueueFamilyIndex, 0, &m_ComputeQueue);
        // vkGetDeviceQueue(m_Device, m_CopyQueueFamilyIndex, 0, &m_CopyQueue);

        return true;
    }

    void DeviceVulKan::CreateQueues()
    {
        // m_GraphicsQueueWrapper = std::make_unique<CommandQueueVulKan>(
            // RHICmdType::Graphics, 
            // m_GraphicsQueue,
            // m_GraphicsQueueFamilyIndex,
            // m_Device
        // );

        // m_ComputeQueueWrapper = std::make_unique<CommandQueueVulKan>(
            // RHICmdType::Compute, 
            // m_ComputeQueue,
            // m_ComputeQueueFamilyIndex,
            // m_Device
        // );

        // m_CopyQueueWrapper = std::make_unique<CommandQueueVulKan>(
            // RHICmdType::Copy, 
            // m_CopyQueue,
            // m_CopyQueueFamilyIndex,
            // m_Device
        // );
    }

    bool DeviceVulKan::CreateDescriptorPools()
    {
        // Standard Pool - Used for most descriptor types
        VkDescriptorPoolSize standardPoolSizes[] = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, RHI_DESCRIPTOR_HEAP_SIZE_STANDARD },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, RHI_DESCRIPTOR_HEAP_SIZE_STANDARD },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, RHI_DESCRIPTOR_HEAP_SIZE_STANDARD },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, RHI_DESCRIPTOR_HEAP_SIZE_STANDARD },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, RHI_DESCRIPTOR_HEAP_SIZE_STANDARD }
        };

        VkDescriptorPoolCreateInfo standardPoolInfo = {};
        standardPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        standardPoolInfo.poolSizeCount = sizeof(standardPoolSizes) / sizeof(standardPoolSizes[0]);
        standardPoolInfo.pPoolSizes = standardPoolSizes;
        standardPoolInfo.maxSets = RHI_DESCRIPTOR_HEAP_SIZE_STANDARD * 5;  // 5 types, each can allocate up to RHI_DESCRIPTOR_HEAP_SIZE_STANDARD

        //VkResult result = vkCreateDescriptorPool(m_device, &standardPoolInfo, nullptr, &m_standardDescriptorPool);
        //if (result != VK_SUCCESS) {
        //    return false;
        //}

        // Sampler Pool - Specifically for samplers
        VkDescriptorPoolSize samplerPoolSizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER }
        };

        VkDescriptorPoolCreateInfo samplerPoolInfo = {};
        samplerPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        samplerPoolInfo.poolSizeCount = sizeof(samplerPoolSizes) / sizeof(samplerPoolSizes[0]);
        samplerPoolInfo.pPoolSizes = samplerPoolSizes;
        samplerPoolInfo.maxSets = RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER;  // Each type can be assigned up to RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER.

        //result = vkCreateDescriptorPool(m_device, &samplerPoolInfo, nullptr, &m_samplerDescriptorPool);
        //if (result != VK_SUCCESS) {
        //    return false;
        //}

        // RTV Pool - Used for render targets (using STORAGE_IMAGE or COMBINED_IMAGE_SAMPLER)
        VkDescriptorPoolSize rtvPoolSizes[] = {
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, RHI_DESCRIPTOR_HEAP_SIZE_RENDER_TARGET }
            // Or if you need to render the target as a texture sample, use:
            // { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, RHI_DESCRIPTOR_HEAP_SIZE_RENDER_TARGET }
        };

        VkDescriptorPoolCreateInfo rtvPoolInfo = {};
        rtvPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        rtvPoolInfo.poolSizeCount = sizeof(rtvPoolSizes) / sizeof(rtvPoolSizes[0]);
        rtvPoolInfo.pPoolSizes = rtvPoolSizes;
        rtvPoolInfo.maxSets = RHI_DESCRIPTOR_HEAP_SIZE_RENDER_TARGET;

        //result = vkCreateDescriptorPool(m_device, &rtvPoolInfo, nullptr, &m_rtvDescriptorPool);
        //if (result != VK_SUCCESS) {
        //    return false;
        //}

        // DSV Pool - Deep Template Texture (using COMBINED_IMAGE_SAMPLER or STORAGE_IMAGE)
        VkDescriptorPoolSize dsvPoolSizes[] = {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, RHI_DESCRIPTOR_HEAP_SIZE_DEPTH_STENCIL }
            // Or if you only need read-only texture access:
            // { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, RHI_DESCRIPTOR_HEAP_SIZE_DEPTH_STENCIL }
        };

        VkDescriptorPoolCreateInfo dsvPoolInfo = {};
        dsvPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        dsvPoolInfo.poolSizeCount = sizeof(dsvPoolSizes) / sizeof(dsvPoolSizes[0]);
        dsvPoolInfo.pPoolSizes = dsvPoolSizes;
        dsvPoolInfo.maxSets = RHI_DESCRIPTOR_HEAP_SIZE_DEPTH_STENCIL;// Each type can be assigned up to RHI_DESCRIPTOR_HEAP_SIZE_DEPTH_STENCIL.

        //result = vkCreateDescriptorPool(m_device, &dsvPoolInfo, nullptr, &m_dsvDescriptorPool);
        //if (result != VK_SUCCESS) {
        //    return false;
        //}

        return true;
    }

    void DeviceVulKan::Shutdown()
    {
        m_pDSVHeap.reset();
        m_pRTVHeap.reset();
        m_pSamplerHeap.reset();
        m_pStandardHeap.reset();

        // m_CopyQueueWrapper.reset();
        // m_ComputeQueueWrapper.reset();
        // m_GraphicsQueueWrapper.reset();

        if (m_Device)
        {
            vkDeviceWaitIdle(m_Device);
            vkDestroyDevice(m_Device, nullptr);
            m_Device = nullptr;
        }

        if (m_Instance)
        {
            vkDestroyInstance(m_Instance, nullptr);
            m_Instance = nullptr;
        }
    }

    bool DeviceVulKan::IsValid() const
    {
        return m_Device != nullptr && m_PhysicalDevice != nullptr && m_Instance != nullptr;
    }

    FeatureLevel DeviceVulKan::GetFeatureLevel() const
    {
        return FromVulkanVersionLevel(ApiVersion);
    }

    std::shared_ptr<RHIPipelineState> DeviceVulKan::CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
    {
        return nullptr;
    }

    std::shared_ptr<RHIPipelineState> DeviceVulKan::CreateComputePipelineState(const ComputePipelineStateDesc& desc)
    {
        return nullptr;
    }

    void DeviceVulKan::DeletePipelineState(std::shared_ptr<RHIPipelineState>& pipelineState)
    {
    }

    RHICommandQueue* DeviceVulKan::GetCommandQueue(RHICmdType Type) const
    {
        switch (Type)
        {
        default:
            return nullptr;
        }
    }

    RHIDescriptorHeap* DeviceVulKan::GetDescriptorHeap(RHIDescriptorHeapType type) const
    {
        switch (type)
        {
        case RHIDescriptorHeapType::Standard:
            return m_pStandardHeap.get();
        case RHIDescriptorHeapType::Sampler:
            return m_pSamplerHeap.get();
        case RHIDescriptorHeapType::RenderTarget:
            return m_pRTVHeap.get();
        case RHIDescriptorHeapType::DepthStencil:
            return m_pDSVHeap.get();
        default:
            return nullptr;
        }
    }
}
