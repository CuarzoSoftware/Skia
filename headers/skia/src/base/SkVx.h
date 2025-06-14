/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKVX_DEFINED
#define SKVX_DEFINED

// skvx::Vec<N,T> are SIMD vectors of N T's, a v1.5 successor to SkNx<N,T>.
//
// This time we're leaning a bit less on platform-specific intrinsics and a bit
// more on Clang/GCC vector extensions, but still keeping the option open to
// drop in platform-specific intrinsics, actually more easily than before.
//
// We've also fixed a few of the caveats that used to make SkNx awkward to work
// with across translation units.  skvx::Vec<N,T> always has N*sizeof(T) size
// and alignment and is safe to use across translation units freely.
// (Ideally we'd only align to T, but that tanks ARMv7 NEON codegen.)

#include "CZ/skia/private/base/SkFeatures.h"
#include "CZ/skia/src/base/SkUtils.h"
#include <algorithm>         // std::min, std::max
#include <cassert>           // assert()
#include <cmath>             // ceilf, floorf, truncf, roundf, sqrtf, etc.
#include <cstdint>           // intXX_t
#include <cstring>           // memcpy()
#include <initializer_list>  // std::initializer_list
#include <type_traits>
#include <utility>           // std::index_sequence

// Users may disable SIMD with SKNX_NO_SIMD, which may be set via compiler flags.
// The gn build has no option which sets SKNX_NO_SIMD.
// Use SKVX_USE_SIMD internally to avoid confusing double negation.
// Do not use 'defined' in a macro expansion.
#if !defined(SKNX_NO_SIMD)
    #define SKVX_USE_SIMD 1
#else
    #define SKVX_USE_SIMD 0
#endif

#if SKVX_USE_SIMD
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX
        #include <immintrin.h>
    #elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
        #include <smmintrin.h>
    #elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
        #include <emmintrin.h>
        #include <xmmintrin.h>
    #elif defined(SK_ARM_HAS_NEON)
        #include <arm_neon.h>
    #elif defined(__wasm_simd128__)
        #include <wasm_simd128.h>
    #elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
        #include <lasxintrin.h>
        #include <lsxintrin.h>
    #elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
        #include <lsxintrin.h>
    #endif
#endif

// To avoid ODR violations, all methods must be force-inlined...
#if defined(_MSC_VER)
    #define SKVX_ALWAYS_INLINE __forceinline
#else
    #define SKVX_ALWAYS_INLINE __attribute__((always_inline))
#endif

// ... and all standalone functions must be static.  Please use these helpers:
#define SI    static inline
#define SIT   template <       typename T> SI
#define SIN   template <int N            > SI
#define SINT  template <int N, typename T> SI
#define SINTU template <int N, typename T, typename U, \
                        typename=std::enable_if_t<std::is_convertible<U,T>::value>> SI

namespace skvx {

template <int N, typename T>
struct alignas(N*sizeof(T)) Vec;

template <int... Ix, int N, typename T>
SI Vec<sizeof...(Ix),T> shuffle(const Vec<N,T>&);

// All Vec have the same simple memory layout, the same as `T vec[N]`.
template <int N, typename T>
struct alignas(N*sizeof(T)) Vec {
    static_assert((N & (N-1)) == 0,        "N must be a power of 2.");
    static_assert(sizeof(T) >= alignof(T), "What kind of unusual T is this?");

    // Methods belong here in the class declaration of Vec only if:
    //   - they must be here, like constructors or operator[];
    //   - they'll definitely never want a specialized implementation.
    // Other operations on Vec should be defined outside the type.

    SKVX_ALWAYS_INLINE Vec() = default;
    SKVX_ALWAYS_INLINE Vec(T s) : lo(s), hi(s) {}

    // NOTE: Vec{x} produces x000..., whereas Vec(x) produces xxxx.... since this constructor fills
    // unspecified lanes with 0s, whereas the single T constructor fills all lanes with the value.
    SKVX_ALWAYS_INLINE Vec(std::initializer_list<T> xs) {
        T vals[N] = {0};
        assert(xs.size() <= (size_t)N);
        memcpy(vals, xs.begin(), std::min(xs.size(), (size_t)N)*sizeof(T));

        this->lo = Vec<N/2,T>::Load(vals +   0);
        this->hi = Vec<N/2,T>::Load(vals + N/2);
    }

    SKVX_ALWAYS_INLINE T  operator[](int i) const { return i<N/2 ? this->lo[i] : this->hi[i-N/2]; }
    SKVX_ALWAYS_INLINE T& operator[](int i)       { return i<N/2 ? this->lo[i] : this->hi[i-N/2]; }

    SKVX_ALWAYS_INLINE static Vec Load(const void* ptr) {
        return sk_unaligned_load<Vec>(ptr);
    }
    SKVX_ALWAYS_INLINE void store(void* ptr) const {
        // Note: Calling sk_unaligned_store produces slightly worse code here, for some reason
        memcpy(ptr, this, sizeof(Vec));
    }

    Vec<N/2,T> lo, hi;
};

// We have specializations for N == 1 (the base-case), as well as 2 and 4, where we add helpful
// constructors and swizzle accessors.
template <typename T>
struct alignas(4*sizeof(T)) Vec<4,T> {
    static_assert(sizeof(T) >= alignof(T), "What kind of unusual T is this?");

    SKVX_ALWAYS_INLINE Vec() = default;
    SKVX_ALWAYS_INLINE Vec(T s) : lo(s), hi(s) {}
    SKVX_ALWAYS_INLINE Vec(T x, T y, T z, T w) : lo(x,y), hi(z,w) {}
    SKVX_ALWAYS_INLINE Vec(Vec<2,T> xy, T z, T w) : lo(xy), hi(z,w) {}
    SKVX_ALWAYS_INLINE Vec(T x, T y, Vec<2,T> zw) : lo(x,y), hi(zw) {}
    SKVX_ALWAYS_INLINE Vec(Vec<2,T> xy, Vec<2,T> zw) : lo(xy), hi(zw) {}

    SKVX_ALWAYS_INLINE Vec(std::initializer_list<T> xs) {
        T vals[4] = {0};
        assert(xs.size() <= (size_t)4);
        memcpy(vals, xs.begin(), std::min(xs.size(), (size_t)4)*sizeof(T));

        this->lo = Vec<2,T>::Load(vals + 0);
        this->hi = Vec<2,T>::Load(vals + 2);
    }

    SKVX_ALWAYS_INLINE T  operator[](int i) const { return i<2 ? this->lo[i] : this->hi[i-2]; }
    SKVX_ALWAYS_INLINE T& operator[](int i)       { return i<2 ? this->lo[i] : this->hi[i-2]; }

    SKVX_ALWAYS_INLINE static Vec Load(const void* ptr) {
        return sk_unaligned_load<Vec>(ptr);
    }
    SKVX_ALWAYS_INLINE void store(void* ptr) const {
        memcpy(ptr, this, sizeof(Vec));
    }

    SKVX_ALWAYS_INLINE Vec<2,T>& xy() { return lo; }
    SKVX_ALWAYS_INLINE Vec<2,T>& zw() { return hi; }
    SKVX_ALWAYS_INLINE T& x() { return lo.lo.val; }
    SKVX_ALWAYS_INLINE T& y() { return lo.hi.val; }
    SKVX_ALWAYS_INLINE T& z() { return hi.lo.val; }
    SKVX_ALWAYS_INLINE T& w() { return hi.hi.val; }

    SKVX_ALWAYS_INLINE Vec<2,T> xy() const { return lo; }
    SKVX_ALWAYS_INLINE Vec<2,T> zw() const { return hi; }
    SKVX_ALWAYS_INLINE T x() const { return lo.lo.val; }
    SKVX_ALWAYS_INLINE T y() const { return lo.hi.val; }
    SKVX_ALWAYS_INLINE T z() const { return hi.lo.val; }
    SKVX_ALWAYS_INLINE T w() const { return hi.hi.val; }

