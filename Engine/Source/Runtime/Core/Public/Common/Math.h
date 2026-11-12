#pragma once

#include <algorithm>
#include <cmath>
#include <cassert>

/*
* We've defined the math types in the Engine here,
* so any files that need to use these math features should include this header.
*/

// First, we need to undefine the Windows macros to avoid polluting our code.
#undef min
#undef max
#undef lerp

// ============================================================
// Basic Template Definition
// ============================================================

template<typename T>
struct Vec2 {
    T x, y;

    Vec2() : x(0), y(0) {}
    Vec2(T v) : x(v), y(v) {}
    Vec2(T x, T y) : x(x), y(y) {}

    T& operator[](size_t i) { return (&x)[i]; }
    const T& operator[](size_t i) const { return (&x)[i]; }

    T* data() { return &x; }
    const T* data() const { return &x; }
};

template<typename T>
struct Vec3 {
    T x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(T v) : x(v), y(v), z(v) {}
    Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
    Vec3(const Vec2<T>& xy, T z) : x(xy.x), y(xy.y), z(z) {}

    T& operator[](size_t i) { return (&x)[i]; }
    const T& operator[](size_t i) const { return (&x)[i]; }

    T* data() { return &x; }
    const T* data() const { return &x; }
};

template<typename T>
struct alignas(16) Vec4 {
    T x, y, z, w;

    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(T v) : x(v), y(v), z(v), w(v) {}
    Vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec2<T>& xy, const Vec2<T>& zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}
    Vec4(const Vec3<T>& xyz, T w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
    Vec4(T x, const Vec3<T>& yzw) : x(x), y(yzw.x), z(yzw.y), w(yzw.z) {}
    Vec4(const Vec2<T>& xy, T z, T w) : x(xy.x), y(xy.y), z(z), w(w) {}

    T& operator[](size_t i) { return (&x)[i]; }
    const T& operator[](size_t i) const { return (&x)[i]; }

    T* data() { return &x; }
    const T* data() const { return &x; }
};

// ============================================================
// Type Alias
// ============================================================

using float2 = Vec2<float>;
using float3 = Vec3<float>;
using float4 = Vec4<float>;
using int2 = Vec2<int>;
using int3 = Vec3<int>;
using int4 = Vec4<int>;

// ============================================================
// Operator Overloading (Unified Implementation)
// ============================================================

// ---------- Unary Operator ----------
template<typename T>
Vec2<T> operator-(const Vec2<T>& a) { return Vec2<T>(-a.x, -a.y); }

template<typename T>
Vec3<T> operator-(const Vec3<T>& a) { return Vec3<T>(-a.x, -a.y, -a.z); }

template<typename T>
Vec4<T> operator-(const Vec4<T>& a) { return Vec4<T>(-a.x, -a.y, -a.z, -a.w); }

// ---------- Binary operator: vector vector ----------
template<typename T> Vec2<T> operator+(const Vec2<T>& a, const Vec2<T>& b) { return Vec2<T>(a.x + b.x, a.y + b.y); }
template<typename T> Vec2<T> operator-(const Vec2<T>& a, const Vec2<T>& b) { return Vec2<T>(a.x - b.x, a.y - b.y); }
template<typename T> Vec2<T> operator*(const Vec2<T>& a, const Vec2<T>& b) { return Vec2<T>(a.x * b.x, a.y * b.y); }
template<typename T> Vec2<T> operator/(const Vec2<T>& a, const Vec2<T>& b) { return Vec2<T>(a.x / b.x, a.y / b.y); }

template<typename T> Vec3<T> operator+(const Vec3<T>& a, const Vec3<T>& b) { return Vec3<T>(a.x + b.x, a.y + b.y, a.z + b.z); }
template<typename T> Vec3<T> operator-(const Vec3<T>& a, const Vec3<T>& b) { return Vec3<T>(a.x - b.x, a.y - b.y, a.z - b.z); }
template<typename T> Vec3<T> operator*(const Vec3<T>& a, const Vec3<T>& b) { return Vec3<T>(a.x * b.x, a.y * b.y, a.z * b.z); }
template<typename T> Vec3<T> operator/(const Vec3<T>& a, const Vec3<T>& b) { return Vec3<T>(a.x / b.x, a.y / b.y, a.z / b.z); }

