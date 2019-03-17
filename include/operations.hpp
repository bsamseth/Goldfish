#pragma once

#define ENABLE_BASE_OPERATORS_ON(T)                                    \
    constexpr T operator+(T d1, T d2) { return T(int(d1) + int(d2)); } \
    constexpr T operator-(T d1, T d2) { return T(int(d1) - int(d2)); } \
    constexpr T operator-(T d) { return T(-int(d)); }                  \
    inline T&   operator+=(T& d1, T d2) { return d1 = d1 + d2; }       \
    inline T&   operator-=(T& d1, T d2) { return d1 = d1 - d2; }

#define ENABLE_INCR_OPERATORS_ON(T)                          \
    inline T& operator++(T& d) { return d = T(int(d) + 1); } \
    inline T& operator--(T& d) { return d = T(int(d) - 1); }

#define ENABLE_FULL_OPERATORS_ON(T)                                     \
    ENABLE_BASE_OPERATORS_ON(T)                                         \
    ENABLE_INCR_OPERATORS_ON(T)                                         \
    constexpr T   operator*(int i, T d) { return T(i * int(d)); }       \
    constexpr T   operator*(T d, int i) { return T(int(d) * i); }       \
    constexpr T   operator/(T d, int i) { return T(int(d) / i); }       \
    constexpr int operator/(T d1, T d2) { return int(d1) / int(d2); }   \
    inline T&     operator*=(T& d, int i) { return d = T(int(d) * i); } \
    inline T&     operator/=(T& d, int i) { return d = T(int(d) / i); }
