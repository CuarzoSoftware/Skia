/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGMask_DEFINED
#define SkSVGMask_DEFINED

#include "CZ/skia/core/SkRect.h"
#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/private/base/SkAPI.h"
#include "CZ/skia/modules/svg/include/SkSVGHiddenContainer.h"
#include "CZ/skia/modules/svg/include/SkSVGNode.h"
#include "CZ/skia/modules/svg/include/SkSVGTypes.h"

class SkSVGRenderContext;

class SK_API SkSVGMask final : public SkSVGHiddenContainer {
public:
    static sk_sp<SkSVGMask> Make() {
        return sk_sp<SkSVGMask>(new SkSVGMask());
    }

    SVG_ATTR(X     , SkSVGLength, SkSVGLength(-10, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(Y     , SkSVGLength, SkSVGLength(-10, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(Width , SkSVGLength, SkSVGLength(120, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(Height, SkSVGLength, SkSVGLength(120, SkSVGLength::Unit::kPercentage))

    SVG_ATTR(MaskUnits, SkSVGObjectBoundingBoxUnits,
             SkSVGObjectBoundingBoxUnits(SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox))
    SVG_ATTR(MaskContentUnits, SkSVGObjectBoundingBoxUnits,
             SkSVGObjectBoundingBoxUnits(SkSVGObjectBoundingBoxUnits::Type::kUserSpaceOnUse))

private:
    friend class SkSVGRenderContext;

    SkSVGMask() : INHERITED(SkSVGTag::kMask) {}

    bool parseAndSetAttribute(const char*, const char*) override;

    SkRect bounds(const SkSVGRenderContext&) const;
    void renderMask(const SkSVGRenderContext&) const;

    using INHERITED = SkSVGHiddenContainer;
};

#endif // SkSVGMask_DEFINED