    // Exchange-based swizzles. These should take 1 cycle on NEON and 3 (pipelined) cycles on SSE.
    SKVX_ALWAYS_INLINE Vec<4,T> yxwz() const { return shuffle<1,0,3,2>(*this); }
    SKVX_ALWAYS_INLINE Vec<4,T> zwxy() const { return shuffle<2,3,0,1>(*this); }

    Vec<2,T> lo, hi;
};

template <typename T>
struct alignas(2*sizeof(T)) Vec<2,T> {
    static_assert(sizeof(T) >= alignof(T), "What kind of unusual T is this?");

    SKVX_ALWAYS_INLINE Vec() = default;
    SKVX_ALWAYS_INLINE Vec(T s) : lo(s), hi(s) {}
    SKVX_ALWAYS_INLINE Vec(T x, T y) : lo(x), hi(y) {}

    SKVX_ALWAYS_INLINE Vec(std::initializer_list<T> xs) {
        T vals[2] = {0};
        assert(xs.size() <= (size_t)2);
        memcpy(vals, xs.begin(), std::min(xs.size(), (size_t)2)*sizeof(T));

        this->lo = Vec<1,T>::Load(vals + 0);
        this->hi = Vec<1,T>::Load(vals + 1);
    }

    SKVX_ALWAYS_INLINE T  operator[](int i) const { return i<1 ? this->lo[i] : this->hi[i-1]; }
    SKVX_ALWAYS_INLINE T& operator[](int i)       { return i<1 ? this->lo[i] : this->hi[i-1]; }

    SKVX_ALWAYS_INLINE static Vec Load(const void* ptr) {
        return sk_unaligned_load<Vec>(ptr);
    }
    SKVX_ALWAYS_INLINE void store(void* ptr) const {
        memcpy(ptr, this, sizeof(Vec));
    }

    SKVX_ALWAYS_INLINE T& x() { return lo.val; }
    SKVX_ALWAYS_INLINE T& y() { return hi.val; }

    SKVX_ALWAYS_INLINE T x() const { return lo.val; }
    SKVX_ALWAYS_INLINE T y() const { return hi.val; }

    // This exchange-based swizzle should take 1 cycle on NEON and 3 (pipelined) cycles on SSE.
    SKVX_ALWAYS_INLINE Vec<2,T> yx() const { return shuffle<1,0>(*this); }
    SKVX_ALWAYS_INLINE Vec<4,T> xyxy() const { return Vec<4,T>(*this, *this); }

    Vec<1,T> lo, hi;
};

template <typename T>
struct Vec<1,T> {
    T val = {};

    SKVX_ALWAYS_INLINE Vec() = default;
    SKVX_ALWAYS_INLINE Vec(T s) : val(s) {}

    SKVX_ALWAYS_INLINE Vec(std::initializer_list<T> xs) : val(xs.size() ? *xs.begin() : 0) {
        assert(xs.size() <= (size_t)1);
    }

    SKVX_ALWAYS_INLINE T  operator[](int i) const { assert(i == 0); return val; }
    SKVX_ALWAYS_INLINE T& operator[](int i)       { assert(i == 0); return val; }

    SKVX_ALWAYS_INLINE static Vec Load(const void* ptr) {
        return sk_unaligned_load<Vec>(ptr);
    }
    SKVX_ALWAYS_INLINE void store(void* ptr) const {
        memcpy(ptr, this, sizeof(Vec));
    }
};

// Translate from a value type T to its corresponding Mask, the result of a comparison.
template <typename T> struct Mask { using type = T; };
template <> struct Mask<float > { using type = int32_t; };
template <> struct Mask<double> { using type = int64_t; };
template <typename T> using M = typename Mask<T>::type;

// Join two Vec<N,T> into one Vec<2N,T>.
SINT Vec<2*N,T> join(const Vec<N,T>& lo, const Vec<N,T>& hi) {
    Vec<2*N,T> v;
    v.lo = lo;
    v.hi = hi;
    return v;
}

// We have three strategies for implementing Vec operations:
//    1) lean on Clang/GCC vector extensions when available;
//    2) use map() to apply a scalar function lane-wise;
//    3) recurse on lo/hi to scalar portable implementations.
// We can slot in platform-specific implementations as overloads for particular Vec<N,T>,
// or often integrate them directly into the recursion of style 3), allowing fine control.

#if SKVX_USE_SIMD && (defined(__clang__) || defined(__GNUC__))

    // VExt<N,T> types have the same size as Vec<N,T> and support most operations directly.
    #if defined(__clang__)
        template <int N, typename T>
        using VExt = T __attribute__((ext_vector_type(N)));

    #elif defined(__GNUC__)
        template <int N, typename T>
        struct VExtHelper {
            typedef T __attribute__((vector_size(N*sizeof(T)))) type;
        };

        template <int N, typename T>
        using VExt = typename VExtHelper<N,T>::type;

        // For some reason some (new!) versions of GCC cannot seem to deduce N in the generic
        // to_vec<N,T>() below for N=4 and T=float.  This workaround seems to help...
        SI Vec<4,float> to_vec(VExt<4,float> v) { return sk_bit_cast<Vec<4,float>>(v); }
    #endif

    SINT VExt<N,T> to_vext(const Vec<N,T>& v) { return sk_bit_cast<VExt<N,T>>(v); }
    SINT Vec <N,T> to_vec(const VExt<N,T>& v) { return sk_bit_cast<Vec <N,T>>(v); }

    SINT Vec<N,T> operator+(const Vec<N,T>& x, const Vec<N,T>& y) {
        return to_vec<N,T>(to_vext(x) + to_vext(y));
    }
    SINT Vec<N,T> operator-(const Vec<N,T>& x, const Vec<N,T>& y) {
        return to_vec<N,T>(to_vext(x) - to_vext(y));
    }
    SINT Vec<N,T> operator*(const Vec<N,T>& x, const Vec<N,T>& y) {
        return to_vec<N,T>(to_vext(x) * to_vext(y));
    }
    SINT Vec<N,T> operator/(const Vec<N,T>& x, const Vec<N,T>& y) {
        return to_vec<N,T>(to_vext(x) / to_vext(y));
    }

    SINT Vec<N,T> operator^(const Vec<N,T>& x, const Vec<N,T>& y) {
        return to_vec<N,T>(to_vext(x) ^ to_vext(y));
    }
    SINT Vec<N,T> operator&(const Vec<N,T>& x, const Vec<N,T>& y) {
        return to_vec<N,T>(to_vext(x) & to_vext(y));
    }
    SINT Vec<N,T> operator|(const Vec<N,T>& x, const Vec<N,T>& y) {
        return to_vec<N,T>(to_vext(x) | to_vext(y));
    }

    SINT Vec<N,T> operator!(const Vec<N,T>& x) { return to_vec<N,T>(!to_vext(x)); }
    SINT Vec<N,T> operator-(const Vec<N,T>& x) { return to_vec<N,T>(-to_vext(x)); }
    SINT Vec<N,T> operator~(const Vec<N,T>& x) { return to_vec<N,T>(~to_vext(x)); }

    SINT Vec<N,T> operator<<(const Vec<N,T>& x, int k) { return to_vec<N,T>(to_vext(x) << k); }
    SINT Vec<N,T> operator>>(const Vec<N,T>& x, int k) { return to_vec<N,T>(to_vext(x) >> k); }

    SINT Vec<N,M<T>> operator==(const Vec<N,T>& x, const Vec<N,T>& y) {
        return sk_bit_cast<Vec<N,M<T>>>(to_vext(x) == to_vext(y));
    }
    SINT Vec<N,M<T>> operator!=(const Vec<N,T>& x, const Vec<N,T>& y) {
        return sk_bit_cast<Vec<N,M<T>>>(to_vext(x) != to_vext(y));
    }
    SINT Vec<N,M<T>> operator<=(const Vec<N,T>& x, const Vec<N,T>& y) {
        return sk_bit_cast<Vec<N,M<T>>>(to_vext(x) <= to_vext(y));
    }
    SINT Vec<N,M<T>> operator>=(const Vec<N,T>& x, const Vec<N,T>& y) {
        return sk_bit_cast<Vec<N,M<T>>>(to_vext(x) >= to_vext(y));
    }
    SINT Vec<N,M<T>> operator< (const Vec<N,T>& x, const Vec<N,T>& y) {
        return sk_bit_cast<Vec<N,M<T>>>(to_vext(x) <  to_vext(y));
    }
    SINT Vec<N,M<T>> operator> (const Vec<N,T>& x, const Vec<N,T>& y) {
        return sk_bit_cast<Vec<N,M<T>>>(to_vext(x) >  to_vext(y));
    }

#else

