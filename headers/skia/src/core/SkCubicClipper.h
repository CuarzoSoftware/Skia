/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkCubicClipper_DEFINED
#define SkCubicClipper_DEFINED

#include "CZ/skia/core/SkRect.h"
#include "CZ/skia/core/SkScalar.h"
#include "CZ/skia/core/SkTypes.h"

struct SkPoint;

/** This class is initialized with a clip rectangle, and then can be fed cubics,
    which must already be monotonic in Y.

    In the future, it might return a series of segments, allowing it to clip
    also in X, to ensure that all segments fit in a finite coordinate system.
 */
class SkCubicClipper {
public:
    SkCubicClipper();

    void setClip(const SkIRect& clip);

    [[nodiscard]] bool clipCubic(const SkPoint src[4], SkPoint dst[4]);

    [[nodiscard]] static bool ChopMonoAtY(const SkPoint pts[4], SkScalar y, SkScalar* t);
private:
    SkRect      fClip;
};

#endif  // SkCubicClipper_DEFINED
