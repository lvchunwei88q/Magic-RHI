#include <Common/Check.h>
#include <Common/RHIConfig.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include "RHICommandListVulKan.h"
#include "RHIVulKan.h"

namespace RHI
{
    namespace
    {
        VkFormat SwapChainFormat_Vulkan = VK_FORMAT_B8G8R8A8_SRGB; // Use sRGB format for correct color space
        VkColorSpaceKHR SwapChainColorSpace_Vulkan = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; // Use sRGB format for correct color space
        RHITextureFormat SwapChainFormat_RHI = RHITextureFormat::R8G8B8A8_SRGB; // Use sRGB format for correct color space
    }

    SwapChainVulKan::SwapChainVulKan()
    {
    }

    SwapChainVulKan::~SwapChainVulKan()
    {
        Shutdown();
    }

    // =========================================================================== Interface
    VkDevice SwapChainVulKan::GetDevice() const
    {
        if (m_pRHI->GetDevice() == nullptr)
        {
            ThrowErrorMessage("Vulkan device not created");
            return VK_NULL_HANDLE;
        }
        return *m_pRHI->GetDevice();
    }

    VkPhysicalDevice SwapChainVulKan::GetPhysicalDevice() const
    {
        if (m_pRHI->GetPhysicalDevice() == nullptr)
        {
            ThrowErrorMessage("Vulkan physical device not created");
            return VK_NULL_HANDLE;
        }
        return *m_pRHI->GetPhysicalDevice();
    }

    VkSurfaceKHR SwapChainVulKan::GetVulkanWindowSurface() const
    {
        if (m_pRHI->GetVulkanWindowSSurface() == nullptr)
        {
            ThrowErrorMessage("Vulkan surface not created");
            return VK_NULL_HANDLE;
        }
        return *m_pRHI->GetVulkanWindowSSurface();
    }

    // ===========================================================================

    bool SwapChainVulKan::CreateVulkanWindowSurface(HWND windowHandle)
    {
        return m_pRHI->CreateVulkanWindowSurface(windowHandle);
    }

    bool SwapChainVulKan::Initialize(Device* device, const SwapChainDesc& desc)
    {
        // Get the window handle from the device
        HWND windowHandle = static_cast<HWND>(desc.WindowHandleRef);

        m_pRHI = SafeCast<DeviceVulKan>(device);
        if (!m_pRHI)
        {
            ThrowErrorMessage("Invalid Vulkan device");
            return false;
        }

        // Create Vulkan window surface
        if (!CreateVulkanWindowSurface(windowHandle))
        {
            ThrowErrorMessage("Failed to create Vulkan window surface");
            return false;
        }

        m_desc = desc;
        if (!VkCreateSwapChain())
        {
            ThrowErrorMessage("Failed to create swap chain");
            return false;
        }

        if (!VkCreateRenderTargetViews())
        {
            ThrowErrorMessage("Failed to create render target views");
            return false;
        }

        if (!VkCreateSyncObjects())
        {
            ThrowErrorMessage("Failed to create sync objects");
            return false;
        }

        m_Initialization = InitialState::Initialize;
        return true;
    }

    void SwapChainVulKan::Shutdown()
    {
        m_Initialization = InitialState::Shutdown;

        if (m_ImageAvailableSemaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(GetDevice(), m_ImageAvailableSemaphore, nullptr);
            m_ImageAvailableSemaphore = VK_NULL_HANDLE;
        }

        // First clear the RTV, let the image memory be handled by the swap chain, and make sure to clear the RTV before deleting the swap chain.
        m_pRenderTargetViews.clear();
        // Reset
        m_pBackBuffers.clear();

        if (m_SwapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(GetDevice(), m_SwapChain, nullptr);
            m_SwapChain = VK_NULL_HANDLE;
        }
    }

    bool SwapChainVulKan::IsValid() const
    {
        return m_SwapChain != VK_NULL_HANDLE && m_Initialization == InitialState::Initialize;
    }