    // Either SKNX_NO_SIMD is defined, or Clang/GCC vector extensions are not available.
    // We'll implement things portably with N==1 scalar implementations and recursion onto them.

    // N == 1 scalar implementations.
    SIT Vec<1,T> operator+(const Vec<1,T>& x, const Vec<1,T>& y) { return x.val + y.val; }
    SIT Vec<1,T> operator-(const Vec<1,T>& x, const Vec<1,T>& y) { return x.val - y.val; }
    SIT Vec<1,T> operator*(const Vec<1,T>& x, const Vec<1,T>& y) { return x.val * y.val; }
    SIT Vec<1,T> operator/(const Vec<1,T>& x, const Vec<1,T>& y) { return x.val / y.val; }

    SIT Vec<1,T> operator^(const Vec<1,T>& x, const Vec<1,T>& y) { return x.val ^ y.val; }
    SIT Vec<1,T> operator&(const Vec<1,T>& x, const Vec<1,T>& y) { return x.val & y.val; }
    SIT Vec<1,T> operator|(const Vec<1,T>& x, const Vec<1,T>& y) { return x.val | y.val; }

    SIT Vec<1,T> operator!(const Vec<1,T>& x) { return !x.val; }
    SIT Vec<1,T> operator-(const Vec<1,T>& x) { return -x.val; }
    SIT Vec<1,T> operator~(const Vec<1,T>& x) { return ~x.val; }

    SIT Vec<1,T> operator<<(const Vec<1,T>& x, int k) { return x.val << k; }
    SIT Vec<1,T> operator>>(const Vec<1,T>& x, int k) { return x.val >> k; }

    SIT Vec<1,M<T>> operator==(const Vec<1,T>& x, const Vec<1,T>& y) {
        return x.val == y.val ? ~0 : 0;
    }
    SIT Vec<1,M<T>> operator!=(const Vec<1,T>& x, const Vec<1,T>& y) {
        return x.val != y.val ? ~0 : 0;
    }
    SIT Vec<1,M<T>> operator<=(const Vec<1,T>& x, const Vec<1,T>& y) {
        return x.val <= y.val ? ~0 : 0;
    }
    SIT Vec<1,M<T>> operator>=(const Vec<1,T>& x, const Vec<1,T>& y) {
        return x.val >= y.val ? ~0 : 0;
    }
    SIT Vec<1,M<T>> operator< (const Vec<1,T>& x, const Vec<1,T>& y) {
        return x.val <  y.val ? ~0 : 0;
    }
    SIT Vec<1,M<T>> operator> (const Vec<1,T>& x, const Vec<1,T>& y) {
        return x.val >  y.val ? ~0 : 0;
    }

    // Recurse on lo/hi down to N==1 scalar implementations.
    SINT Vec<N,T> operator+(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo + y.lo, x.hi + y.hi);
    }
    SINT Vec<N,T> operator-(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo - y.lo, x.hi - y.hi);
    }
    SINT Vec<N,T> operator*(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo * y.lo, x.hi * y.hi);
    }
    SINT Vec<N,T> operator/(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo / y.lo, x.hi / y.hi);
    }

    SINT Vec<N,T> operator^(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo ^ y.lo, x.hi ^ y.hi);
    }
    SINT Vec<N,T> operator&(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo & y.lo, x.hi & y.hi);
    }
    SINT Vec<N,T> operator|(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo | y.lo, x.hi | y.hi);
    }

    SINT Vec<N,T> operator!(const Vec<N,T>& x) { return join(!x.lo, !x.hi); }
    SINT Vec<N,T> operator-(const Vec<N,T>& x) { return join(-x.lo, -x.hi); }
    SINT Vec<N,T> operator~(const Vec<N,T>& x) { return join(~x.lo, ~x.hi); }

    SINT Vec<N,T> operator<<(const Vec<N,T>& x, int k) { return join(x.lo << k, x.hi << k); }
    SINT Vec<N,T> operator>>(const Vec<N,T>& x, int k) { return join(x.lo >> k, x.hi >> k); }

    SINT Vec<N,M<T>> operator==(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo == y.lo, x.hi == y.hi);
    }
    SINT Vec<N,M<T>> operator!=(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo != y.lo, x.hi != y.hi);
    }
    SINT Vec<N,M<T>> operator<=(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo <= y.lo, x.hi <= y.hi);
    }
    SINT Vec<N,M<T>> operator>=(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo >= y.lo, x.hi >= y.hi);
    }
    SINT Vec<N,M<T>> operator< (const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo <  y.lo, x.hi <  y.hi);
    }
    SINT Vec<N,M<T>> operator> (const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo >  y.lo, x.hi >  y.hi);
    }
#endif

// Scalar/vector operations splat the scalar to a vector.
SINTU Vec<N,T>    operator+ (U x, const Vec<N,T>& y) { return Vec<N,T>(x) +  y; }
SINTU Vec<N,T>    operator- (U x, const Vec<N,T>& y) { return Vec<N,T>(x) -  y; }
SINTU Vec<N,T>    operator* (U x, const Vec<N,T>& y) { return Vec<N,T>(x) *  y; }
SINTU Vec<N,T>    operator/ (U x, const Vec<N,T>& y) { return Vec<N,T>(x) /  y; }
SINTU Vec<N,T>    operator^ (U x, const Vec<N,T>& y) { return Vec<N,T>(x) ^  y; }
SINTU Vec<N,T>    operator& (U x, const Vec<N,T>& y) { return Vec<N,T>(x) &  y; }
SINTU Vec<N,T>    operator| (U x, const Vec<N,T>& y) { return Vec<N,T>(x) |  y; }
SINTU Vec<N,M<T>> operator==(U x, const Vec<N,T>& y) { return Vec<N,T>(x) == y; }
SINTU Vec<N,M<T>> operator!=(U x, const Vec<N,T>& y) { return Vec<N,T>(x) != y; }
SINTU Vec<N,M<T>> operator<=(U x, const Vec<N,T>& y) { return Vec<N,T>(x) <= y; }
SINTU Vec<N,M<T>> operator>=(U x, const Vec<N,T>& y) { return Vec<N,T>(x) >= y; }
SINTU Vec<N,M<T>> operator< (U x, const Vec<N,T>& y) { return Vec<N,T>(x) <  y; }
SINTU Vec<N,M<T>> operator> (U x, const Vec<N,T>& y) { return Vec<N,T>(x) >  y; }

SINTU Vec<N,T>    operator+ (const Vec<N,T>& x, U y) { return x +  Vec<N,T>(y); }
SINTU Vec<N,T>    operator- (const Vec<N,T>& x, U y) { return x -  Vec<N,T>(y); }
SINTU Vec<N,T>    operator* (const Vec<N,T>& x, U y) { return x *  Vec<N,T>(y); }
SINTU Vec<N,T>    operator/ (const Vec<N,T>& x, U y) { return x /  Vec<N,T>(y); }
SINTU Vec<N,T>    operator^ (const Vec<N,T>& x, U y) { return x ^  Vec<N,T>(y); }
SINTU Vec<N,T>    operator& (const Vec<N,T>& x, U y) { return x &  Vec<N,T>(y); }
SINTU Vec<N,T>    operator| (const Vec<N,T>& x, U y) { return x |  Vec<N,T>(y); }
SINTU Vec<N,M<T>> operator==(const Vec<N,T>& x, U y) { return x == Vec<N,T>(y); }
SINTU Vec<N,M<T>> operator!=(const Vec<N,T>& x, U y) { return x != Vec<N,T>(y); }
SINTU Vec<N,M<T>> operator<=(const Vec<N,T>& x, U y) { return x <= Vec<N,T>(y); }
SINTU Vec<N,M<T>> operator>=(const Vec<N,T>& x, U y) { return x >= Vec<N,T>(y); }
SINTU Vec<N,M<T>> operator< (const Vec<N,T>& x, U y) { return x <  Vec<N,T>(y); }
SINTU Vec<N,M<T>> operator> (const Vec<N,T>& x, U y) { return x >  Vec<N,T>(y); }

