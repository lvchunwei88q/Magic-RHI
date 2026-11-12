#pragma once

#if defined(_WIN32)
    #ifdef RHI_EXPORTS
        #define RHI_API __declspec(dllexport)
    #else
        #define RHI_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__) && __GNUC__ >= 4
    #define RHI_API __attribute__((visibility("default")))
#else
    #define RHI_API
#endif