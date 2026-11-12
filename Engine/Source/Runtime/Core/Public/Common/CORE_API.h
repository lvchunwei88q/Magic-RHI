#pragma once
// Export/Import Macro
#if defined(_WIN32)
    #ifdef CORE_EXPORTS
        #define CORE_API __declspec(dllexport)
    #else
        #define CORE_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__) && __GNUC__ >= 4
    #define CORE_API __attribute__((visibility("default")))
#else
    #define CORE_API
#endif