SINT Vec<N,T>& operator+=(Vec<N,T>& x, const Vec<N,T>& y) { return (x = x + y); }
SINT Vec<N,T>& operator-=(Vec<N,T>& x, const Vec<N,T>& y) { return (x = x - y); }
SINT Vec<N,T>& operator*=(Vec<N,T>& x, const Vec<N,T>& y) { return (x = x * y); }
SINT Vec<N,T>& operator/=(Vec<N,T>& x, const Vec<N,T>& y) { return (x = x / y); }
SINT Vec<N,T>& operator^=(Vec<N,T>& x, const Vec<N,T>& y) { return (x = x ^ y); }
SINT Vec<N,T>& operator&=(Vec<N,T>& x, const Vec<N,T>& y) { return (x = x & y); }
SINT Vec<N,T>& operator|=(Vec<N,T>& x, const Vec<N,T>& y) { return (x = x | y); }

SINTU Vec<N,T>& operator+=(Vec<N,T>& x, U y) { return (x = x + Vec<N,T>(y)); }
SINTU Vec<N,T>& operator-=(Vec<N,T>& x, U y) { return (x = x - Vec<N,T>(y)); }
SINTU Vec<N,T>& operator*=(Vec<N,T>& x, U y) { return (x = x * Vec<N,T>(y)); }
SINTU Vec<N,T>& operator/=(Vec<N,T>& x, U y) { return (x = x / Vec<N,T>(y)); }
SINTU Vec<N,T>& operator^=(Vec<N,T>& x, U y) { return (x = x ^ Vec<N,T>(y)); }
SINTU Vec<N,T>& operator&=(Vec<N,T>& x, U y) { return (x = x & Vec<N,T>(y)); }
SINTU Vec<N,T>& operator|=(Vec<N,T>& x, U y) { return (x = x | Vec<N,T>(y)); }

SINT Vec<N,T>& operator<<=(Vec<N,T>& x, int bits) { return (x = x << bits); }
SINT Vec<N,T>& operator>>=(Vec<N,T>& x, int bits) { return (x = x >> bits); }

// Some operations we want are not expressible with Clang/GCC vector extensions.

// Clang can reason about naive_if_then_else() and optimize through it better
// than if_then_else(), so it's sometimes useful to call it directly when we
// think an entire expression should optimize away, e.g. min()/max().
SINT Vec<N,T> naive_if_then_else(const Vec<N,M<T>>& cond, const Vec<N,T>& t, const Vec<N,T>& e) {
    return sk_bit_cast<Vec<N,T>>(( cond & sk_bit_cast<Vec<N, M<T>>>(t)) |
                                 (~cond & sk_bit_cast<Vec<N, M<T>>>(e)) );
}

SIT Vec<1,T> if_then_else(const Vec<1,M<T>>& cond, const Vec<1,T>& t, const Vec<1,T>& e) {
    // In practice this scalar implementation is unlikely to be used.  See next if_then_else().
    return sk_bit_cast<Vec<1,T>>(( cond & sk_bit_cast<Vec<1, M<T>>>(t)) |
                                 (~cond & sk_bit_cast<Vec<1, M<T>>>(e)) );
}
SINT Vec<N,T> if_then_else(const Vec<N,M<T>>& cond, const Vec<N,T>& t, const Vec<N,T>& e) {
    // Specializations inline here so they can generalize what types the apply to.
#if SKVX_USE_SIMD && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    if constexpr (N*sizeof(T) == 32) {
        return sk_bit_cast<Vec<N,T>>(_mm256_blendv_epi8(sk_bit_cast<__m256i>(e),
                                                        sk_bit_cast<__m256i>(t),
                                                        sk_bit_cast<__m256i>(cond)));
    }
#endif
#if SKVX_USE_SIMD && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
    if constexpr (N*sizeof(T) == 16) {
        return sk_bit_cast<Vec<N,T>>(_mm_blendv_epi8(sk_bit_cast<__m128i>(e),
                                                     sk_bit_cast<__m128i>(t),
                                                     sk_bit_cast<__m128i>(cond)));
    }
#endif
#if SKVX_USE_SIMD && defined(SK_ARM_HAS_NEON)
    if constexpr (N*sizeof(T) == 16) {
        return sk_bit_cast<Vec<N,T>>(vbslq_u8(sk_bit_cast<uint8x16_t>(cond),
                                              sk_bit_cast<uint8x16_t>(t),
                                              sk_bit_cast<uint8x16_t>(e)));
    }
#endif
#if SKVX_USE_SIMD && SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    if constexpr (N*sizeof(T) == 32) {
        return sk_bit_cast<Vec<N,T>>(__lasx_xvbitsel_v(sk_bit_cast<__m256i>(e),
                                                       sk_bit_cast<__m256i>(t),
                                                       sk_bit_cast<__m256i>(cond)));
    }
#endif
#if SKVX_USE_SIMD && SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    if constexpr (N*sizeof(T) == 16) {
        return sk_bit_cast<Vec<N,T>>(__lsx_vbitsel_v(sk_bit_cast<__m128i>(e),
                                                     sk_bit_cast<__m128i>(t),
                                                     sk_bit_cast<__m128i>(cond)));
    }
#endif
    // Recurse for large vectors to try to hit the specializations above.
    if constexpr (N*sizeof(T) > 16) {
        return join(if_then_else(cond.lo, t.lo, e.lo),
                    if_then_else(cond.hi, t.hi, e.hi));
    }
    // This default can lead to better code than the recursing onto scalars.
    return naive_if_then_else(cond, t, e);
}

SIT  bool any(const Vec<1,T>& x) { return x.val != 0; }
SINT bool any(const Vec<N,T>& x) {
    // For any(), the _mm_testz intrinsics are correct and don't require comparing 'x' to 0, so it's
    // lower latency compared to _mm_movemask + _mm_compneq on plain SSE.
#if SKVX_USE_SIMD && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    if constexpr (N*sizeof(T) == 32) {
        return !_mm256_testz_si256(sk_bit_cast<__m256i>(x), _mm256_set1_epi32(-1));
    }
#endif
#if SKVX_USE_SIMD && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
    if constexpr (N*sizeof(T) == 16) {
        return !_mm_testz_si128(sk_bit_cast<__m128i>(x), _mm_set1_epi32(-1));
    }
#endif
#if SKVX_USE_SIMD && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
    if constexpr (N*sizeof(T) == 16) {
        // On SSE, movemask checks only the MSB in each lane, which is fine if the lanes were set
        // directly from a comparison op (which sets all bits to 1 when true), but skvx::Vec<>
        // treats any non-zero value as true, so we have to compare 'x' to 0 before calling movemask
        return _mm_movemask_ps(_mm_cmpneq_ps(sk_bit_cast<__m128>(x), _mm_set1_ps(0))) != 0b0000;
    }
#endif
#if SKVX_USE_SIMD && defined(__aarch64__)
    // On 64-bit NEON, take the max across lanes, which will be non-zero if any lane was true.
    // The specific lane-size doesn't really matter in this case since it's really any set bit
    // that we're looking for.
    if constexpr (N*sizeof(T) == 8 ) { return vmaxv_u8 (sk_bit_cast<uint8x8_t> (x)) > 0; }
    if constexpr (N*sizeof(T) == 16) { return vmaxvq_u8(sk_bit_cast<uint8x16_t>(x)) > 0; }
