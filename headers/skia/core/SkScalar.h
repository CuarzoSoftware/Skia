/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScalar_DEFINED
#define SkScalar_DEFINED

#include "CZ/skia/private/base/SkAssert.h"
#include "CZ/skia/private/base/SkFloatingPoint.h"

#include <cmath>

typedef float SkScalar;

#define SK_Scalar1                  1.0f
#define SK_ScalarHalf               0.5f
#define SK_ScalarSqrt2              SK_FloatSqrt2
#define SK_ScalarPI                 SK_FloatPI
#define SK_ScalarTanPIOver8         0.414213562f
#define SK_ScalarRoot2Over2         0.707106781f
#define SK_ScalarMax                3.402823466e+38f
#define SK_ScalarMin                (-SK_ScalarMax)
#define SK_ScalarInfinity           SK_FloatInfinity
#define SK_ScalarNegativeInfinity   SK_FloatNegativeInfinity
#define SK_ScalarNaN                SK_FloatNaN

#define SkScalarFloorToScalar(x)    std::floor(x)
#define SkScalarCeilToScalar(x)     std::ceil(x)
#define SkScalarRoundToScalar(x)    sk_float_round(x)
#define SkScalarTruncToScalar(x)    std::trunc(x)

#define SkScalarFloorToInt(x)       sk_float_floor2int(x)
#define SkScalarCeilToInt(x)        sk_float_ceil2int(x)
#define SkScalarRoundToInt(x)       sk_float_round2int(x)

#define SkScalarAbs(x)              std::fabs(x)
#define SkScalarCopySign(x, y)      std::copysign(x, y)
#define SkScalarMod(x, y)           std::fmod(x,y)
#define SkScalarSqrt(x)             std::sqrt(x)
#define SkScalarPow(b, e)           std::pow(b, e)

#define SkScalarSin(radians)        ((float)std::sin(radians))
#define SkScalarCos(radians)        ((float)std::cos(radians))
#define SkScalarTan(radians)        ((float)std::tan(radians))
#define SkScalarASin(val)           ((float)std::asin(val))
#define SkScalarACos(val)           ((float)std::acos(val))
#define SkScalarATan2(y, x)         ((float)std::atan2(y,x))
#define SkScalarExp(x)              ((float)std::exp(x))
#define SkScalarLog(x)              ((float)std::log(x))
#define SkScalarLog2(x)             ((float)std::log2(x))

//////////////////////////////////////////////////////////////////////////////////////////////////

#define SkIntToScalar(x)        static_cast<SkScalar>(x)
#define SkIntToFloat(x)         static_cast<float>(x)
#define SkScalarTruncToInt(x)   sk_float_saturate2int(x)

#define SkScalarToFloat(x)      static_cast<float>(x)
#define SkFloatToScalar(x)      static_cast<SkScalar>(x)
#define SkScalarToDouble(x)     static_cast<double>(x)
#define SkDoubleToScalar(x)     sk_double_to_float(x)

/** Returns the fractional part of the scalar. */
static inline SkScalar SkScalarFraction(SkScalar x) {
    return x - SkScalarTruncToScalar(x);
}

static inline SkScalar SkScalarSquare(SkScalar x) { return x * x; }

#define SkScalarInvert(x)           (SK_Scalar1 / (x))
#define SkScalarAve(a, b)           (((a) + (b)) * SK_ScalarHalf)
#define SkScalarHalf(a)             ((a) * SK_ScalarHalf)

#define SkDegreesToRadians(degrees) ((degrees) * (SK_ScalarPI / 180))
#define SkRadiansToDegrees(radians) ((radians) * (180 / SK_ScalarPI))

static inline bool SkScalarIsInt(SkScalar x) {
    return x == SkScalarFloorToScalar(x);
}

/**
 *  Returns -1 || 0 || 1 depending on the sign of value:
 *  -1 if x < 0
 *   0 if x == 0
 *   1 if x > 0
 */
static inline int SkScalarSignAsInt(SkScalar x) {
    return x < 0 ? -1 : (x > 0);
}

// Scalar result version of above
static inline SkScalar SkScalarSignAsScalar(SkScalar x) {
    return x < 0 ? -SK_Scalar1 : ((x > 0) ? SK_Scalar1 : 0);
}

#define SK_ScalarNearlyZero         (SK_Scalar1 / (1 << 12))

static inline bool SkScalarNearlyZero(SkScalar x,
                                      SkScalar tolerance = SK_ScalarNearlyZero) {
    SkASSERT(tolerance >= 0);
    return SkScalarAbs(x) <= tolerance;
}

static inline bool SkScalarNearlyEqual(SkScalar x, SkScalar y,
                                       SkScalar tolerance = SK_ScalarNearlyZero) {
    SkASSERT(tolerance >= 0);
    return SkScalarAbs(x-y) <= tolerance;
}

#define SK_ScalarSinCosNearlyZero   (SK_Scalar1 / (1 << 16))

static inline float SkScalarSinSnapToZero(SkScalar radians) {
    float v = SkScalarSin(radians);
    return SkScalarNearlyZero(v, SK_ScalarSinCosNearlyZero) ? 0.0f : v;
}

static inline float SkScalarCosSnapToZero(SkScalar radians) {
    float v = SkScalarCos(radians);
    return SkScalarNearlyZero(v, SK_ScalarSinCosNearlyZero) ? 0.0f : v;
}

/** Linearly interpolate between A and B, based on t.
    If t is 0, return A
    If t is 1, return B
    else interpolate.
    t must be [0..SK_Scalar1]
*/
static inline SkScalar SkScalarInterp(SkScalar A, SkScalar B, SkScalar t) {
    SkASSERT(t >= 0 && t <= SK_Scalar1);
    return A + (B - A) * t;
}

/** Interpolate along the function described by (keys[length], values[length])
    for the passed searchKey. SearchKeys outside the range keys[0]-keys[Length]
    clamp to the min or max value. This function assumes the number of pairs
    (length) will be small and a linear search is used.

    Repeated keys are allowed for discontinuous functions (so long as keys is
    monotonically increasing). If key is the value of a repeated scalar in
    keys the first one will be used.
*/
SkScalar SkScalarInterpFunc(SkScalar searchKey, const SkScalar keys[],
                            const SkScalar values[], int length);

/*
 *  Helper to compare an array of scalars.
 */
static inline bool SkScalarsEqual(const SkScalar a[], const SkScalar b[], int n) {
    SkASSERT(n >= 0);
    for (int i = 0; i < n; ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

#endif
