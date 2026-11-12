#pragma once
#include "Subsystem/Subsystem.h"
#include "Utilities/Singleton.h"

// the subsystems template Cannot be used as an export API!!!
template<typename T, Core::Priority Priority>
class SubsystemTemplate : public SubSystem, public Singleton<T>
{
public:
    static void RegisterStatic() {
        SubsystemTemplate::Get().template Register<T, Priority>();
    }
    static void RegisterStatic(const char* tags) {
        SubsystemTemplate::Get().template Register<T,Priority>(tags);
    }
};

// If you use the notification system, we recommend using strings to identify the type and using encodeToSizeT to encode the string into size_t,
// which can improve efficiency while keeping it readable.
template<size_t N>
constexpr size_t encodeToSizeT(const char(&str)[N]) {   // Max 10 characters
    static_assert(N <= 11, "The input character label is more than 10, please reduce the label size");

    size_t result = 0;
    int count = 0;
    for (const char* p = str; *p && count < N; ++p, ++count) {
        char c = *p;
        int val = (c >= 'A' && c <= 'Z') ? (c - 'A') :           // 0-25
            (c >= 'a' && c <= 'z') ? (c - 'a' + 26) :      // 26-51
            -1;
        if (val == -1) return 0;  // Invalid character returned0
        result = (result << 6) | val;
    }
    for (int i = count; i < N; ++i) {
        result = (result << 6) | 63;
    }

    return result;
}

#define CAT(a, b) a##b
#define AUTO_REGISTER(T) \
    inline static bool CAT(T, _registered) = []() { \
        T::RegisterStatic(); \
        return true; \
    }(); \
    inline bool CAT(T, _dummy) = CAT(T, _registered);

#define AUTO_REGISTER_NOTIFICATION(T,TAGS) \
    inline static bool CAT(T, _registered) = []() { \
        T::RegisterStatic(TAGS); \
        return true; \
    }(); \
    inline bool CAT(T, _dummy) = CAT(T, _registered);

/*
* This class can't be used to export APIs when using templates.
* If you need to export an API, use the AUTO_REGISTER_SINGLETON macro and implement the registration logic in the corresponding cpp file.
* Like directly inheriting from Subsystem and Singleton, instead of SubsystemTemplate.
*/
#define AUTO_REGISTER_SINGLETON(T, Priority_)                               \
struct CAT(T, _AutoRegister)                                                \
{                                                                           \
    CAT(T, _AutoRegister)() {                                               \
        T::Get().Register<T, ::Core::Priority::Priority_>();                \
    }                                                                       \
};                                                                          \
static CAT(T, _AutoRegister) CAT(s_##T, _AutoRegister);

#define AUTO_REGISTER_SINGLETON_NOTIFICATION(T,TAGS , Priority_)            \
struct CAT(T, _AutoRegister)                                                \
{                                                                           \
    CAT(T, _AutoRegister)() {                                               \
        T::Get().Register<T, ::Core::Priority::Priority_>(TAGS);            \
    }                                                                       \
};                                                                          \
static CAT(T, _AutoRegister) CAT(s_##T, _AutoRegister);

#define AUTO_REGISTER_SINGLETON_INCLUDE(T)                        \
struct CAT(T, _AutoRegister);                                     \
extern CAT(T, _AutoRegister) CAT(s_##T, _AutoRegister);           