#endif
#if SKVX_USE_SIMD && defined(__wasm_simd128__)
    if constexpr (N == 4 && sizeof(T) == 4) {
        return wasm_i32x4_any_true(sk_bit_cast<VExt<4,int>>(x));
    }
#endif
#if SKVX_USE_SIMD && SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    if constexpr (N*sizeof(T) == 32) {
        v8i32 retv = (v8i32)__lasx_xvmskltz_w(__lasx_xvslt_wu(__lasx_xvldi(0),
                                                              sk_bit_cast<__m256i>(x)));
        return (retv[0] | retv[4]) != 0b0000;
    }
#endif
#if SKVX_USE_SIMD && SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    if constexpr (N*sizeof(T) == 16) {
        v4i32 retv = (v4i32)__lsx_vmskltz_w(__lsx_vslt_wu(__lsx_vldi(0),
                                                          sk_bit_cast<__m128i>(x)));
        return retv[0] != 0b0000;
    }
#endif
    return any(x.lo)
        || any(x.hi);
}

SIT  bool all(const Vec<1,T>& x) { return x.val != 0; }
SINT bool all(const Vec<N,T>& x) {
// Unlike any(), we have to respect the lane layout, or we'll miss cases where a
// true lane has a mix of 0 and 1 bits.
#if SKVX_USE_SIMD && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
    // Unfortunately, the _mm_testc intrinsics don't let us avoid the comparison to 0 for all()'s
    // correctness, so always just use the plain SSE version.
    if constexpr (N == 4 && sizeof(T) == 4) {
        return _mm_movemask_ps(_mm_cmpneq_ps(sk_bit_cast<__m128>(x), _mm_set1_ps(0))) == 0b1111;
    }
#endif
#if SKVX_USE_SIMD && defined(__aarch64__)
    // On 64-bit NEON, take the min across the lanes, which will be non-zero if all lanes are != 0.
    if constexpr (sizeof(T)==1 && N==8)  {return vminv_u8  (sk_bit_cast<uint8x8_t> (x)) > 0;}
    if constexpr (sizeof(T)==1 && N==16) {return vminvq_u8 (sk_bit_cast<uint8x16_t>(x)) > 0;}
    if constexpr (sizeof(T)==2 && N==4)  {return vminv_u16 (sk_bit_cast<uint16x4_t>(x)) > 0;}
    if constexpr (sizeof(T)==2 && N==8)  {return vminvq_u16(sk_bit_cast<uint16x8_t>(x)) > 0;}
    if constexpr (sizeof(T)==4 && N==2)  {return vminv_u32 (sk_bit_cast<uint32x2_t>(x)) > 0;}
    if constexpr (sizeof(T)==4 && N==4)  {return vminvq_u32(sk_bit_cast<uint32x4_t>(x)) > 0;}
#endif
#if SKVX_USE_SIMD && defined(__wasm_simd128__)
    if constexpr (N == 4 && sizeof(T) == 4) {
        return wasm_i32x4_all_true(sk_bit_cast<VExt<4,int>>(x));
    }
#endif
#if SKVX_USE_SIMD && SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    if constexpr (N == 8 && sizeof(T) == 4) {
        v8i32 retv = (v8i32)__lasx_xvmskltz_w(__lasx_xvslt_wu(__lasx_xvldi(0),
                                                              sk_bit_cast<__m256i>(x)));
        return (retv[0] & retv[4]) == 0b1111;
    }
#endif
#if SKVX_USE_SIMD && SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    if constexpr (N == 4 && sizeof(T) == 4) {
        v4i32 retv = (v4i32)__lsx_vmskltz_w(__lsx_vslt_wu(__lsx_vldi(0),
                                                          sk_bit_cast<__m128i>(x)));
        return retv[0] == 0b1111;
    }
#endif
    return all(x.lo)
        && all(x.hi);
}

// cast() Vec<N,S> to Vec<N,D>, as if applying a C-cast to each lane.
// TODO: implement with map()?
template <typename D, typename S>
SI Vec<1,D> cast(const Vec<1,S>& src) { return (D)src.val; }

template <typename D, int N, typename S>
SI Vec<N,D> cast(const Vec<N,S>& src) {
#if SKVX_USE_SIMD && defined(__clang__)
    return to_vec(__builtin_convertvector(to_vext(src), VExt<N,D>));
#else
    return join(cast<D>(src.lo), cast<D>(src.hi));
#endif
}

// min/max match logic of std::min/std::max, which is important when NaN is involved.
SIT  T min(const Vec<1,T>& x) { return x.val; }
SIT  T max(const Vec<1,T>& x) { return x.val; }
SINT T min(const Vec<N,T>& x) { return std::min(min(x.lo), min(x.hi)); }
SINT T max(const Vec<N,T>& x) { return std::max(max(x.lo), max(x.hi)); }

SINT Vec<N,T> min(const Vec<N,T>& x, const Vec<N,T>& y) { return naive_if_then_else(y < x, y, x); }
SINT Vec<N,T> max(const Vec<N,T>& x, const Vec<N,T>& y) { return naive_if_then_else(x < y, y, x); }

SINTU Vec<N,T> min(const Vec<N,T>& x, U y) { return min(x, Vec<N,T>(y)); }
SINTU Vec<N,T> max(const Vec<N,T>& x, U y) { return max(x, Vec<N,T>(y)); }
SINTU Vec<N,T> min(U x, const Vec<N,T>& y) { return min(Vec<N,T>(x), y); }
SINTU Vec<N,T> max(U x, const Vec<N,T>& y) { return max(Vec<N,T>(x), y); }

// pin matches the logic of SkTPin, which is important when NaN is involved. It always returns
// values in the range lo..hi, and if x is NaN, it returns lo.
SINT Vec<N,T> pin(const Vec<N,T>& x, const Vec<N,T>& lo, const Vec<N,T>& hi) {
    return max(lo, min(x, hi));
}

// Shuffle values from a vector pretty arbitrarily:
//    skvx::Vec<4,float> rgba = {R,G,B,A};
//    shuffle<2,1,0,3>        (rgba) ~> {B,G,R,A}
//    shuffle<2,1>            (rgba) ~> {B,G}
//    shuffle<2,1,2,1,2,1,2,1>(rgba) ~> {B,G,B,G,B,G,B,G}
//    shuffle<3,3,3,3>        (rgba) ~> {A,A,A,A}
// The only real restriction is that the output also be a legal N=power-of-two sknx::Vec.
template <int... Ix, int N, typename T>
SI Vec<sizeof...(Ix),T> shuffle(const Vec<N,T>& x) {
#if SKVX_USE_SIMD && defined(__clang__)
    // TODO: can we just always use { x[Ix]... }?
    return to_vec<sizeof...(Ix),T>(__builtin_shufflevector(to_vext(x), to_vext(x), Ix...));
#else
    return { x[Ix]... };
#endif
}

// Call map(fn, x) for a vector with fn() applied to each lane of x, { fn(x[0]), fn(x[1]), ... },
// or map(fn, x,y) for a vector of fn(x[i], y[i]), etc.

template <typename Fn, typename... Args, size_t... I>
SI auto map(std::index_sequence<I...>,
            Fn&& fn, const Args&... args) -> skvx::Vec<sizeof...(I), decltype(fn(args[0]...))> {
    auto lane = [&](size_t i)
    // CFI, specifically -fsanitize=cfi-icall, seems to give a false positive here,
    // with errors like "control flow integrity check for type 'float (float)
    // noexcept' failed during indirect function call... note: sqrtf.cfi_jt defined
    // here".  But we can be quite sure fn is the right type: it's all inferred!
    // So, stifle CFI in this function.
    SK_NO_SANITIZE_CFI
    { return fn(args[static_cast<int>(i)]...); };

    return { lane(I)... };
}

template <typename Fn, int N, typename T, typename... Rest>
auto map(Fn&& fn, const Vec<N,T>& first, const Rest&... rest) {
    // Derive an {0...N-1} index_sequence from the size of the first arg: N lanes in, N lanes out.
    return map(std::make_index_sequence<N>{}, fn, first,rest...);
}

