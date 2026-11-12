#pragma once

#if defined(_WIN32)
    #ifdef VULKANRHI_EXPORTS
        #define VULKANRHI_API __declspec(dllexport)
    #else
        #define VULKANRHI_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__) && __GNUC__ >= 4
    #define VULKANRHI_API __attribute__((visibility("default")))
#else
    #define VULKANRHI_API
#endif