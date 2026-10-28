#pragma once
// In the early stages of RHI, I will initially use Windows as the development platform,
// but ensure that it can be extended to other systems later on.
#include <cstdint>
#include <string>
#include "Common/RHI_API.h"
#include "Tools/EnumClassFlags.h"

// Platform Check
#if defined(_WIN32) || defined(_WIN64)
    #define RHI_PLATFORM_SUPPORTED_WINDOWS 1
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    #define RHI_PLATFORM_SUPPORTED_LINUX 1
#else
    #error "Unsupported platform for RHI. "
#endif

namespace RHI
{
    /**
     * @brief RHI Type Enum.
     */
    enum class RHIType : uint8_t
    {
        None = 0,
        D3D12   = 1 <<  1,      // DirectX 12
        VulKan  = 1 <<  2,      // Vulkan
        // More RHI types ...

        Unknown = 0xFF,      // Unknown
    };
    ENUM_CLASS_FLAGS(RHIType);

    /**
     * @brief Utility function to convert RHIType to string representation
     * @param type
     * @return
     */
    inline std::string RHITypeToString(RHIType type)
    {
        // If no RHI is available, return "RHIType: None"
        if (type == RHIType::None) return "RHIType: None";
        
        std::string result = "RHIType: ";

        // Check each RHI type and append its string representation if it is set in the type bitmask
        if (EnumHasAnyFlags(type , RHIType::D3D12))  result += "D3D12 | ";
        if (EnumHasAnyFlags(type , RHIType::VulKan)) result += "VulKan | ";

        if (result.length() > 10) // "RHIType: " is 10 characters long, so we check if there's anything after it
        {
            // Remove the trailing " | " from the result string
            result.erase(result.length() - 3);
        }

        return result.c_str();
    }

    /**
     * @brief Function to get the best available RHI type based on platform support
     * @return
     */
    RHIType RHI_API GetBestAvailableRHI();
    
    /**
     * @brief
     * @param type
     * @return
     */
    inline bool IsMultiThreadingSupported(RHIType type) {
        if(EnumHasAnyFlags(type, RHIType::D3D12 | RHIType::VulKan)) {
            return true; // Modern APIs with native support for multi-threaded recording
        }
        else {
            return false; // Unknown or unsupported RHI type
        }
    }
}