SIN Vec<N,float>  ceil(const Vec<N,float>& x) { return map( ceilf, x); }
SIN Vec<N,float> floor(const Vec<N,float>& x) { return map(floorf, x); }
SIN Vec<N,float> trunc(const Vec<N,float>& x) { return map(truncf, x); }
SIN Vec<N,float> round(const Vec<N,float>& x) { return map(roundf, x); }
SIN Vec<N,float>  sqrt(const Vec<N,float>& x) { return map( sqrtf, x); }
SIN Vec<N,float>   abs(const Vec<N,float>& x) { return map( fabsf, x); }
SIN Vec<N,float>   fma(const Vec<N,float>& x,
                       const Vec<N,float>& y,
                       const Vec<N,float>& z) {
    // I don't understand why Clang's codegen is terrible if we write map(fmaf, x,y,z) directly.
    auto fn = [](float x, float y, float z) { return fmaf(x,y,z); };
    return map(fn, x,y,z);
}

SI Vec<1,int> lrint(const Vec<1,float>& x) {
    return (int)lrintf(x.val);
}
SIN Vec<N,int> lrint(const Vec<N,float>& x) {
#if SKVX_USE_SIMD && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX
    if constexpr (N == 8) {
        return sk_bit_cast<Vec<N,int>>(_mm256_cvtps_epi32(sk_bit_cast<__m256>(x)));
    }
#endif
#if SKVX_USE_SIMD && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
    if constexpr (N == 4) {
        return sk_bit_cast<Vec<N,int>>(_mm_cvtps_epi32(sk_bit_cast<__m128>(x)));
    }
#endif
#if SKVX_USE_SIMD && SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    if constexpr (N == 8) {
        return sk_bit_cast<Vec<N,int>>(__lasx_xvftint_w_s(sk_bit_cast<__m256>(x)));
    }
#endif
#if SKVX_USE_SIMD && SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    if constexpr (N == 4) {
        return sk_bit_cast<Vec<N,int>>(__lsx_vftint_w_s(sk_bit_cast<__m128>(x)));
    }
#endif
    return join(lrint(x.lo),
                lrint(x.hi));
}

SIN Vec<N,float> fract(const Vec<N,float>& x) { return x - floor(x); }

// Converts float to half, rounding to nearest even, and supporting de-normal f16 conversion,
// and overflow to f16 infinity. Should not be called with NaNs, since it can convert NaN->inf.
// KEEP IN SYNC with skcms' Half_from_F to ensure that f16 colors are computed consistently in both
// skcms and skvx.
SIN Vec<N,uint16_t> to_half(const Vec<N,float>& x) {
    assert(all(x == x)); // No NaNs should reach this function

    // Intrinsics for float->half tend to operate on 4 lanes, and the default implementation has
    // enough instructions that it's better to split and join on 128 bits groups vs.
    // recursing for each min/max/shift/etc.
    if constexpr (N > 4) {
        return join(to_half(x.lo),
                    to_half(x.hi));
    }

#if SKVX_USE_SIMD && defined(__aarch64__)
    if constexpr (N == 4) {
        return sk_bit_cast<Vec<N,uint16_t>>(vcvt_f16_f32(sk_bit_cast<float32x4_t>(x)));

    }
#endif

#define I(x) sk_bit_cast<Vec<N,int32_t>>(x)
#define F(x) sk_bit_cast<Vec<N,float>>(x)
    Vec<N,int32_t> sem = I(x),
                   s   = sem & 0x8000'0000,
                    em = min(sem ^ s, 0x4780'0000), // |x| clamped to f16 infinity
                 // F(em)*8192 increases the exponent by 13, which when added back to em will shift
                 // the mantissa bits 13 to the right. We clamp to 1/2 for subnormal values, which
                 // automatically shifts the mantissa to match 2^-14 expected for a subnorm f16.
                 magic = I(max(F(em) * 8192.f, 0.5f)) & (255 << 23),
               rounded = I((F(em) + F(magic))), // shift mantissa with automatic round-to-even
                   // Subtract 127 for f32 bias, subtract 13 to undo the *8192, subtract 1 to remove
                   // the implicit leading 1., and add 15 to get the f16 biased exponent.
                   exp = ((magic >> 13) - ((127-15+13+1)<<10)), // shift and re-bias exponent
                   f16 = rounded + exp; // use + if 'rounded' rolled over into first exponent bit
    return cast<uint16_t>((s>>16) | f16);
#undef I
#undef F
}

// Converts from half to float, preserving NaN and +/- infinity.
// KEEP IN SYNC with skcms' F_from_Half to ensure that f16 colors are computed consistently in both
// skcms and skvx.
SIN Vec<N,float> from_half(const Vec<N,uint16_t>& x) {
    if constexpr (N > 4) {
        return join(from_half(x.lo),
                    from_half(x.hi));
    }

#if SKVX_USE_SIMD && defined(__aarch64__)
    if constexpr (N == 4) {
        return sk_bit_cast<Vec<N,float>>(vcvt_f32_f16(sk_bit_cast<float16x4_t>(x)));
    }
#endif

    Vec<N,int32_t> wide = cast<int32_t>(x),
                      s  = wide & 0x8000,
                      em = wide ^ s,
              inf_or_nan =  (em >= (31 << 10)) & (255 << 23),  // Expands exponent to fill 8 bits
                 is_norm =   em > 0x3ff,
                     // subnormal f16's are 2^-14*0.[m0:9] == 2^-24*[m0:9].0
                     sub = sk_bit_cast<Vec<N,int32_t>>((cast<float>(em) * (1.f/(1<<24)))),
                    norm = ((em<<13) + ((127-15)<<23)), // Shifts mantissa, shifts + re-biases exp
                  finite = (is_norm & norm) | (~is_norm & sub);
    // If 'x' is f16 +/- infinity, inf_or_nan will be the filled 8-bit exponent but 'norm' will be
    // all 0s since 'x's mantissa is 0. Thus norm | inf_or_nan becomes f32 infinity. However, if
    // 'x' is an f16 NaN, some bits of 'norm' will be non-zero, so it stays an f32 NaN after the OR.
    return sk_bit_cast<Vec<N,float>>((s<<16) | finite | inf_or_nan);
}

// div255(x) = (x + 127) / 255 is a bit-exact rounding divide-by-255, packing down to 8-bit.
SIN Vec<N,uint8_t> div255(const Vec<N,uint16_t>& x) {
    return cast<uint8_t>( (x+127)/255 );
}

// approx_scale(x,y) approximates div255(cast<uint16_t>(x)*cast<uint16_t>(y)) within a bit,
// and is always perfect when x or y is 0 or 255.
SIN Vec<N,uint8_t> approx_scale(const Vec<N,uint8_t>& x, const Vec<N,uint8_t>& y) {
    // All of (x*y+x)/256, (x*y+y)/256, and (x*y+255)/256 meet the criteria above.
    // We happen to have historically picked (x*y+x)/256.
    auto X = cast<uint16_t>(x),
         Y = cast<uint16_t>(y);
    return cast<uint8_t>( (X*Y+X)/256 );
}

// saturated_add(x,y) sums values and clamps to the maximum value instead of overflowing.
SINT std::enable_if_t<std::is_unsigned_v<T>, Vec<N,T>> saturated_add(const Vec<N,T>& x,
                                                                     const Vec<N,T>& y) {
#if SKVX_USE_SIMD && (SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1 || defined(SK_ARM_HAS_NEON) || \
        SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX)
    // Both SSE and ARM have 16-lane saturated adds, so use intrinsics for those and recurse down
    // or join up to take advantage.
    if constexpr (N == 16 && sizeof(T) == 1) {
        #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
        return sk_bit_cast<Vec<N,T>>(_mm_adds_epu8(sk_bit_cast<__m128i>(x),
                                                   sk_bit_cast<__m128i>(y)));
        #elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
        return sk_bit_cast<Vec<N,T>>(__lsx_vsadd_bu(sk_bit_cast<__m128i>(x),
                                                    sk_bit_cast<__m128i>(y)));
        #else  // SK_ARM_HAS_NEON
        return sk_bit_cast<Vec<N,T>>(vqaddq_u8(sk_bit_cast<uint8x16_t>(x),
                                               sk_bit_cast<uint8x16_t>(y)));
        #endif
    } else if constexpr (N < 16 && sizeof(T) == 1) {
        return saturated_add(join(x,x), join(y,y)).lo;
    } else if constexpr (sizeof(T) == 1) {
        return join(saturated_add(x.lo, y.lo), saturated_add(x.hi, y.hi));
    }
#endif
    // Otherwise saturate manually
    auto sum = x + y;
    return if_then_else(sum < x, Vec<N,T>(std::numeric_limits<T>::max()), sum);
}

