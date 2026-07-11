#include <Common/Check.h>
#include <Common/RHIFeatureLevel.h>
#include "RHICommandListVulKan.h"
#include "RHIVulKan.h"
#include <IO.h>
#include <algorithm>
#include <set>

// SetEnvironmentVariable
#include <windows.h>

namespace RHI
{
    // ========== Local Required Features ==========
    struct LocalVkPhysicalDeviceFeatures {
        VkPhysicalDeviceFeatures2 GetPhysicalDeviceFeatures() {
            return features2;
        }
        VkPhysicalDeviceFeatures2 features2{};
        VkPhysicalDeviceShaderFloat16Int8Features float16Int8Features{};
        VkPhysicalDeviceTimelineSemaphoreFeatures timelineFeatures{};
        // You can add other extensions
    };

    struct LocalVulkanPhysicalDeviceInfo
    {
        VkPhysicalDevice device;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memoryProperties;
        VkPhysicalDeviceFeatures features;
        uint32_t graphicsFamily;
        uint32_t computeFamily;
        uint32_t transferFamily;
        uint32_t score; // Score for sorting
        bool isSoftware;
    };
    
    namespace
    {
        bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers)
        {
            // Get all available validation layers count
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            // Get all available validation layers
            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            // Check if all required validation layers are available
            for (const char* layerName : validationLayers)
            {
                bool layerFound = false;
                // Check if the layer is available
                for (const auto& layerProperties : availableLayers)
                {
                    if (strcmp(layerName, layerProperties.layerName) == 0)
                    {
                        layerFound = true;break;
                    }
                }
                // If the layer is not found, return false
                if (!layerFound)
                    return false;   
            }
            return true;
        }

        std::vector<const char*> GetRequiredValidationLayers()
        {
#ifdef RHI_ENABLE_DEBUG_INFO
            // Add validation layers for debug info
            return {
                "VK_LAYER_KHRONOS_validation"
            };
#else
            return {};
#endif
        }

