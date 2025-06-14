/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMacros_DEFINED
#define SkMacros_DEFINED

#include "CZ/skia/private/base/SkTo.h" // IWYU pragma: keep

/*
 *  Usage:  SK_MACRO_CONCAT(a, b)   to construct the symbol ab
 *
 *  SK_MACRO_CONCAT_IMPL_PRIV just exists to make this work. Do not use directly
 *
 */
#define SK_MACRO_CONCAT(X, Y)           SK_MACRO_CONCAT_IMPL_PRIV(X, Y)
#define SK_MACRO_CONCAT_IMPL_PRIV(X, Y)  X ## Y

/*
 *  Usage: SK_MACRO_APPEND_LINE(foo)    to make foo123, where 123 is the current
 *                                      line number. Easy way to construct
 *                                      unique names for local functions or
 *                                      variables.
 */
#define SK_MACRO_APPEND_LINE(name)  SK_MACRO_CONCAT(name, __LINE__)

#define SK_MACRO_APPEND_COUNTER(name) SK_MACRO_CONCAT(name, __COUNTER__)

////////////////////////////////////////////////////////////////////////////////

// Can be used to bracket data types that must be dense/packed, e.g. hash keys.
#if defined(__clang__)  // This should work on GCC too, but GCC diagnostic pop didn't seem to work!
    #define SK_BEGIN_REQUIRE_DENSE _Pragma("GCC diagnostic push") \
                                   _Pragma("GCC diagnostic error \"-Wpadded\"")
    #define SK_END_REQUIRE_DENSE   _Pragma("GCC diagnostic pop")
#else
    #define SK_BEGIN_REQUIRE_DENSE
    #define SK_END_REQUIRE_DENSE
#endif

#if defined(__clang__) && defined(__has_feature)
    // Some compilers have a preprocessor that does not appear to do short-circuit
    // evaluation as expected
    #if __has_feature(leak_sanitizer) || __has_feature(address_sanitizer)
        // Chrome had issues if we tried to include lsan_interface.h ourselves.
        // https://github.com/llvm/llvm-project/blob/10a35632d55bb05004fe3d0c2d4432bb74897ee7/compiler-rt/include/sanitizer/lsan_interface.h#L26
extern "C" {
        void __lsan_ignore_object(const void *p);
}
        #define SK_INTENTIONALLY_LEAKED(X) __lsan_ignore_object(X)
    #else
        #define SK_INTENTIONALLY_LEAKED(X) ((void)0)
    #endif
#else
    #define SK_INTENTIONALLY_LEAKED(X) ((void)0)
#endif

#define SK_INIT_TO_AVOID_WARNING    = 0

////////////////////////////////////////////////////////////////////////////////

/**
 * Defines overloaded bitwise operators to make it easier to use an enum as a
 * bitfield.
 */
#define SK_MAKE_BITFIELD_OPS(X) \
    inline X operator ~(X a) { \
        using U = std::underlying_type_t<X>; \
        return (X) (~static_cast<U>(a)); \
    } \
    inline X operator |(X a, X b) { \
        using U = std::underlying_type_t<X>; \
        return (X) (static_cast<U>(a) | static_cast<U>(b)); \
    } \
    inline X& operator |=(X& a, X b) { \
        return (a = a | b); \
    } \
    inline X operator &(X a, X b) { \
        using U = std::underlying_type_t<X>; \
        return (X) (static_cast<U>(a) & static_cast<U>(b)); \
    } \
    inline X& operator &=(X& a, X b) { \
        return (a = a & b); \
    }

#define SK_DECL_BITFIELD_OPS_FRIENDS(X) \
    friend X operator ~(X a); \
    friend X operator |(X a, X b); \
    friend X& operator |=(X& a, X b); \
    \
    friend X operator &(X a, X b); \
    friend X& operator &=(X& a, X b);

/**
 * Wraps a C++11 enum that we use as a bitfield, and enables a limited amount of
 * masking with type safety. Instantiated with the ~ operator.
 */
template<typename TFlags> class SkTFlagsMask {
public:
    constexpr explicit SkTFlagsMask(TFlags value) : SkTFlagsMask(static_cast<int>(value)) {}
    constexpr explicit SkTFlagsMask(int value) : fValue(value) {}
    constexpr int value() const { return fValue; }
private:
    const int fValue;
};

/**
 * Defines bitwise operators that make it possible to use an enum class as a
 * basic bitfield.
 */
#define SK_MAKE_BITFIELD_CLASS_OPS(X) \
    [[maybe_unused]] constexpr SkTFlagsMask<X> operator~(X a) { \
        return SkTFlagsMask<X>(~static_cast<int>(a)); \
    } \
    [[maybe_unused]] constexpr X operator|(X a, X b) { \
        return static_cast<X>(static_cast<int>(a) | static_cast<int>(b)); \
    } \
    [[maybe_unused]] inline X& operator|=(X& a, X b) { \
        return (a = a | b); \
    } \
    [[maybe_unused]] constexpr bool operator&(X a, X b) { \
        return SkToBool(static_cast<int>(a) & static_cast<int>(b)); \
    } \
    [[maybe_unused]] constexpr SkTFlagsMask<X> operator|(SkTFlagsMask<X> a, SkTFlagsMask<X> b) { \
        return SkTFlagsMask<X>(a.value() | b.value()); \
    } \
    [[maybe_unused]] constexpr SkTFlagsMask<X> operator|(SkTFlagsMask<X> a, X b) { \
        return SkTFlagsMask<X>(a.value() | static_cast<int>(b)); \
    } \
    [[maybe_unused]] constexpr SkTFlagsMask<X> operator|(X a, SkTFlagsMask<X> b) { \
        return SkTFlagsMask<X>(static_cast<int>(a) | b.value()); \
    } \
    [[maybe_unused]] constexpr X operator&(SkTFlagsMask<X> a, SkTFlagsMask<X> b) { \
        return static_cast<X>(a.value() & b.value()); \
    } \
    [[maybe_unused]] constexpr X operator&(SkTFlagsMask<X> a, X b) { \
        return static_cast<X>(a.value() & static_cast<int>(b)); \
    } \
    [[maybe_unused]] constexpr X operator&(X a, SkTFlagsMask<X> b) { \
        return static_cast<X>(static_cast<int>(a) & b.value()); \
    } \
    [[maybe_unused]] inline X& operator&=(X& a, SkTFlagsMask<X> b) { \
        return (a = a & b); \
    } \

#define SK_DECL_BITFIELD_CLASS_OPS_FRIENDS(X) \
    friend constexpr SkTFlagsMask<X> operator ~(X); \
    friend constexpr X operator |(X, X); \
    friend X& operator |=(X&, X); \
    friend constexpr bool operator &(X, X); \
    friend constexpr SkTFlagsMask<X> operator|(SkTFlagsMask<X>, SkTFlagsMask<X>); \
    friend constexpr SkTFlagsMask<X> operator|(SkTFlagsMask<X>, X); \
    friend constexpr SkTFlagsMask<X> operator|(X, SkTFlagsMask<X>); \
    friend constexpr X operator&(SkTFlagsMask<X>, SkTFlagsMask<X>); \
    friend constexpr X operator&(SkTFlagsMask<X>, X); \
    friend constexpr X operator&(X, SkTFlagsMask<X>); \
    friend X& operator &=(X&, SkTFlagsMask<X>)

#endif  // SkMacros_DEFINED
