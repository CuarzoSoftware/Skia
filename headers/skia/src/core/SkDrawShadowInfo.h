/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawShadowInfo_DEFINED
#define SkDrawShadowInfo_DEFINED

#include "CZ/skia/core/SkColor.h"
#include "CZ/skia/core/SkPoint.h"
#include "CZ/skia/core/SkPoint3.h"
#include "CZ/skia/core/SkScalar.h"
#include "CZ/skia/private/base/SkAssert.h"
#include "CZ/skia/private/base/SkFloatingPoint.h"
#include "CZ/skia/private/base/SkTPin.h"

#include <algorithm>
#include <cstdint>

class SkMatrix;
class SkPath;
struct SkRect;

struct SkDrawShadowRec {
    SkPoint3    fZPlaneParams;
    SkPoint3    fLightPos;
    SkScalar    fLightRadius;
    SkColor     fAmbientColor;
    SkColor     fSpotColor;
    uint32_t    fFlags;
};

namespace SkDrawShadowMetrics {

static constexpr auto kAmbientHeightFactor = 1.0f / 128.0f;
static constexpr auto kAmbientGeomFactor = 64.0f;
// Assuming that we have a light height of 600 for the spot shadow,
// the spot values will reach their maximum at a height of approximately 292.3077.
// We'll round up to 300 to keep it simple.
static constexpr auto kMaxAmbientRadius = 300*kAmbientHeightFactor*kAmbientGeomFactor;

static inline float divide_and_pin(float numer, float denom, float min, float max) {
    float result = SkTPin(sk_ieee_float_divide(numer, denom), min, max);
    // ensure that SkTPin handled non-finites correctly
    SkASSERT(result >= min && result <= max);
    return result;
}

inline SkScalar AmbientBlurRadius(SkScalar height) {
    return std::min(height*kAmbientHeightFactor*kAmbientGeomFactor, kMaxAmbientRadius);
}

inline SkScalar AmbientRecipAlpha(SkScalar height) {
    return 1.0f + std::max(height*kAmbientHeightFactor, 0.0f);
}

inline SkScalar SpotBlurRadius(SkScalar occluderZ, SkScalar lightZ, SkScalar lightRadius) {
    return lightRadius*divide_and_pin(occluderZ, lightZ - occluderZ, 0.0f, 0.95f);
}

inline void GetSpotParams(SkScalar occluderZ, SkScalar lightX, SkScalar lightY, SkScalar lightZ,
                          SkScalar lightRadius,
                          SkScalar* blurRadius, SkScalar* scale, SkVector* translate) {
    SkScalar zRatio = divide_and_pin(occluderZ, lightZ - occluderZ, 0.0f, 0.95f);
    *blurRadius = lightRadius*zRatio;
    *scale = divide_and_pin(lightZ, lightZ - occluderZ, 1.0f, 1.95f);
    *translate = SkVector::Make(-zRatio * lightX, -zRatio * lightY);
}

inline void GetDirectionalParams(SkScalar occluderZ, SkScalar lightX, SkScalar lightY,
                                 SkScalar lightZ, SkScalar lightRadius,
                                 SkScalar* blurRadius, SkScalar* scale, SkVector* translate) {
    *blurRadius = lightRadius*occluderZ;
    *scale = 1;
    // Max z-ratio is "max expected elevation"/"min allowable z"
    constexpr SkScalar kMaxZRatio = 64/SK_ScalarNearlyZero;
    SkScalar zRatio = divide_and_pin(occluderZ, lightZ, 0.0f, kMaxZRatio);
    *translate = SkVector::Make(-zRatio * lightX, -zRatio * lightY);
}

// Create the transformation to apply to a path to get its base shadow outline, given the light
// parameters and the path's 3D transformation (given by ctm and zPlaneParams).
// Also computes the blur radius to apply the transformed outline.
bool GetSpotShadowTransform(const SkPoint3& lightPos, SkScalar lightRadius,
                            const SkMatrix& ctm, const SkPoint3& zPlaneParams,
                            const SkRect& pathBounds, bool directional,
                            SkMatrix* shadowTransform, SkScalar* radius);

// get bounds prior to the ctm being applied
void GetLocalBounds(const SkPath&, const SkDrawShadowRec&, const SkMatrix& ctm, SkRect* bounds);

}  // namespace SkDrawShadowMetrics

#endif
