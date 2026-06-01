#pragma once
#include "Subsystem/Subsystem.h"
#include "Tools/Singleton.h"

// the subsystems template Cannot be used as an export API!!!
template<typename T, Core::Priority Priority>
class SubsystemTemplate : public Core::Subsystem, public Singleton<T>
{
public:
    static void RegisterStatic() {
        SubsystemTemplate::Get().template Register<T, Priority>();
    }
    static void RegisterStatic(const char* tags) {
        SubsystemTemplate::Get().template Register<T,Priority>(tags);
    }
};

// 如果使用通知系统我们推荐使用字符串来确定type，使用encodeToSizeT来编码字符串到size_t，这样可以在保持可读性的同时提高效率。
constexpr size_t encodeToSizeT(const char* str) {   // Max 10 characters
    int len = 0;
    for (const char* p = str; *p; ++p) ++len;
    if (len > 10) {
#ifdef _DEBUG
        __debugbreak();
#endif
        return 0;
    }

    size_t result = 0;
    int count = 0;
    for (const char* p = str; *p && count < 10; ++p, ++count) {
        char c = *p;
        int val = (c >= 'A' && c <= 'Z') ? (c - 'A') :           // 0-25
            (c >= 'a' && c <= 'z') ? (c - 'a' + 26) :      // 26-51
            -1;
        if (val == -1) return 0;  // 无效字符返回0
        result = (result << 6) | val;
    }
    for (int i = count; i < 10; ++i) {
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
* 在使用模板时这个Class将不可以用于导出API。
* 如果需要导出API，请使用AUTO_REGISTER_SINGLETON宏，并在对应的cpp文件中实现注册逻辑。
* 并且直接继承Subsystem和Singleton，而不是SubsystemTemplate。
*/
#define AUTO_REGISTER_SINGLETON(T, Priority_)                               \
struct CAT(T, _AutoRegister)                                                 \
{                                                                           \
    CAT(T, _AutoRegister)() {                                                \
        T::Get().Register<T, Core::Priority::Priority_>();\
    }                                                                       \
};                                                                          \
static CAT(T, _AutoRegister) CAT(s_##T, _AutoRegister);

#define AUTO_REGISTER_SINGLETON_NOTIFICATION(T,TAGS , Priority_)             \
struct CAT(T, _AutoRegister)                                                 \
{                                                                           \
    CAT(T, _AutoRegister)() {                                                \
        T::Get().Register<T, Core::Priority::Priority_>(TAGS);\
    }                                                                       \
};                                                                          \
static CAT(T, _AutoRegister) CAT(s_##T, _AutoRegister);

#define AUTO_REGISTER_SINGLETON_INCLUDE(T)                        \
struct CAT(T, _AutoRegister);                                     \
extern CAT(T, _AutoRegister) CAT(s_##T, _AutoRegister);           