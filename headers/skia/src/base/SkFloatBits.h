/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFloatBits_DEFINED
#define SkFloatBits_DEFINED

#include "CZ/skia/private/base/SkMath.h"

#include <cstdint>
#include <cstring>

/** Convert a sign-bit int (i.e. float interpreted as int) into a 2s compliement
    int. This also converts -0 (0x80000000) to 0. Doing this to a float allows
    it to be compared using normal C operators (<, <=, etc.)
*/
static inline int32_t SkSignBitTo2sCompliment(int32_t x) {
    if (x < 0) {
        x &= 0x7FFFFFFF;
        x = -x;
    }
    return x;
}

/** Convert a 2s compliment int to a sign-bit (i.e. int interpreted as float).
    This undoes the result of SkSignBitTo2sCompliment().
 */
static inline int32_t Sk2sComplimentToSignBit(int32_t x) {
    int sign = x >> 31;
    // make x positive
    x = (x ^ sign) - sign;
    // set the sign bit as needed
    x |= SkLeftShift(sign, 31);
    return x;
}

// Helper to see a float as its bit pattern (w/o aliasing warnings)
static inline uint32_t SkFloat2Bits(float value) {
    uint32_t bits;
    memcpy(&bits, &value, sizeof(uint32_t));
    return bits;
}

// Helper to see a bit pattern as a float (w/o aliasing warnings)
static inline float SkBits2Float(uint32_t bits) {
    float value;
    memcpy(&value, &bits, sizeof(float));
    return value;
}

/** Return the float as a 2s compliment int. Just to be used to compare floats
    to each other or against positive float-bit-constants (like 0). This does
    not return the int equivalent of the float, just something cheaper for
    compares-only.
 */
static inline int32_t SkFloatAs2sCompliment(float x) {
    return SkSignBitTo2sCompliment((int32_t)SkFloat2Bits(x));
}

/** Return the 2s compliment int as a float. This undos the result of
    SkFloatAs2sCompliment
 */
static inline float Sk2sComplimentAsFloat(int32_t x) {
    return SkBits2Float((uint32_t)Sk2sComplimentToSignBit(x));
}

//  Scalar wrappers for float-bit routines

#define SkScalarAs2sCompliment(x)    SkFloatAs2sCompliment(x)

#endif
