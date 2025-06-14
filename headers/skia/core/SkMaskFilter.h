/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMaskFilter_DEFINED
#define SkMaskFilter_DEFINED

#include "CZ/skia/core/SkFlattenable.h"
#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/core/SkScalar.h"
#include "CZ/skia/core/SkTypes.h"

#include <cstddef>

enum SkBlurStyle : int;
struct SkDeserialProcs;

/** \class SkMaskFilter

    SkMaskFilter is the base class for object that perform transformations on
    the mask before drawing it. An example subclass is Blur.
*/
class SK_API SkMaskFilter : public SkFlattenable {
public:
    /** Create a blur maskfilter.
     *  @param style      The SkBlurStyle to use
     *  @param sigma      Standard deviation of the Gaussian blur to apply. Must be > 0.
     *  @param respectCTM if true the blur's sigma is modified by the CTM.
     *  @return The new blur maskfilter
     */
    static sk_sp<SkMaskFilter> MakeBlur(SkBlurStyle style, SkScalar sigma,
                                        bool respectCTM = true);

    static sk_sp<SkMaskFilter> Deserialize(const void* data, size_t size,
                                           const SkDeserialProcs* procs = nullptr);

private:
    static void RegisterFlattenables();
    friend class SkFlattenable;
};

#endif