        std::vector<const char*> GetRequiredExtensions(bool enableValidation)
        {
            // Get all available extensions count
            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
            if (extensionCount == 0)
            {
                Core::WarningCapture::Capture("No Vulkan instance extensions found!");
                return {};
            }

            // Get all available extensions
            std::vector<VkExtensionProperties> extensions(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

            // Helper function: Check if the extension exists
            auto isExtensionAvailable = [&](const char* extensionName) -> bool {
                for (const auto& ext : extensions)
                {
                    if (strcmp(ext.extensionName, extensionName) == 0)
                    {
                        return true;
                    }
                }
                return false;
            };

            // Build the list of needed extensions
            std::vector<const char*> requiredExtensions;
            // Add basic extension 
            requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
            requiredExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

            // Add debug utils extension
            if (enableValidation)
            {
                if (isExtensionAvailable(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
                {
                    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                }
                else
                {
                    Core::WarningCapture::Capture("VK_EXT_DEBUG_UTILS_EXTENSION_NAME is not available, debug utils disabled");
                }
            }

            return requiredExtensions;
        }

        LocalVkPhysicalDeviceFeatures GetVkPhysicalDeviceFeatures()
        {
            LocalVkPhysicalDeviceFeatures features{};
            
            // Initialize the main structure
            features.features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            
            // Core features (members of VkPhysicalDeviceFeatures)
            features.features2.features.geometryShader = VK_TRUE;
            features.features2.features.tessellationShader = VK_TRUE;
            features.features2.features.shaderStorageImageMultisample = VK_TRUE;
            features.features2.features.shaderStorageImageReadWithoutFormat = VK_TRUE;
            features.features2.features.shaderStorageImageWriteWithoutFormat = VK_TRUE;
            
            // Extension features ...
            features.float16Int8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES;
            features.float16Int8Features.shaderFloat16 = VK_FALSE;
            features.float16Int8Features.shaderInt8 = VK_FALSE;
            features.features2.pNext = &features.float16Int8Features;

            features.timelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
            features.timelineFeatures.timelineSemaphore = VK_TRUE;
            features.float16Int8Features.pNext = &features.timelineFeatures;
            
            return features;
        }

        bool IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice, uint32_t& graphicsFamily, uint32_t& computeFamily, uint32_t& transferFamily)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
            // Check if the device is a discrete GPU or an integrated GPU
            if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {
                // If the device is not a discrete GPU or an integrated GPU, return false
                return false;
            }
            
            // Get queue family count
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

            // Set default values for queue families
            graphicsFamily = UINT32_MAX;
            computeFamily = UINT32_MAX;
            transferFamily = UINT32_MAX;

            for (uint32_t i = 0; i < queueFamilyCount; ++i)
            {
                const auto& queueFamily = queueFamilies[i];

                // if the queue family supports graphics queue
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    graphicsFamily = i;
                    if (computeFamily == UINT32_MAX) computeFamily = i;
                    if (transferFamily == UINT32_MAX) transferFamily = i;
                }

                // if the queue family supports compute queue
                if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) && !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
                {
                    computeFamily = i;
                }

                // if the queue family supports transfer queue
                if ((queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) && 
                    !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && 
                    !(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
                {
                    transferFamily = i;
                }
            }

            // Check if all required queue families are found
            return graphicsFamily != UINT32_MAX && computeFamily != UINT32_MAX && transferFamily != UINT32_MAX;
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

        // ============================================================
        // Convert VkPhysicalDeviceType to a string
        // ============================================================
        const char* VkPhysicalDeviceTypeToString(VkPhysicalDeviceType type)
        {
            switch (type)
            {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:          return "Other";
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return "Discrete GPU";
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    return "Virtual GPU";
            case VK_PHYSICAL_DEVICE_TYPE_CPU:            return "CPU";
            case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:       return "Max Enum";
            default:                                     return "Unknown";
            }
        }

        uint32_t CalculatePhysicalDeviceScore(
            VkPhysicalDevice physicalDevice,
            const VkPhysicalDeviceProperties& properties,
            const VkPhysicalDeviceFeatures& features,
            const VkPhysicalDeviceMemoryProperties& memoryProperties)
        {
            uint32_t score = 0;

            // ============================================================
            // Device Type (Most Important)
            // ============================================================
            switch (properties.deviceType)
            {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                score += 1000;  // Dedicated GPU: Highest priority
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                score += 500;   // Integrated GPU: Medium priority
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                score += 200;   // Virtual GPU: Low priority
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                score += 50;    // CPU Software Simulation: Lowest priority
                break;
            default:
                score += 0;     // Unknown type
                break;
            }

            // ============================================================
            // VRAM Size (Calculated via Memory Heap)
            // ============================================================
            VkDeviceSize totalVideoMemory = 0;
            for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i)
            {
                if (memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                {
                    totalVideoMemory += memoryProperties.memoryHeaps[i].size;
                }
            }

            // VRAM Score (in GB)
            uint32_t vramGB = static_cast<uint32_t>(totalVideoMemory / (1024ULL * 1024ULL * 1024ULL));
            if (vramGB >= 16) score += 160;      // 16GB+ 
            else if (vramGB >= 12) score += 120; // 12GB+
            else if (vramGB >= 8) score += 80;   // 8GB+
            else if (vramGB >= 6) score += 60;   // 6GB+
            else if (vramGB >= 4) score += 40;   // 4GB+
            else if (vramGB >= 2) score += 20;   // 2GB+
            else score += 10;                    // <2GB

            // ============================================================
            // API Version
            // ============================================================
            uint32_t major = VK_VERSION_MAJOR(properties.apiVersion);
            uint32_t minor = VK_VERSION_MINOR(properties.apiVersion);

            if (major >= 1)
            {
                if (minor >= 4) score += 140;      // Vulkan 1.4
                else if (minor >= 3) score += 130; // Vulkan 1.3
                else if (minor >= 2) score += 120; // Vulkan 1.2
                else if (minor >= 1) score += 110; // Vulkan 1.1
                else score += 100;                 // Vulkan 1.0
            }

            return score;
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
        m_Initialization = InitialState::Initialize;

		// Set the environment variable to specify the path for Vulkan layers
		std::wstring layerPath = IO::AbsolutePath::Get().GetExecutableDirectory();
        if (!SetEnvironmentVariableW(L"VK_ADD_LAYER_PATH", layerPath.c_str()))
        {
            ThrowErrorMessage("Failed to set environment variable for Vulkan layers");
            return false;
        }

        // Create Vulkan instance 
        if (!CreateInstance())
        {
            ThrowErrorMessage("Failed to create Vulkan instance");
            return false;
        }

        // Pick physical device
        if (!PickPhysicalDevice())
        {
            ThrowErrorMessage("Failed to pick physical device");
            return false;
        }

        // Create logical device
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
#ifdef RHI_ENABLE_DEBUG_INFO
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
        auto validationLayers = GetRequiredValidationLayers();

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (enableValidation)
        {
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
            SdkVkApiVersion = apiVersion;
        }else{
            Core::ErrorCapture::Capture("Failed to enumerate instance version");
        }

        return true;
    }

    bool DeviceVulKan::PickPhysicalDevice()
    {
        // Get physical device count
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

        // Check if any physical device is available
        if (deviceCount == 0)
        {
            ThrowErrorMessage("No Vulkan compatible GPUs found");
            return false;
        }

        // Enumerate physical devices
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());
        
        std::vector<LocalVulkanPhysicalDeviceInfo> candidates;

        // Check each physical device for suitability
        for (VkPhysicalDevice device : devices)
        {
            uint32_t graphicsFamily, computeFamily, transferFamily;
            bool isSoftware = !IsPhysicalDeviceSuitable(device, graphicsFamily, computeFamily, transferFamily);
            
            // Get physical device properties
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            // Get physical device memory properties
            VkPhysicalDeviceMemoryProperties memoryProperties;
            vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

            // Get physical device features
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(device, &features);

            // Calculate scores: discrete GPU priority, VRAM size, feature support
            uint32_t score = CalculatePhysicalDeviceScore(device, properties, features, memoryProperties);

            // add candidate to list
            LocalVulkanPhysicalDeviceInfo candidate;
            candidate.device = device;
            candidate.memoryProperties = memoryProperties;
            candidate.graphicsFamily = graphicsFamily;
            candidate.computeFamily = computeFamily;
            candidate.transferFamily = transferFamily;
            candidate.features = features;
            candidate.properties = properties;
            candidate.score = score;
            candidate.isSoftware = isSoftware;
            candidates.push_back(candidate);
        }

        // IF no suitable GPU found, throw error
        if (candidates.empty())
        {
            ThrowErrorMessage("No suitable GPU found");
            return false;
        }
        
        // Sort candidates by score in descending order
        std::sort(candidates.begin(), candidates.end(), [](const LocalVulkanPhysicalDeviceInfo& a, const LocalVulkanPhysicalDeviceInfo& b)
        {
            return a.score > b.score;
        });
        
        // Pick the first candidate as the physical device
        m_PhysicalDevice = candidates[0].device;
        /* Each GPU might have different types of queue families.
         * So we need to first get the queue family index for each type of queue, and then when creating the device,
         * we tell the driver which queue family we want to allocate the queue from using the index we got.
         * Finally, we get the queue handle using vkGetDeviceQueue.
        */
        m_GraphicsQueueFamilyIndex = candidates[0].graphicsFamily;
        m_ComputeQueueFamilyIndex = candidates[0].computeFamily;
        m_CopyQueueFamilyIndex = candidates[0].transferFamily;
        // Convert to wide string
        m_AdapterName = IO::ToWideString(candidates[0].properties.deviceName);
        PhysicalDeviceApiVersion = candidates[0].properties.apiVersion;

#if RHI_ENABLE_DEBUG_INFO
        // If in debug mode, output detailed information
        char debugBuffer[512];
        const auto& best = candidates[0];
        sprintf_s(debugBuffer,
            "[Vulkan] Selected Device:\n"
            "  Name: %s\n"
            "  Type: %s\n"
            "  API Version: %d.%d.%d\n"
            "  Score: %u\n"
            "  VRAM: %.2f GB\n"
            "  Graphics Queue: %u\n"
            "  Compute Queue: %u\n"
            "  Transfer Queue: %u\n",
            best.properties.deviceName,
            VkPhysicalDeviceTypeToString(best.properties.deviceType),
            VK_VERSION_MAJOR(PhysicalDeviceApiVersion),
            VK_VERSION_MINOR(PhysicalDeviceApiVersion),
            VK_VERSION_PATCH(PhysicalDeviceApiVersion),
            best.score,
            (double)best.memoryProperties.memoryHeaps[0].size / (1024.0 * 1024.0 * 1024.0),
            m_GraphicsQueueFamilyIndex,
            m_ComputeQueueFamilyIndex,
            m_CopyQueueFamilyIndex
        );
        Core::InfoCapture::Capture(debugBuffer);
#endif

        candidates.clear();
        return true;
    }

    bool DeviceVulKan::CreateLogicalDevice()
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        // Get unique queue families
        std::set<uint32_t> uniqueQueueFamilies = {
            m_GraphicsQueueFamilyIndex,
            m_ComputeQueueFamilyIndex,
            m_CopyQueueFamilyIndex
        };

        // This is the queue's execution priority, 1.0f is the highest priority.
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

        // Get required features
        LocalVkPhysicalDeviceFeatures deviceFeatures = GetVkPhysicalDeviceFeatures();
		VkPhysicalDeviceFeatures2 features2 = deviceFeatures.GetPhysicalDeviceFeatures();

        // Set device extensions
        std::vector<const char*> deviceExtensions = {
            // Swapchain extension (must) 
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &features2;
        createInfo.enabledLayerCount = 0;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        VkResult result = vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device);
        if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("Failed to create logical device");
            return false;
        }

