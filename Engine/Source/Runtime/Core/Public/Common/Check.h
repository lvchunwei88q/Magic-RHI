#pragma once

#include <cassert>
#include <type_traits>
#include <stdexcept>

using HRESULT = long;
using UINT = unsigned int;

#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif

// Visual Studio IntelliSense
#if defined(__INTELLISENSE__)
#define MAGIC_IN_IDE 1  
// JetBrains ReSharper C++
#elif defined(__RESHARPER__) || defined(__JETBRAINS_IDE__)
#define MAGIC_IN_IDE 1
// Clang tools
#elif defined(__clang_analyzer__)
#define MAGIC_IN_IDE 1
// Other static analysis tools
#elif defined(__cppcheck__) || defined(__clang_tidy__)
#define MAGIC_IN_IDE 1
#else
#define MAGIC_IN_IDE 0
#endif

/*
* This is used to control the compiler (MSVC) compilation settings.
* Note that you should not use the tools here casually unless you know what you are doing.
*/

#define DISABLE_DLL_WARNINGS_PUSH __pragma(warning(push)) __pragma(warning(disable: 4251))
#define DISABLE_DLL_WARNINGS_POP __pragma(warning(pop))

template<typename To, typename From>
inline To* SafeCast(From* ptr) {
    if (!ptr) return nullptr;
    
    // Check type relationships at compile time
    constexpr bool is_upcast = std::is_base_of_v<To, From>;
    constexpr bool is_downcast = std::is_base_of_v<From, To>;
    
    static_assert(is_upcast || is_downcast,
        "SafeCast: Types must be related by inheritance");
    
    if constexpr (is_upcast) {
        return static_cast<To*>(ptr);
    } 
    if constexpr (is_downcast) {
#ifdef _DEBUG
        To* result = dynamic_cast<To*>(ptr);
        assert(result && "SafeCast: Type mismatch");
        return result;
#else
        return static_cast<To*>(ptr);
#endif
    }
    
    return nullptr;
}

inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    explicit HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}

template<typename T>
inline void ThrowErrorMessage(T&& message)
{
    throw std::runtime_error(std::forward<T>(message));
}

inline void ThrowIf(bool condition, const std::string& message)
{
    if (condition)
    {
        throw std::runtime_error(message);
    }
}

// // Check if it inherits from the parent class
#define FORBIDDEN_METHOD_CONCEPT(Space,method_name) \
    template<typename T> \
    concept HasForbiddenMethod_##Space_##method_name = requires(T t) { t.method_name(); }

#define COMBINE_FORBIDDEN_METHODS(Space,method_name) \
    HasForbiddenMethod_##Space_##method_name<T>

#define COMBINE_FORBIDDEN_NAME(Space) \
    Has##Space

#define CHECK_COMBINE_MEMBER(ClassName, CheckerName) \
    static_assert(Has##CheckerName<ClassName>, \
        "ERROR: " #ClassName " is missing required member functions! " \
        "Please ensure it inherits from " #CheckerName)

#define IDE_CHECK_COMBINE_MEMBER(CLASS_NAME,TARGET)                         \
    [[maybe_unused]] static constexpr bool __check = [] {                   \
    if constexpr (MAGIC_IN_IDE) {                                           \
            /* IDE/Compiler analysis mode - perform checks */               \
            CHECK_COMBINE_MEMBER(CLASS_NAME, TARGET);                       \
    }return true;}();