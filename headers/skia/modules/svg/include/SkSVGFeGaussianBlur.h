/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFeGaussianBlur_DEFINED
#define SkSVGFeGaussianBlur_DEFINED

#include "cz/skia/core/SkRefCnt.h"
#include "cz/skia/private/base/SkAPI.h"
#include "cz/skia/modules/svg/include/SkSVGFe.h"
#include "cz/skia/modules/svg/include/SkSVGNode.h"
#include "cz/skia/modules/svg/include/SkSVGTypes.h"

#include <vector>

class SkImageFilter;
class SkSVGFilterContext;
class SkSVGRenderContext;

class SK_API SkSVGFeGaussianBlur : public SkSVGFe {
public:
    struct StdDeviation {
        SkSVGNumberType fX;
        SkSVGNumberType fY;
    };

    static sk_sp<SkSVGFeGaussianBlur> Make() {
        return sk_sp<SkSVGFeGaussianBlur>(new SkSVGFeGaussianBlur());
    }

    SVG_ATTR(StdDeviation, StdDeviation, StdDeviation({0, 0}))

protected:
    sk_sp<SkImageFilter> onMakeImageFilter(const SkSVGRenderContext&,
                                           const SkSVGFilterContext&) const override;

    std::vector<SkSVGFeInputType> getInputs() const override { return {this->getIn()}; }

    bool parseAndSetAttribute(const char*, const char*) override;

private:
    SkSVGFeGaussianBlur() : INHERITED(SkSVGTag::kFeGaussianBlur) {}

    using INHERITED = SkSVGFe;
};

#endif  // SkSVGFeGaussianBlur_DEFINED
