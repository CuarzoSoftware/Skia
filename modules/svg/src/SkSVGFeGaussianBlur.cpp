/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "skia/modules/svg/include/SkSVGFeGaussianBlur.h"

#include "skia/core/SkM44.h"
#include "skia/effects/SkImageFilters.h"
#include "skia/modules/svg/include/SkSVGAttributeParser.h"
#include "skia/modules/svg/include/SkSVGFilterContext.h"
#include "skia/modules/svg/include/SkSVGRenderContext.h"

class SkImageFilter;

bool SkSVGFeGaussianBlur::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setStdDeviation(SkSVGAttributeParser::parse<SkSVGFeGaussianBlur::StdDeviation>(
                   "stdDeviation", name, value));
}

sk_sp<SkImageFilter> SkSVGFeGaussianBlur::onMakeImageFilter(const SkSVGRenderContext& ctx,
                                                            const SkSVGFilterContext& fctx) const {
    const auto sigma = SkV2{fStdDeviation.fX, fStdDeviation.fY}
                     * ctx.transformForCurrentOBB(fctx.primitiveUnits()).scale;

    return SkImageFilters::Blur(
            sigma.x, sigma.y,
            fctx.resolveInput(ctx, this->getIn(), this->resolveColorspace(ctx, fctx)),
            this->resolveFilterSubregion(ctx, fctx));
}

template <>
bool SkSVGAttributeParser::parse<SkSVGFeGaussianBlur::StdDeviation>(
        SkSVGFeGaussianBlur::StdDeviation* stdDeviation) {
    std::vector<SkSVGNumberType> values;
    if (!this->parse(&values)) {
        return false;
    }

    stdDeviation->fX = values[0];
    stdDeviation->fY = values.size() > 1 ? values[1] : values[0];
    return true;
}
