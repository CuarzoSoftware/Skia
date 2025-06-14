/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFilterContext_DEFINED
#define SkSVGFilterContext_DEFINED

#include "CZ/skia/core/SkImageFilter.h"
#include "CZ/skia/core/SkRect.h"
#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/modules/svg/include/SkSVGTypes.h"
#include "CZ/skia/src/core/SkTHash.h"

#include <tuple>

class SkSVGRenderContext;

class SkSVGFilterContext {
public:
    SkSVGFilterContext(const SkRect& filterEffectsRegion,
                       const SkSVGObjectBoundingBoxUnits& primitiveUnits)
            : fFilterEffectsRegion(filterEffectsRegion)
            , fPrimitiveUnits(primitiveUnits)
            , fPreviousResult({nullptr, filterEffectsRegion, SkSVGColorspace::kSRGB}) {}

    const SkRect& filterEffectsRegion() const { return fFilterEffectsRegion; }

    const SkRect& filterPrimitiveSubregion(const SkSVGFeInputType&) const;

    const SkSVGObjectBoundingBoxUnits& primitiveUnits() const { return fPrimitiveUnits; }

    void registerResult(const SkSVGStringType&, const sk_sp<SkImageFilter>&, const SkRect&, SkSVGColorspace);

    void setPreviousResult(const sk_sp<SkImageFilter>&, const SkRect&, SkSVGColorspace);

    bool previousResultIsSourceGraphic() const;

    SkSVGColorspace resolveInputColorspace(const SkSVGRenderContext&,
                                           const SkSVGFeInputType&) const;

    sk_sp<SkImageFilter> resolveInput(const SkSVGRenderContext&, const SkSVGFeInputType&) const;

    sk_sp<SkImageFilter> resolveInput(const SkSVGRenderContext&, const SkSVGFeInputType&, SkSVGColorspace) const;

private:
    struct Result {
        sk_sp<SkImageFilter> fImageFilter;
        SkRect fFilterSubregion;
        SkSVGColorspace fColorspace;
    };

    const Result* findResultById(const SkSVGStringType&) const;

    std::tuple<sk_sp<SkImageFilter>, SkSVGColorspace> getInput(const SkSVGRenderContext&,
                                                               const SkSVGFeInputType&) const;

    SkRect fFilterEffectsRegion;

    SkSVGObjectBoundingBoxUnits fPrimitiveUnits;

    skia_private::THashMap<SkSVGStringType, Result> fResults;

    Result fPreviousResult;
};

#endif  // SkSVGFilterContext_DEFINED
