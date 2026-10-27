#include "RHIPlatformDetection.h"

// For Windows, we can use the Windows API to check if D3D11, Vulkan, and D3D12 are available.
#if RHI_PLATFORM_SUPPORTED_WINDOWS
#include <windows.h>
#endif

/*
* We'll implement Windows detection here
**/
#if RHI_PLATFORM_SUPPORTED_WINDOWS 
static bool IsD3D12Available()
{
    static bool available = ([]() {
        HMODULE dll = LoadLibraryA("d3d12.dll");
        if (dll) FreeLibrary(dll);
        return dll != nullptr;
    })();
    return available;
}
static bool IsVulkanAvailable()
{
    static bool available = ([]() {
        HMODULE dll = LoadLibraryA("vulkan-1.dll");
        if (dll) FreeLibrary(dll);
        return dll != nullptr;
    })();
    return available;
}
#endif

/*
* We’re going to implement Linux detection here
**/
#if RHI_PLATFORM_SUPPORTED_LINUX
// Linux detection logic can be implemented here, for example, checking for Vulkan loader or other libraries
#endif

namespace RHI
{
    RHIType GetBestAvailableRHI()
    {
        RHIType bestRHI = RHIType::None;
        // Check for the availability of RHI types in order of preference
#if RHI_PLATFORM_SUPPORTED_WINDOWS // Windows platform
        if (IsD3D12Available()) bestRHI |= RHIType::D3D12;
#endif
        // Vulkan is cross-platform, so we can check for it on all platforms
        if (IsVulkanAvailable()) bestRHI |= RHIType::VulKan;

        // If no RHI is available, return Unknown
        if (bestRHI == RHIType::None) 
            bestRHI = RHIType::Unknown; 
        
        return bestRHI;
    }
}