    uint32_t SwapChainVulKan::AcquireNextImage()
    {
        VkResult result = vkAcquireNextImageKHR(
            GetDevice(),
            m_SwapChain,
            UINT64_MAX,
            m_ImageAvailableSemaphore,
            VK_NULL_HANDLE,
            &m_AvailableSwapChainImageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            Resize(m_desc.Width, m_desc.Height);
            result = vkAcquireNextImageKHR(
                GetDevice(),
                m_SwapChain,
                UINT64_MAX,
                m_ImageAvailableSemaphore,
                VK_NULL_HANDLE,
                &m_AvailableSwapChainImageIndex
            );
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            ThrowErrorMessage("Failed to acquire swap chain image");
            return UINT32_MAX;
        }

        return m_AvailableSwapChainImageIndex;
    }

    void SwapChainVulKan::Resize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;
        // Wait for all commands to finish
        vkDeviceWaitIdle(GetDevice());

        m_desc.Width = width;
        m_desc.Height = height;

        // First clear the RTV, let the image memory be handled by the swap chain, and make sure to clear the RTV before deleting the swap chain.
        m_pRenderTargetViews.clear();
        // Reset
        m_pBackBuffers.clear();

        if (m_SwapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(GetDevice(), m_SwapChain, nullptr);
            m_SwapChain = VK_NULL_HANDLE;
        }