// The ScaledDividerU32 takes a divisor > 1, and creates a function divide(numerator) that
// calculates a numerator / denominator. For this to be rounded properly, numerator should have
// half added in:
// divide(numerator + half) == floor(numerator/denominator + 1/2).
//
// This gives an answer within +/- 1 from the true value.
//
// Derivation of half:
//    numerator/denominator + 1/2 = (numerator + half) / d
//    numerator + denominator / 2 = numerator + half
//    half = denominator / 2.
//
// Because half is divided by 2, that division must also be rounded.
//    half == denominator / 2 = (denominator + 1) / 2.
//
// The divisorFactor is just a scaled value:
//    divisorFactor = (1 / divisor) * 2 ^ 32.
// The maximum that can be divided and rounded is UINT_MAX - half.
class ScaledDividerU32 {
public:
    explicit ScaledDividerU32(uint32_t divisor)
            : fDivisorFactor{(uint32_t)(std::round((1.0 / divisor) * (1ull << 32)))}
            , fHalf{(divisor + 1) >> 1} {
        assert(divisor > 1);
    }

    Vec<4, uint32_t> divide(const Vec<4, uint32_t>& numerator) const {
#if SKVX_USE_SIMD && defined(SK_ARM_HAS_NEON)
        uint64x2_t hi = vmull_n_u32(vget_high_u32(to_vext(numerator)), fDivisorFactor);
        uint64x2_t lo = vmull_n_u32(vget_low_u32(to_vext(numerator)),  fDivisorFactor);

        return to_vec<4, uint32_t>(vcombine_u32(vshrn_n_u64(lo,32), vshrn_n_u64(hi,32)));
#else
        return cast<uint32_t>((cast<uint64_t>(numerator) * fDivisorFactor) >> 32);
#endif
    }

    uint32_t half() const { return fHalf; }
    uint32_t divisorFactor() const { return fDivisorFactor; }

private:
    const uint32_t fDivisorFactor;
    const uint32_t fHalf;
};


SIN Vec<N,uint16_t> mull(const Vec<N,uint8_t>& x,
                         const Vec<N,uint8_t>& y) {
#if SKVX_USE_SIMD && defined(SK_ARM_HAS_NEON)
    // With NEON we can do eight u8*u8 -> u16 in one instruction, vmull_u8 (read, mul-long).
    if constexpr (N == 8) {
        return to_vec<8,uint16_t>(vmull_u8(to_vext(x), to_vext(y)));
    } else if constexpr (N < 8) {
        return mull(join(x,x), join(y,y)).lo;
    } else { // N > 8
        return join(mull(x.lo, y.lo), mull(x.hi, y.hi));
    }
#else
    return cast<uint16_t>(x) * cast<uint16_t>(y);
#endif
}

SIN Vec<N,uint32_t> mull(const Vec<N,uint16_t>& x,
                         const Vec<N,uint16_t>& y) {
#if SKVX_USE_SIMD && defined(SK_ARM_HAS_NEON)
    // NEON can do four u16*u16 -> u32 in one instruction, vmull_u16
    if constexpr (N == 4) {
        return to_vec<4,uint32_t>(vmull_u16(to_vext(x), to_vext(y)));
    } else if constexpr (N < 4) {
        return mull(join(x,x), join(y,y)).lo;
    } else { // N > 4
        return join(mull(x.lo, y.lo), mull(x.hi, y.hi));
    }
#else
    return cast<uint32_t>(x) * cast<uint32_t>(y);
#endif
}

SIN Vec<N,uint16_t> mulhi(const Vec<N,uint16_t>& x,
                          const Vec<N,uint16_t>& y) {
#if SKVX_USE_SIMD && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
    // Use _mm_mulhi_epu16 for 8xuint16_t and join or split to get there.
    if constexpr (N == 8) {
        return sk_bit_cast<Vec<8,uint16_t>>(_mm_mulhi_epu16(sk_bit_cast<__m128i>(x),
                                                            sk_bit_cast<__m128i>(y)));
    } else if constexpr (N < 8) {
        return mulhi(join(x,x), join(y,y)).lo;
    } else { // N > 8
        return join(mulhi(x.lo, y.lo), mulhi(x.hi, y.hi));
    }
#elif SKVX_USE_SIMD && SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    if constexpr (N == 8) {
        return sk_bit_cast<Vec<8,uint16_t>>(__lsx_vmuh_hu(sk_bit_cast<__m128i>(x),
                                                          sk_bit_cast<__m128i>(y)));
    } else if constexpr (N < 8) {
        return mulhi(join(x,x), join(y,y)).lo;
    } else { // N > 8
        return join(mulhi(x.lo, y.lo), mulhi(x.hi, y.hi));
    }
#else
    return skvx::cast<uint16_t>(mull(x, y) >> 16);
#endif
}

SINT T dot(const Vec<N, T>& a, const Vec<N, T>& b) {
    // While dot is a "horizontal" operation like any or all, it needs to remain
    // in floating point and there aren't really any good SIMD instructions that make it faster.
    // The constexpr cases remove the for loop in the only cases we realistically call.
    auto ab = a*b;
    if constexpr (N == 2) {
        return ab[0] + ab[1];
    } else if constexpr (N == 4) {
        return ab[0] + ab[1] + ab[2] + ab[3];
    } else {
        T sum = ab[0];
        for (int i = 1; i < N; ++i) {
            sum += ab[i];
        }
        return sum;
    }
}

SIT T cross(const Vec<2, T>& a, const Vec<2, T>& b) {
    auto x = a * shuffle<1,0>(b);
    return x[0] - x[1];
}

SIN float length(const Vec<N, float>& v) {
    return std::sqrt(dot(v, v));
}

SIN double length(const Vec<N, double>& v) {
    return std::sqrt(dot(v, v));
}

SIN Vec<N, float> normalize(const Vec<N, float>& v) {
    return v / length(v);
}

SIN Vec<N, double> normalize(const Vec<N, double>& v) {
    return v / length(v);
}

SINT bool isfinite(const Vec<N, T>& v) {
    // Multiply all values together with 0. If they were all finite, the output is
    // 0 (also finite). If any were not, we'll get nan.
    return SkIsFinite(dot(v, Vec<N, T>(0)));
}

