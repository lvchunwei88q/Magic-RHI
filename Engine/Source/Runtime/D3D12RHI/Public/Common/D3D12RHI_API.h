#pragma once

#if defined(_WIN32)
    #ifdef D3D12RHI_EXPORTS
        #define D3D12RHI_API __declspec(dllexport)
    #else
        #define D3D12RHI_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__) && __GNUC__ >= 4
    #define D3D12RHI_API __attribute__((visibility("default")))
#else
    #define D3D12RHI_API
#endif