        return true;
    }

    void DeviceVulKan::CreateQueues()
    {
        // Get queue handles
        VkQueue graphicsQueue, computeQueue, copyQueue;
        vkGetDeviceQueue(m_Device, m_GraphicsQueueFamilyIndex, 0, &graphicsQueue);
        vkGetDeviceQueue(m_Device, m_ComputeQueueFamilyIndex, 0, &computeQueue);
        vkGetDeviceQueue(m_Device, m_CopyQueueFamilyIndex, 0, &copyQueue);
        
        // Create queues
        m_GraphicsQueue = std::make_unique<GraphicsCommandQueueVulKan>(
            RHICmdType::Graphics, 
            graphicsQueue,
            m_GraphicsQueueFamilyIndex,
            GetDevice(),
            GetPhysicalDevice()
        );

        m_ComputeQueue = std::make_unique<ComputeCommandQueueVulKan>(
            RHICmdType::Compute, 
            computeQueue,
            m_ComputeQueueFamilyIndex,
            GetDevice(),
            GetPhysicalDevice()
        );

        m_CopyQueue = std::make_unique<CopyCommandQueueVulKan>(
            RHICmdType::Copy, 
            copyQueue,
            m_CopyQueueFamilyIndex,
            GetDevice(),
            GetPhysicalDevice()
        );
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
        uint32_t standardPoolSizeCount = sizeof(standardPoolSizes) / sizeof(standardPoolSizes[0]);
        standardPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        standardPoolInfo.poolSizeCount = standardPoolSizeCount;
        standardPoolInfo.pPoolSizes = standardPoolSizes;
        standardPoolInfo.maxSets = RHI_DESCRIPTOR_HEAP_SIZE_STANDARD;  // Each type can be assigned up to RHI_DESCRIPTOR_HEAP_SIZE_STANDARD.

        m_pStandardHeap = std::make_unique<DescriptorHeapVulKan>(
            GetDevice(),
            RHIDescriptorHeapType::Standard,
            standardPoolInfo
        );

        // Sampler Pool - Specifically for samplers
        VkDescriptorPoolSize samplerPoolSizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER }
        };

        VkDescriptorPoolCreateInfo samplerPoolInfo = {};
        samplerPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        samplerPoolInfo.poolSizeCount = sizeof(samplerPoolSizes) / sizeof(samplerPoolSizes[0]);
        samplerPoolInfo.pPoolSizes = samplerPoolSizes;
        samplerPoolInfo.maxSets = RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER;  // Each type can be assigned up to RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER.

        m_pSamplerHeap = std::make_unique<DescriptorHeapVulKan>(
            GetDevice(),
            RHIDescriptorHeapType::Sampler,
            samplerPoolInfo
        );

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

        m_pRTVHeap = std::make_unique<DescriptorHeapVulKan>(
            GetDevice(),
            RHIDescriptorHeapType::RenderTarget,
            rtvPoolInfo
        );

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

        m_pDSVHeap = std::make_unique<DescriptorHeapVulKan>(
            GetDevice(),
            RHIDescriptorHeapType::DepthStencil,
            dsvPoolInfo
        );

        return true;
    }

    bool DeviceVulKan::CreateVulkanWindowSurface(HWND windowHandle)
    {
        if (!m_Instance)
        {
            ThrowErrorMessage("Vulkan instance not created");
            return false;
        }

        VkWin32SurfaceCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = windowHandle;
        createInfo.hinstance = GetModuleHandle(nullptr);

        VkResult result = vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface);
        if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("Failed to create Vulkan surface");
            return false;
        }

        return true;
    }

    void DeviceVulKan::Shutdown()
    {
        m_Initialization = InitialState::Shutdown;
        m_pDSVHeap.reset();
        m_pRTVHeap.reset();
        m_pSamplerHeap.reset();
        m_pStandardHeap.reset();

        m_CopyQueue.reset();
        m_ComputeQueue.reset();
        m_GraphicsQueue.reset();

        if (m_Surface && m_Instance)
        {
            vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
            m_Surface = nullptr;
        }

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
        return m_Device != nullptr && m_PhysicalDevice != nullptr && m_Instance != nullptr
                                                     && m_Initialization == InitialState::Initialize;
    }

    FeatureLevel DeviceVulKan::GetFeatureLevel() const
    {
        return FromVulkanVersionLevel(PhysicalDeviceApiVersion);
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
        case RHICmdType::Graphics:
            return m_GraphicsQueue.get();
        case RHICmdType::Compute:
            return m_ComputeQueue.get();
        case RHICmdType::Copy:
            return m_CopyQueue.get();
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
