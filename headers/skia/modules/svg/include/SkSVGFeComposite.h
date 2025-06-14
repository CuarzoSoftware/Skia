/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFeComposite_DEFINED
#define SkSVGFeComposite_DEFINED

#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/private/base/SkAPI.h"
#include "CZ/skia/modules/svg/include/SkSVGFe.h"
#include "CZ/skia/modules/svg/include/SkSVGNode.h"
#include "CZ/skia/modules/svg/include/SkSVGTypes.h"

#include <vector>

class SkImageFilter;
class SkSVGFilterContext;
class SkSVGRenderContext;
enum class SkBlendMode;

class SK_API SkSVGFeComposite final : public SkSVGFe {
public:
    static sk_sp<SkSVGFeComposite> Make() {
        return sk_sp<SkSVGFeComposite>(new SkSVGFeComposite());
    }

    SVG_ATTR(In2, SkSVGFeInputType, SkSVGFeInputType())
    SVG_ATTR(K1, SkSVGNumberType, SkSVGNumberType(0))
    SVG_ATTR(K2, SkSVGNumberType, SkSVGNumberType(0))
    SVG_ATTR(K3, SkSVGNumberType, SkSVGNumberType(0))
    SVG_ATTR(K4, SkSVGNumberType, SkSVGNumberType(0))
    SVG_ATTR(Operator, SkSVGFeCompositeOperator, SkSVGFeCompositeOperator::kOver)

protected:
    sk_sp<SkImageFilter> onMakeImageFilter(const SkSVGRenderContext&,
                                           const SkSVGFilterContext&) const override;

    std::vector<SkSVGFeInputType> getInputs() const override {
        return {this->getIn(), this->getIn2()};
    }

    bool parseAndSetAttribute(const char*, const char*) override;

private:
    SkSVGFeComposite() : INHERITED(SkSVGTag::kFeComposite) {}

    static SkBlendMode BlendModeForOperator(SkSVGFeCompositeOperator);

    using INHERITED = SkSVGFe;
};

#endif  // SkSVGFeComposite_DEFINED
