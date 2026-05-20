#pragma once

#include <type_traits>

template<typename To, typename From>
inline To* SafeCast(From* ptr) {
    static_assert(std::is_pointer_v<To> || std::is_class_v<To>,
        "To must be a pointer or class type");
    if constexpr (std::is_void_v<From>) {
        return static_cast<To*>(ptr);
    } else {
        static_assert(std::is_convertible_v<From*, To*>,
            "Incompatible types");
        return static_cast<To*>(ptr);
    }
}