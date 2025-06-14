/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFilter_DEFINED
#define SkSVGFilter_DEFINED

#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/private/base/SkAPI.h"
#include "CZ/skia/modules/svg/include/SkSVGHiddenContainer.h"
#include "CZ/skia/modules/svg/include/SkSVGNode.h"
#include "CZ/skia/modules/svg/include/SkSVGTypes.h"

class SkImageFilter;
class SkSVGRenderContext;

class SK_API SkSVGFilter final : public SkSVGHiddenContainer {
public:
    static sk_sp<SkSVGFilter> Make() { return sk_sp<SkSVGFilter>(new SkSVGFilter()); }

    /** Propagates any inherited presentation attributes in the given context. */
    void applyProperties(SkSVGRenderContext*) const;

    sk_sp<SkImageFilter> buildFilterDAG(const SkSVGRenderContext&) const;

    SVG_ATTR(X, SkSVGLength, SkSVGLength(-10, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(Y, SkSVGLength, SkSVGLength(-10, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(Width, SkSVGLength, SkSVGLength(120, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(Height, SkSVGLength, SkSVGLength(120, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(FilterUnits,
             SkSVGObjectBoundingBoxUnits,
             SkSVGObjectBoundingBoxUnits(SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox))
    SVG_ATTR(PrimitiveUnits,
             SkSVGObjectBoundingBoxUnits,
             SkSVGObjectBoundingBoxUnits(SkSVGObjectBoundingBoxUnits::Type::kUserSpaceOnUse))

private:
    SkSVGFilter() : INHERITED(SkSVGTag::kFilter) {}

    bool parseAndSetAttribute(const char*, const char*) override;

    using INHERITED = SkSVGHiddenContainer;
};

#endif  // SkSVGFilter_DEFINED
