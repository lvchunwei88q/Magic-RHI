#pragma once
#include <cassert>

/**
 * 单例基类
 * @tparam T - 子类类型（必须通过 CRTP 传入）
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