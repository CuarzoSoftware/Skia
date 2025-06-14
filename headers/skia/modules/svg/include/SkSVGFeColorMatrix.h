/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFeColorMatrix_DEFINED
#define SkSVGFeColorMatrix_DEFINED

#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/effects/SkColorMatrix.h"
#include "CZ/skia/private/base/SkAPI.h"
#include "CZ/skia/modules/svg/include/SkSVGFe.h"
#include "CZ/skia/modules/svg/include/SkSVGNode.h"
#include "CZ/skia/modules/svg/include/SkSVGTypes.h"

#include <vector>

class SkImageFilter;
class SkSVGFilterContext;
class SkSVGRenderContext;

class SK_API SkSVGFeColorMatrix final : public SkSVGFe {
public:
    static sk_sp<SkSVGFeColorMatrix> Make() {
        return sk_sp<SkSVGFeColorMatrix>(new SkSVGFeColorMatrix());
    }

    SVG_ATTR(Type, SkSVGFeColorMatrixType, SkSVGFeColorMatrixType(SkSVGFeColorMatrixType::kMatrix))
    SVG_ATTR(Values, SkSVGFeColorMatrixValues, SkSVGFeColorMatrixValues())

protected:
    sk_sp<SkImageFilter> onMakeImageFilter(const SkSVGRenderContext&,
                                           const SkSVGFilterContext&) const override;

    std::vector<SkSVGFeInputType> getInputs() const override { return {this->getIn()}; }

    bool parseAndSetAttribute(const char*, const char*) override;

private:
    SkSVGFeColorMatrix() : INHERITED(SkSVGTag::kFeColorMatrix) {}

    SkColorMatrix makeMatrixForType() const;

    static SkColorMatrix MakeSaturate(SkSVGNumberType s);

    static SkColorMatrix MakeHueRotate(SkSVGNumberType degrees);

    static SkColorMatrix MakeLuminanceToAlpha();

    using INHERITED = SkSVGFe;
};

#endif  // SkSVGFeColorMatrix_DEFINED
