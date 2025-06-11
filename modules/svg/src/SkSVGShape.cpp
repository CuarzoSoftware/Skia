/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "skia/modules/svg/include/SkSVGShape.h"

#include "skia/core/SkPaint.h"  // IWYU pragma: keep
#include "skia/private/base/SkDebug.h"
#include "skia/modules/svg/include/SkSVGAttribute.h"
#include "skia/modules/svg/include/SkSVGRenderContext.h"
#include "skia/modules/svg/include/SkSVGTypes.h"
#include "skia/src/base/SkTLazy.h"

class SkSVGNode;
enum class SkSVGTag;

SkSVGShape::SkSVGShape(SkSVGTag t) : INHERITED(t) {}

void SkSVGShape::onRender(const SkSVGRenderContext& ctx) const {
    const auto fillType = ctx.presentationContext().fInherited.fFillRule->asFillType();

    const auto fillPaint = ctx.fillPaint(),
             strokePaint = ctx.strokePaint();

    // TODO: this approach forces duplicate geometry resolution in onDraw(); refactor to avoid.
    if (fillPaint.isValid()) {
        this->onDraw(ctx.canvas(), ctx.lengthContext(), *fillPaint, fillType);
    }

    if (strokePaint.isValid()) {
        this->onDraw(ctx.canvas(), ctx.lengthContext(), *strokePaint, fillType);
    }
}

void SkSVGShape::appendChild(sk_sp<SkSVGNode>) {
    SkDEBUGF("cannot append child nodes to an SVG shape.\n");
}
