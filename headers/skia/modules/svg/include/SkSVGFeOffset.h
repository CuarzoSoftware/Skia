/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFeOffset_DEFINED
#define SkSVGFeOffset_DEFINED

#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/private/base/SkAPI.h"
#include "CZ/skia/modules/svg/include/SkSVGFe.h"
#include "CZ/skia/modules/svg/include/SkSVGNode.h"
#include "CZ/skia/modules/svg/include/SkSVGTypes.h"

#include <vector>

class SkImageFilter;
class SkSVGFilterContext;
class SkSVGRenderContext;

class SK_API SkSVGFeOffset : public SkSVGFe {
public:
    static sk_sp<SkSVGFeOffset> Make() { return sk_sp<SkSVGFeOffset>(new SkSVGFeOffset()); }

    SVG_ATTR(Dx, SkSVGNumberType, SkSVGNumberType(0))
    SVG_ATTR(Dy, SkSVGNumberType, SkSVGNumberType(0))

protected:
    sk_sp<SkImageFilter> onMakeImageFilter(const SkSVGRenderContext&,
                                           const SkSVGFilterContext&) const override;

    std::vector<SkSVGFeInputType> getInputs() const override { return {this->getIn()}; }

    bool parseAndSetAttribute(const char*, const char*) override;

private:
    SkSVGFeOffset() : INHERITED(SkSVGTag::kFeOffset) {}

    using INHERITED = SkSVGFe;
};

#endif  // SkSVGFeOffset_DEFINED
