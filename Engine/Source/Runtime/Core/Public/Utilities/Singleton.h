#pragma once
#include <cassert>

/**
 * Singleton base class
 * @tparam T - Subclass type (must be passed through CRTP)
 * If you want to use a singleton and expose an API,
 * you need to give the API to the class itself rather than a specific function,
 * otherwise users won't be able to access your singleton.
 */
template<typename T>
class Singleton {
public:
    // No copying or moving
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    /**
     * Get the singleton instance
     * Thread safety (C 11 and above guarantees thread-safe initialization of local static variables)
     */
    static T& Get() {
        static T instance;
        return instance;
    }

protected:
    Singleton() = default;
    virtual ~Singleton() = default;
};