// De-interleaving load of 4 vectors.
//
// WARNING: These are really only supported well on NEON. Consider restructuring your data before
// resorting to these methods.
SIT void strided_load4(const T* v,
                       Vec<1,T>& a,
                       Vec<1,T>& b,
                       Vec<1,T>& c,
                       Vec<1,T>& d) {
    a.val = v[0];
    b.val = v[1];
    c.val = v[2];
    d.val = v[3];
}
SINT void strided_load4(const T* v,
                        Vec<N,T>& a,
                        Vec<N,T>& b,
                        Vec<N,T>& c,
                        Vec<N,T>& d) {
    strided_load4(v, a.lo, b.lo, c.lo, d.lo);
    strided_load4(v + 4*(N/2), a.hi, b.hi, c.hi, d.hi);
}
#if SKVX_USE_SIMD && defined(SK_ARM_HAS_NEON)
#define IMPL_LOAD4_TRANSPOSED(N, T, VLD) \
SI void strided_load4(const T* v, \
                      Vec<N,T>& a, \
                      Vec<N,T>& b, \
                      Vec<N,T>& c, \
                      Vec<N,T>& d) { \
    auto mat = VLD(v); \
    a = sk_bit_cast<Vec<N,T>>(mat.val[0]); \
    b = sk_bit_cast<Vec<N,T>>(mat.val[1]); \
    c = sk_bit_cast<Vec<N,T>>(mat.val[2]); \
    d = sk_bit_cast<Vec<N,T>>(mat.val[3]); \
}
IMPL_LOAD4_TRANSPOSED(2, uint32_t, vld4_u32)
IMPL_LOAD4_TRANSPOSED(4, uint16_t, vld4_u16)
IMPL_LOAD4_TRANSPOSED(8, uint8_t, vld4_u8)
IMPL_LOAD4_TRANSPOSED(2, int32_t, vld4_s32)
IMPL_LOAD4_TRANSPOSED(4, int16_t, vld4_s16)
IMPL_LOAD4_TRANSPOSED(8, int8_t, vld4_s8)
IMPL_LOAD4_TRANSPOSED(2, float, vld4_f32)
IMPL_LOAD4_TRANSPOSED(4, uint32_t, vld4q_u32)
IMPL_LOAD4_TRANSPOSED(8, uint16_t, vld4q_u16)
IMPL_LOAD4_TRANSPOSED(16, uint8_t, vld4q_u8)
IMPL_LOAD4_TRANSPOSED(4, int32_t, vld4q_s32)
IMPL_LOAD4_TRANSPOSED(8, int16_t, vld4q_s16)
IMPL_LOAD4_TRANSPOSED(16, int8_t, vld4q_s8)
IMPL_LOAD4_TRANSPOSED(4, float, vld4q_f32)
#undef IMPL_LOAD4_TRANSPOSED

#elif SKVX_USE_SIMD && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1

SI void strided_load4(const float* v,
                      Vec<4,float>& a,
                      Vec<4,float>& b,
                      Vec<4,float>& c,
                      Vec<4,float>& d) {
    __m128 a_ = _mm_loadu_ps(v);
    __m128 b_ = _mm_loadu_ps(v+4);
    __m128 c_ = _mm_loadu_ps(v+8);
    __m128 d_ = _mm_loadu_ps(v+12);
    _MM_TRANSPOSE4_PS(a_, b_, c_, d_);
    a = sk_bit_cast<Vec<4,float>>(a_);
    b = sk_bit_cast<Vec<4,float>>(b_);
    c = sk_bit_cast<Vec<4,float>>(c_);
    d = sk_bit_cast<Vec<4,float>>(d_);
}

#elif SKVX_USE_SIMD && SKVX_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
#define _LSX_TRANSPOSE4(row0, row1, row2, row3) \
do {                                            \
    __m128i __t0 = __lsx_vilvl_w (row1, row0);  \
    __m128i __t1 = __lsx_vilvl_w (row3, row2);  \
    __m128i __t2 = __lsx_vilvh_w (row1, row0);  \
    __m128i __t3 = __lsx_vilvh_w (row3, row2);  \
    (row0) = __lsx_vilvl_d (__t1, __t0);        \
    (row1) = __lsx_vilvh_d (__t1, __t0);        \
    (row2) = __lsx_vilvl_d (__t3, __t2);        \
    (row3) = __lsx_vilvh_d (__t3, __t2);        \
} while (0)

SI void strided_load4(const int* v,
                      Vec<4,int>& a,
                      Vec<4,int>& b,
                      Vec<4,int>& c,
                      Vec<4,int>& d) {
    __m128i a_ = __lsx_vld(v, 0);
    __m128i b_ = __lsx_vld(v, 16);
    __m128i c_ = __lsx_vld(v, 32);
    __m128i d_ = __lsx_vld(v, 48);
    _LSX_TRANSPOSE4(a_, b_, c_, d_);
    a = sk_bit_cast<Vec<4,int>>(a_);
    b = sk_bit_cast<Vec<4,int>>(b_);
    c = sk_bit_cast<Vec<4,int>>(c_);
    d = sk_bit_cast<Vec<4,int>>(d_);
}
#endif

// De-interleaving load of 2 vectors.
//
// WARNING: These are really only supported well on NEON. Consider restructuring your data before
// resorting to these methods.
SIT void strided_load2(const T* v, Vec<1,T>& a, Vec<1,T>& b) {
    a.val = v[0];
    b.val = v[1];
}
SINT void strided_load2(const T* v, Vec<N,T>& a, Vec<N,T>& b) {
    strided_load2(v, a.lo, b.lo);
    strided_load2(v + 2*(N/2), a.hi, b.hi);
}
#if SKVX_USE_SIMD && defined(SK_ARM_HAS_NEON)
#define IMPL_LOAD2_TRANSPOSED(N, T, VLD) \
SI void strided_load2(const T* v, Vec<N,T>& a, Vec<N,T>& b) { \
    auto mat = VLD(v); \
    a = sk_bit_cast<Vec<N,T>>(mat.val[0]); \
    b = sk_bit_cast<Vec<N,T>>(mat.val[1]); \
}
IMPL_LOAD2_TRANSPOSED(2, uint32_t, vld2_u32)
IMPL_LOAD2_TRANSPOSED(4, uint16_t, vld2_u16)
IMPL_LOAD2_TRANSPOSED(8, uint8_t, vld2_u8)
IMPL_LOAD2_TRANSPOSED(2, int32_t, vld2_s32)
IMPL_LOAD2_TRANSPOSED(4, int16_t, vld2_s16)
IMPL_LOAD2_TRANSPOSED(8, int8_t, vld2_s8)
IMPL_LOAD2_TRANSPOSED(2, float, vld2_f32)
IMPL_LOAD2_TRANSPOSED(4, uint32_t, vld2q_u32)
IMPL_LOAD2_TRANSPOSED(8, uint16_t, vld2q_u16)
IMPL_LOAD2_TRANSPOSED(16, uint8_t, vld2q_u8)
IMPL_LOAD2_TRANSPOSED(4, int32_t, vld2q_s32)
IMPL_LOAD2_TRANSPOSED(8, int16_t, vld2q_s16)
IMPL_LOAD2_TRANSPOSED(16, int8_t, vld2q_s8)
IMPL_LOAD2_TRANSPOSED(4, float, vld2q_f32)
#undef IMPL_LOAD2_TRANSPOSED
#endif

// Define commonly used aliases
using float2  = Vec< 2, float>;
using float4  = Vec< 4, float>;
using float8  = Vec< 8, float>;

using double2 = Vec< 2, double>;
using double4 = Vec< 4, double>;
using double8 = Vec< 8, double>;

using byte2   = Vec< 2, uint8_t>;
using byte4   = Vec< 4, uint8_t>;
using byte8   = Vec< 8, uint8_t>;
using byte16  = Vec<16, uint8_t>;

using int2    = Vec< 2, int32_t>;
using int4    = Vec< 4, int32_t>;
using int8    = Vec< 8, int32_t>;

using ushort2 = Vec< 2, uint16_t>;
using ushort4 = Vec< 4, uint16_t>;
using ushort8 = Vec< 8, uint16_t>;

using uint2   = Vec< 2, uint32_t>;
using uint4   = Vec< 4, uint32_t>;
using uint8   = Vec< 8, uint32_t>;

using long2   = Vec< 2, int64_t>;
using long4   = Vec< 4, int64_t>;
using long8   = Vec< 8, int64_t>;

// Use with from_half and to_half to convert between floatX, and use these for storage.
using half2   = Vec< 2, uint16_t>;
using half4   = Vec< 4, uint16_t>;
using half8   = Vec< 8, uint16_t>;

}  // namespace skvx

#undef SINTU
#undef SINT
#undef SIN
#undef SIT
#undef SI
#undef SKVX_ALWAYS_INLINE
#undef SKVX_USE_SIMD

#endif//SKVX_DEFINED
