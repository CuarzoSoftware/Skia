/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathMeasurePriv_DEFINED
#define SkPathMeasurePriv_DEFINED

#include "CZ/skia/core/SkPath.h"
#include "CZ/skia/core/SkPoint.h"
#include "CZ/skia/src/core/SkGeometry.h"

// Used in the Segment struct defined in SkPathMeasure.h
// It is used as a 2-bit field so if you add to this
// you must increase the size of the bitfield there.
enum SkSegType {
    kLine_SegType,
    kQuad_SegType,
    kCubic_SegType,
    kConic_SegType,
};


void SkPathMeasure_segTo(const SkPoint pts[], unsigned segType,
                   SkScalar startT, SkScalar stopT, SkPath* dst);

// for testing

class SkPathMeasure;

class SkPathMeasurePriv {
public:
    static size_t CountSegments(const SkPathMeasure&);
};

#endif  // SkPathMeasurePriv_DEFINED