        VkCreateSwapChain();
        VkCreateRenderTargetViews();
    }

    RHIRenderTargetView* SwapChainVulKan::GetRenderTargetView(uint32_t index) const
    {
        if (index >= m_pRenderTargetViews.size())
            return nullptr;
        return m_pRenderTargetViews[index].get();
    }

    RHITextureFormat SwapChainVulKan::GetRenderTargetFormat() const
    {
        // Return the RHI texture format corresponding to the swap chain format
        return SwapChainFormat_RHI;
    }

    RHITexture* SwapChainVulKan::GetBackBuffer(uint32_t index) const
    {
        if (index >= m_pBackBuffers.size())
            return nullptr;
        return m_pBackBuffers[index].get();
    }

    GraphicsQueueSwapChainInfo SwapChainVulKan::GetGraphicsQueueSwapChainInfo() const 
    {
        GraphicsQueueSwapChainInfo Info;
        Info.imageIndex = m_AvailableSwapChainImageIndex;
        Info.SwapChain = (uint64_t)m_SwapChain;
        Info.Semaphore = (uint64_t)m_ImageAvailableSemaphore;
        Info.VSync = m_desc.VSync;
        return Info;
    }

    bool SwapChainVulKan::VkCreateSwapChain()
    {
        // Next, let's set up a safe and efficient configuration
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GetPhysicalDevice(), GetVulkanWindowSurface(), &capabilities);

        // Get surface formats list
        std::vector<VkSurfaceFormatKHR> formats;
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(GetPhysicalDevice(), GetVulkanWindowSurface(), &formatCount, nullptr);
        if (formatCount > 0)
        {
            formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(GetPhysicalDevice(), GetVulkanWindowSurface(), &formatCount, formats.data());
        }

        // Get present modes list
        std::vector<VkPresentModeKHR> presentModes;
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(GetPhysicalDevice(), GetVulkanWindowSurface(), &presentModeCount, nullptr);
        if (presentModeCount > 0)
        {
            presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(GetPhysicalDevice(), GetVulkanWindowSurface(), &presentModeCount, presentModes.data());
        }

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(presentModes);
        VkExtent2D extent = ChooseSwapExtent(capabilities);

        m_SwapChainFormat = surfaceFormat.format;
        m_SwapChainExtent = extent;

        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        {
            imageCount = capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.flags = 0;
        createInfo.surface = GetVulkanWindowSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t graphicsQueueFamilyIndex = m_pRHI->GetGraphicsQueueFamilyIndex();
        uint32_t computeQueueFamilyIndex = m_pRHI->GetComputeQueueFamilyIndex();
        uint32_t queueFamilyIndices[] = { graphicsQueueFamilyIndex, computeQueueFamilyIndex };

        // Check if they are the same queue use queue family index
        if (graphicsQueueFamilyIndex != computeQueueFamilyIndex)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VkResult result = vkCreateSwapchainKHR(GetDevice(), &createInfo, nullptr, &m_SwapChain);
        if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("Failed to create Vulkan swap chain");
            return false;
        }

        // The images in the swapchain are automatically managed by the Vulkan driver, so you don't need to, and actually shouldn't, call vkAllocateMemory to allocate their memory yourself.
        std::vector<VkImage> images;
        vkGetSwapchainImagesKHR(GetDevice(), m_SwapChain, &imageCount, nullptr);
        m_pBackBuffers.resize(imageCount);
        images.resize(imageCount);
        vkGetSwapchainImagesKHR(GetDevice(), m_SwapChain, &imageCount, images.data());

        m_pBackBuffers.resize(imageCount);
        for (size_t i = 0; i < imageCount; ++i)
        {   
            auto& image = images[i];
            TextureDesc desc = {};
            desc.Format = SwapChainFormat_RHI;
            desc.Width = m_SwapChainExtent.width;
            desc.Height = m_SwapChainExtent.height;
            desc.MipLevels = 1;
            desc.ArrayLayers = 1;
            desc.SampleCount = VK_SAMPLE_COUNT_1_BIT;
            m_pBackBuffers[i] = std::make_unique<SwapChainTextureVulKan>(image,VK_NULL_HANDLE,desc,m_pRHI->GetDevice());
        }

        return true;
    }

    bool SwapChainVulKan::VkCreateRenderTargetViews()
    {
        // Create render target views
        m_pRenderTargetViews.resize(m_pBackBuffers.size());
        std::vector<VkImageView> imageViews;
        imageViews.resize(m_pBackBuffers.size());

        for (size_t i = 0; i < m_pBackBuffers.size(); ++i)
        {
            // Create render target view
            SwapChainTextureVulKan* texture = SafeCast<SwapChainTextureVulKan>(m_pBackBuffers[i].get());
            if (texture == nullptr)
            {
                ThrowErrorMessage("Failed to cast texture");
                return false;
            }

            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = texture->GetImage();
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_SwapChainFormat;
            viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            VkResult result = vkCreateImageView(GetDevice(), &viewInfo, nullptr, &imageViews[i]);
            if (result != VK_SUCCESS)
            {
                ThrowErrorMessage("Failed to create render target view");
                return false;
            }
        }

        for (size_t i = 0; i < m_pRenderTargetViews.size(); ++i)
            m_pRenderTargetViews[i] = std::make_unique<RenderTargetViewVulKan>(imageViews[i], m_pRHI->GetDevice());
        return true;
    }

    bool SwapChainVulKan::VkCreateSyncObjects()
    {
        // Semaphore creation info
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        // Fence creation info
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkResult result = vkCreateSemaphore(GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore);
        if (result != VK_SUCCESS)
        {
            ThrowErrorMessage("Failed to create image available semaphore");
            return false;
        }

        return true;
    }

    VkSurfaceFormatKHR SwapChainVulKan::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        if (availableFormats.empty())
        {
            return { SwapChainFormat_Vulkan, SwapChainColorSpace_Vulkan };
        }

        // Prefer VK_FORMAT_B8G8R8A8_UNORM with VK_COLOR_SPACE_SRGB_NONLINEAR_KHR for correct color space
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == SwapChainFormat_Vulkan &&
                availableFormat.colorSpace == SwapChainColorSpace_Vulkan)
            {
                return availableFormat;
            }
        }

        // If not, return the first available one
        return availableFormats[0];
    }

    VkPresentModeKHR SwapChainVulKan::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        if (availablePresentModes.empty())
        {
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }

        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChainVulKan::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(m_desc.Width),
            static_cast<uint32_t>(m_desc.Height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}
