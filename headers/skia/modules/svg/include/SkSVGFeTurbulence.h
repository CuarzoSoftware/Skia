/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFeTurbulence_DEFINED
#define SkSVGFeTurbulence_DEFINED

#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/private/base/SkAPI.h"
#include "CZ/skia/modules/svg/include/SkSVGFe.h"
#include "CZ/skia/modules/svg/include/SkSVGNode.h"
#include "CZ/skia/modules/svg/include/SkSVGTypes.h"

#include <vector>

class SkImageFilter;
class SkSVGFilterContext;
class SkSVGRenderContext;

class SK_API SkSVGFeTurbulence : public SkSVGFe {
public:
    static sk_sp<SkSVGFeTurbulence> Make() {
        return sk_sp<SkSVGFeTurbulence>(new SkSVGFeTurbulence());
    }

    SVG_ATTR(BaseFrequency, SkSVGFeTurbulenceBaseFrequency, SkSVGFeTurbulenceBaseFrequency({}))
    SVG_ATTR(NumOctaves, SkSVGIntegerType, SkSVGIntegerType(1))
    SVG_ATTR(Seed, SkSVGNumberType, SkSVGNumberType(0))
    SVG_ATTR(TurbulenceType,
             SkSVGFeTurbulenceType,
             SkSVGFeTurbulenceType(SkSVGFeTurbulenceType::Type::kTurbulence))

protected:
    sk_sp<SkImageFilter> onMakeImageFilter(const SkSVGRenderContext&,
                                           const SkSVGFilterContext&) const override;

    std::vector<SkSVGFeInputType> getInputs() const override { return {}; }

    bool parseAndSetAttribute(const char*, const char*) override;
private:
    SkSVGFeTurbulence() : INHERITED(SkSVGTag::kFeTurbulence) {}

    using INHERITED = SkSVGFe;
};

#endif  // SkSVGFeTurbulence_DEFINED
