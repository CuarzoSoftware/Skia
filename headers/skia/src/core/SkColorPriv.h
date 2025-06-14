/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorPriv_DEFINED
#define SkColorPriv_DEFINED

#include "CZ/skia/core/SkColor.h"
#include "CZ/skia/core/SkScalar.h"
#include "CZ/skia/core/SkTypes.h"
#include "CZ/skia/private/base/SkCPUTypes.h"
#include "CZ/skia/private/base/SkMath.h"
#include "CZ/skia/private/base/SkTPin.h"
#include "CZ/skia/private/base/SkTo.h"

#include <algorithm>
#include <cstdint>

/** Turn 0..255 into 0..256 by adding 1 at the half-way point. Used to turn a
    byte into a scale value, so that we can say scale * value >> 8 instead of
    alpha * value / 255.

    In debugging, asserts that alpha is 0..255
*/
static inline unsigned SkAlpha255To256(U8CPU alpha) {
    SkASSERT(SkToU8(alpha) == alpha);
    // this one assues that blending on top of an opaque dst keeps it that way
    // even though it is less accurate than a+(a>>7) for non-opaque dsts
    return alpha + 1;
}

/** Multiplify value by 0..256, and shift the result down 8
    (i.e. return (value * alpha256) >> 8)
 */
#define SkAlphaMul(value, alpha256)     (((value) * (alpha256)) >> 8)

static inline U8CPU SkUnitScalarClampToByte(SkScalar x) {
    return static_cast<U8CPU>(SkTPin(x, 0.0f, 1.0f) * 255 + 0.5);
}

#define SK_A32_BITS     8
#define SK_R32_BITS     8
#define SK_G32_BITS     8
#define SK_B32_BITS     8

#define SK_A32_MASK     ((1 << SK_A32_BITS) - 1)
#define SK_R32_MASK     ((1 << SK_R32_BITS) - 1)
#define SK_G32_MASK     ((1 << SK_G32_BITS) - 1)
#define SK_B32_MASK     ((1 << SK_B32_BITS) - 1)

/*
 *  Skia's 32bit backend only supports 1 swizzle order at a time (compile-time).
 *  This is specified by SK_R32_SHIFT=0 or SK_R32_SHIFT=16.
 *
 *  For easier compatibility with Skia's GPU backend, we further restrict these
 *  to either (in memory-byte-order) RGBA or BGRA. Note that this "order" does
 *  not directly correspond to the same shift-order, since we have to take endianess
 *  into account.
 *
 *  Here we enforce this constraint.
 */

#define SK_RGBA_R32_SHIFT   0
#define SK_RGBA_G32_SHIFT   8
#define SK_RGBA_B32_SHIFT   16
#define SK_RGBA_A32_SHIFT   24

#define SK_BGRA_B32_SHIFT   0
#define SK_BGRA_G32_SHIFT   8
#define SK_BGRA_R32_SHIFT   16
#define SK_BGRA_A32_SHIFT   24

#if defined(SK_PMCOLOR_IS_RGBA) || defined(SK_PMCOLOR_IS_BGRA)
    #error "Configure PMCOLOR by setting SK_R32_SHIFT."
#endif

// Deduce which SK_PMCOLOR_IS_ to define from the _SHIFT defines

#if (SK_A32_SHIFT == SK_RGBA_A32_SHIFT && \
     SK_R32_SHIFT == SK_RGBA_R32_SHIFT && \
     SK_G32_SHIFT == SK_RGBA_G32_SHIFT && \
     SK_B32_SHIFT == SK_RGBA_B32_SHIFT)
    #define SK_PMCOLOR_IS_RGBA
#elif (SK_A32_SHIFT == SK_BGRA_A32_SHIFT && \
       SK_R32_SHIFT == SK_BGRA_R32_SHIFT && \
       SK_G32_SHIFT == SK_BGRA_G32_SHIFT && \
       SK_B32_SHIFT == SK_BGRA_B32_SHIFT)
    #define SK_PMCOLOR_IS_BGRA
#else
    #error "need 32bit packing to be either RGBA or BGRA"
#endif

#define SkGetPackedA32(packed)      ((uint32_t)((packed) << (24 - SK_A32_SHIFT)) >> 24)
#define SkGetPackedR32(packed)      ((uint32_t)((packed) << (24 - SK_R32_SHIFT)) >> 24)
#define SkGetPackedG32(packed)      ((uint32_t)((packed) << (24 - SK_G32_SHIFT)) >> 24)
#define SkGetPackedB32(packed)      ((uint32_t)((packed) << (24 - SK_B32_SHIFT)) >> 24)

#define SkA32Assert(a)  SkASSERT((unsigned)(a) <= SK_A32_MASK)
#define SkR32Assert(r)  SkASSERT((unsigned)(r) <= SK_R32_MASK)
#define SkG32Assert(g)  SkASSERT((unsigned)(g) <= SK_G32_MASK)
#define SkB32Assert(b)  SkASSERT((unsigned)(b) <= SK_B32_MASK)

/**
 *  Pack the components into a SkPMColor
 */
static inline SkPMColor SkPackARGB32(U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
    SkA32Assert(a);
    SkR32Assert(r);
    SkG32Assert(g);
    SkB32Assert(b);

    return (a << SK_A32_SHIFT) | (r << SK_R32_SHIFT) |
           (g << SK_G32_SHIFT) | (b << SK_B32_SHIFT);
}

static inline
SkPMColor SkPremultiplyARGBInline(U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
    SkA32Assert(a);
    SkR32Assert(r);
    SkG32Assert(g);
    SkB32Assert(b);

    if (a != 255) {
        r = SkMulDiv255Round(r, a);
        g = SkMulDiv255Round(g, a);
        b = SkMulDiv255Round(b, a);
    }
    return SkPackARGB32(a, r, g, b);
}

// When Android is compiled optimizing for size, SkAlphaMulQ doesn't get
// inlined; forcing inlining significantly improves performance.
static SK_ALWAYS_INLINE uint32_t SkAlphaMulQ(uint32_t c, unsigned scale) {
    static constexpr uint32_t kMask = 0x00FF00FF;

    uint32_t rb = ((c & kMask) * scale) >> 8;
    uint32_t ag = ((c >> 8) & kMask) * scale;
    return (rb & kMask) | (ag & ~kMask);
}

static inline SkPMColor SkPMSrcOver(SkPMColor src, SkPMColor dst) {
    uint32_t scale = SkAlpha255To256(255 - SkGetPackedA32(src));

    static constexpr uint32_t kMask = 0x00FF00FF;
    uint32_t rb = (((dst & kMask) * scale) >> 8) & kMask;
    uint32_t ag = (((dst >> 8) & kMask) * scale) & ~kMask;

    rb += (src &  kMask);
    ag += (src & ~kMask);

    // Color channels (but not alpha) can overflow, so we have to saturate to 0xFF in each lane.
    return std::min(rb & 0x000001FF, 0x000000FFU) |
           std::min(ag & 0x0001FF00, 0x0000FF00U) |
           std::min(rb & 0x01FF0000, 0x00FF0000U) |
                   (ag & 0xFF000000);
}

#endif