template<typename T> Vec4<T> operator+(const Vec4<T>& a, const Vec4<T>& b) { return Vec4<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
template<typename T> Vec4<T> operator-(const Vec4<T>& a, const Vec4<T>& b) { return Vec4<T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
template<typename T> Vec4<T> operator*(const Vec4<T>& a, const Vec4<T>& b) { return Vec4<T>(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
template<typename T> Vec4<T> operator/(const Vec4<T>& a, const Vec4<T>& b) { return Vec4<T>(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }

// ---------- Comparison Operators (Vector to Vector, returns bool) ---------- 
template<typename T> bool operator==(const Vec2<T>& a, const Vec2<T>& b) {return a.x == b.x && a.y == b.y;}
template<typename T> bool operator!=(const Vec2<T>& a, const Vec2<T>& b) {return a.x != b.x || a.y != b.y;}
template<typename T> bool operator<(const Vec2<T>& a, const Vec2<T>& b) {return a.x < b.x && a.y < b.y;}
template<typename T> bool operator>(const Vec2<T>& a, const Vec2<T>& b) {return a.x > b.x && a.y > b.y;}
template<typename T> bool operator<=(const Vec2<T>& a, const Vec2<T>& b) {return a.x <= b.x && a.y <= b.y;}
template<typename T> bool operator>=(const Vec2<T>& a, const Vec2<T>& b) {return a.x >= b.x && a.y >= b.y;}

template<typename T> bool operator==(const Vec3<T>& a, const Vec3<T>& b) {return a.x == b.x && a.y == b.y && a.z == b.z;}
template<typename T> bool operator!=(const Vec3<T>& a, const Vec3<T>& b) {return a.x != b.x || a.y != b.y || a.z != b.z;}
template<typename T> bool operator<(const Vec3<T>& a, const Vec3<T>& b) {return a.x < b.x && a.y < b.y && a.z < b.z;}
template<typename T> bool operator>(const Vec3<T>& a, const Vec3<T>& b) {return a.x > b.x && a.y > b.y && a.z > b.z;}
template<typename T> bool operator<=(const Vec3<T>& a, const Vec3<T>& b) {return a.x <= b.x && a.y <= b.y && a.z <= b.z;}
template<typename T> bool operator>=(const Vec3<T>& a, const Vec3<T>& b) {return a.x >= b.x && a.y >= b.y && a.z >= b.z;}

template<typename T> bool operator==(const Vec4<T>& a, const Vec4<T>& b) {return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;}
template<typename T> bool operator!=(const Vec4<T>& a, const Vec4<T>& b) {return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;}
template<typename T> bool operator<(const Vec4<T>& a, const Vec4<T>& b) {return a.x < b.x && a.y < b.y && a.z < b.z && a.w < b.w;}
template<typename T> bool operator>(const Vec4<T>& a, const Vec4<T>& b) {return a.x > b.x && a.y > b.y && a.z > b.z && a.w > b.w;}
template<typename T> bool operator<=(const Vec4<T>& a, const Vec4<T>& b) {return a.x <= b.x && a.y <= b.y && a.z <= b.z && a.w <= b.w;}
template<typename T> bool operator>=(const Vec4<T>& a, const Vec4<T>& b) {return a.x >= b.x && a.y >= b.y && a.z >= b.z && a.w >= b.w;}

// ---------- Binary operator: vector scalar ----------
template<typename T> Vec2<T> operator+(const Vec2<T>& a, T s) { return Vec2<T>(a.x + s, a.y + s); }
template<typename T> Vec2<T> operator+(T s, const Vec2<T>& a) { return Vec2<T>(s + a.x, s + a.y); }
template<typename T> Vec2<T> operator-(const Vec2<T>& a, T s) { return Vec2<T>(a.x - s, a.y - s); }
template<typename T> Vec2<T> operator-(T s, const Vec2<T>& a) { return Vec2<T>(s - a.x, s - a.y); }
template<typename T> Vec2<T> operator*(const Vec2<T>& a, T s) { return Vec2<T>(a.x * s, a.y * s); }
template<typename T> Vec2<T> operator*(T s, const Vec2<T>& a) { return Vec2<T>(s * a.x, s * a.y); }
template<typename T> Vec2<T> operator/(const Vec2<T>& a, T s) { return Vec2<T>(a.x / s, a.y / s); }
template<typename T> Vec2<T> operator/(T s, const Vec2<T>& a) { return Vec2<T>(s / a.x, s / a.y); }

template<typename T> Vec3<T> operator+(const Vec3<T>& a, T s) { return Vec3<T>(a.x + s, a.y + s, a.z + s); }
template<typename T> Vec3<T> operator+(T s, const Vec3<T>& a) { return Vec3<T>(s + a.x, s + a.y, s + a.z); }
template<typename T> Vec3<T> operator-(const Vec3<T>& a, T s) { return Vec3<T>(a.x - s, a.y - s, a.z - s); }
template<typename T> Vec3<T> operator-(T s, const Vec3<T>& a) { return Vec3<T>(s - a.x, s - a.y, s - a.z); }
template<typename T> Vec3<T> operator*(const Vec3<T>& a, T s) { return Vec3<T>(a.x * s, a.y * s, a.z * s); }
template<typename T> Vec3<T> operator*(T s, const Vec3<T>& a) { return Vec3<T>(s * a.x, s * a.y, s * a.z); }
template<typename T> Vec3<T> operator/(const Vec3<T>& a, T s) { return Vec3<T>(a.x / s, a.y / s, a.z / s); }
template<typename T> Vec3<T> operator/(T s, const Vec3<T>& a) { return Vec3<T>(s / a.x, s / a.y, s / a.z); }

template<typename T> Vec4<T> operator+(const Vec4<T>& a, T s) { return Vec4<T>(a.x + s, a.y + s, a.z + s, a.w + s); }
template<typename T> Vec4<T> operator+(T s, const Vec4<T>& a) { return Vec4<T>(s + a.x, s + a.y, s + a.z, s + a.w); }
template<typename T> Vec4<T> operator-(const Vec4<T>& a, T s) { return Vec4<T>(a.x - s, a.y - s, a.z - s, a.w - s); }
template<typename T> Vec4<T> operator-(T s, const Vec4<T>& a) { return Vec4<T>(s - a.x, s - a.y, s - a.z, s - a.w); }
template<typename T> Vec4<T> operator*(const Vec4<T>& a, T s) { return Vec4<T>(a.x * s, a.y * s, a.z * s, a.w * s); }
template<typename T> Vec4<T> operator*(T s, const Vec4<T>& a) { return Vec4<T>(s * a.x, s * a.y, s * a.z, s * a.w); }
template<typename T> Vec4<T> operator/(const Vec4<T>& a, T s) { return Vec4<T>(a.x / s, a.y / s, a.z / s, a.w / s); }
template<typename T> Vec4<T> operator/(T s, const Vec4<T>& a) { return Vec4<T>(s / a.x, s / a.y, s / a.z, s / a.w); }

// ---------- Compound Assignment Operator ----------
template<typename T> Vec2<T>& operator+=(Vec2<T>& a, const Vec2<T>& b) { a.x += b.x; a.y += b.y; return a; }
template<typename T> Vec2<T>& operator-=(Vec2<T>& a, const Vec2<T>& b) { a.x -= b.x; a.y -= b.y; return a; }
template<typename T> Vec2<T>& operator*=(Vec2<T>& a, const Vec2<T>& b) { a.x *= b.x; a.y *= b.y; return a; }
template<typename T> Vec2<T>& operator/=(Vec2<T>& a, const Vec2<T>& b) { a.x /= b.x; a.y /= b.y; return a; }
template<typename T> Vec2<T>& operator+=(Vec2<T>& a, T s) { a.x += s; a.y += s; return a; }
template<typename T> Vec2<T>& operator-=(Vec2<T>& a, T s) { a.x -= s; a.y -= s; return a; }
template<typename T> Vec2<T>& operator*=(Vec2<T>& a, T s) { a.x *= s; a.y *= s; return a; }
template<typename T> Vec2<T>& operator/=(Vec2<T>& a, T s) { a.x /= s; a.y /= s; return a; }

template<typename T> Vec3<T>& operator+=(Vec3<T>& a, const Vec3<T>& b) { a.x += b.x; a.y += b.y; a.z += b.z; return a; }
template<typename T> Vec3<T>& operator-=(Vec3<T>& a, const Vec3<T>& b) { a.x -= b.x; a.y -= b.y; a.z -= b.z; return a; }
template<typename T> Vec3<T>& operator*=(Vec3<T>& a, const Vec3<T>& b) { a.x *= b.x; a.y *= b.y; a.z *= b.z; return a; }
template<typename T> Vec3<T>& operator/=(Vec3<T>& a, const Vec3<T>& b) { a.x /= b.x; a.y /= b.y; a.z /= b.z; return a; }
template<typename T> Vec3<T>& operator+=(Vec3<T>& a, T s) { a.x += s; a.y += s; a.z += s; return a; }
template<typename T> Vec3<T>& operator-=(Vec3<T>& a, T s) { a.x -= s; a.y -= s; a.z -= s; return a; }
template<typename T> Vec3<T>& operator*=(Vec3<T>& a, T s) { a.x *= s; a.y *= s; a.z *= s; return a; }
template<typename T> Vec3<T>& operator/=(Vec3<T>& a, T s) { a.x /= s; a.y /= s; a.z /= s; return a; }

template<typename T> Vec4<T>& operator+=(Vec4<T>& a, const Vec4<T>& b) { a.x += b.x; a.y += b.y; a.z += b.z; a.w += b.w; return a; }
template<typename T> Vec4<T>& operator-=(Vec4<T>& a, const Vec4<T>& b) { a.x -= b.x; a.y -= b.y; a.z -= b.z; a.w -= b.w; return a; }
template<typename T> Vec4<T>& operator*=(Vec4<T>& a, const Vec4<T>& b) { a.x *= b.x; a.y *= b.y; a.z *= b.z; a.w *= b.w; return a; }
template<typename T> Vec4<T>& operator/=(Vec4<T>& a, const Vec4<T>& b) { a.x /= b.x; a.y /= b.y; a.z /= b.z; a.w /= b.w; return a; }
template<typename T> Vec4<T>& operator+=(Vec4<T>& a, T s) { a.x += s; a.y += s; a.z += s; a.w += s; return a; }
template<typename T> Vec4<T>& operator-=(Vec4<T>& a, T s) { a.x -= s; a.y -= s; a.z -= s; a.w -= s; return a; }
template<typename T> Vec4<T>& operator*=(Vec4<T>& a, T s) { a.x *= s; a.y *= s; a.z *= s; a.w *= s; return a; }
template<typename T> Vec4<T>& operator/=(Vec4<T>& a, T s) { a.x /= s; a.y /= s; a.z /= s; a.w /= s; return a; }

// ============================================================
// Common Functions (Member/Global)
// ============================================================

// ---------- Length and normalization ----------
template<typename T>
T length(const Vec2<T>& v) { return std::sqrt(v.x * v.x + v.y * v.y); }

template<typename T>
T length(const Vec3<T>& v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

template<typename T>
T length(const Vec4<T>& v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }

template<typename T>
Vec2<T> normalize(const Vec2<T>& v) { T len = length(v); return len > 0 ? v / len : Vec2<T>(0); }

template<typename T>
Vec3<T> normalize(const Vec3<T>& v) { T len = length(v); return len > 0 ? v / len : Vec3<T>(0); }

template<typename T>
Vec4<T> normalize(const Vec4<T>& v) { T len = length(v); return len > 0 ? v / len : Vec4<T>(0); }

// ---------- Dot product ----------
template<typename T>
T dot(const Vec2<T>& a, const Vec2<T>& b) { return a.x * b.x + a.y * b.y; }

template<typename T>
T dot(const Vec3<T>& a, const Vec3<T>& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

template<typename T>
T dot(const Vec4<T>& a, const Vec4<T>& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

// ---------- Cross Product (3D Only) ----------
template<typename T>
Vec3<T> cross(const Vec3<T>& a, const Vec3<T>& b) {
    return Vec3<T>(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

// ---------- Minimum/Maximum Amount ----------
template<typename T>
Vec2<T> min(const Vec2<T>& a, const Vec2<T>& b) {
    return Vec2<T>(std::min(a.x, b.x), std::min(a.y, b.y));
}

template<typename T>
Vec3<T> min(const Vec3<T>& a, const Vec3<T>& b) {
    return Vec3<T>(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

template<typename T>
Vec4<T> min(const Vec4<T>& a, const Vec4<T>& b) {
    return Vec4<T>(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w));
}

template<typename T>
Vec2<T> max(const Vec2<T>& a, const Vec2<T>& b) {
    return Vec2<T>(std::max(a.x, b.x), std::max(a.y, b.y));
}

template<typename T>
Vec3<T> max(const Vec3<T>& a, const Vec3<T>& b) {
    return Vec3<T>(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}

template<typename T>
Vec4<T> max(const Vec4<T>& a, const Vec4<T>& b) {
    return Vec4<T>(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w));
}

// ---------- Linear interpolation ----------
template<typename T>
Vec2<T> lerp(const Vec2<T>& a, const Vec2<T>& b, T t) {
    return a + (b - a) * t;
}

template<typename T>
Vec3<T> lerp(const Vec3<T>& a, const Vec3<T>& b, T t) {
    return a + (b - a) * t;
}

template<typename T>
Vec4<T> lerp(const Vec4<T>& a, const Vec4<T>& b, T t) {
    return a + (b - a) * t;
}

// ---------- Type Conversion (float <-> int) ----------
inline float2 to_float2(const int2& v) { return float2((float)v.x, (float)v.y); }
inline float3 to_float3(const int3& v) { return float3((float)v.x, (float)v.y, (float)v.z); }
inline float4 to_float4(const int4& v) { return float4((float)v.x, (float)v.y, (float)v.z, (float)v.w); }

inline int2 to_int2(const float2& v) { return int2((int)v.x, (int)v.y); }
inline int3 to_int3(const float3& v) { return int3((int)v.x, (int)v.y, (int)v.z); }
inline int4 to_int4(const float4& v) { return int4((int)v.x, (int)v.y, (int)v.z, (int)v.w); }
