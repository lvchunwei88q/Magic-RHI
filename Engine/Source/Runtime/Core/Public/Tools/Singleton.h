#pragma once
#include <cassert>

/**
 * 单例基类
 * @tparam T - 子类类型（必须通过 CRTP 传入）
 * 并且一般来说我们可以之间继承此类并且公开API接口，但是这也要求了你必须将API写在Class名称中而非者单独的函数中，否则你就无法通过Get()获取到实例了
 */
template<typename T>
class Singleton {
public:
    // 禁止拷贝和移动
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    /**
     * 获取单例实例
     * 线程安全（C++11 及以上保证局部静态变量初始化的线程安全）
     */
    static T& Get() {
        static T instance;
        return instance;
    }

protected:
    Singleton() = default;
    virtual ~Singleton() = default;
};