/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGRadialGradient_DEFINED
#define SkSVGRadialGradient_DEFINED

#include "cz/skia/core/SkColor.h"
#include "cz/skia/core/SkRefCnt.h"
#include "cz/skia/core/SkScalar.h"
#include "cz/skia/private/base/SkAPI.h"
#include "cz/skia/modules/svg/include/SkSVGGradient.h"
#include "cz/skia/modules/svg/include/SkSVGNode.h"
#include "cz/skia/modules/svg/include/SkSVGTypes.h"
#include "cz/skia/src/base/SkTLazy.h"

class SkMatrix;
class SkSVGRenderContext;
class SkShader;
enum class SkTileMode;

class SK_API SkSVGRadialGradient final : public SkSVGGradient {
public:
    static sk_sp<SkSVGRadialGradient> Make() {
        return sk_sp<SkSVGRadialGradient>(new SkSVGRadialGradient());
    }

    SVG_ATTR(Cx, SkSVGLength, SkSVGLength(50, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(Cy, SkSVGLength, SkSVGLength(50, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(R,  SkSVGLength, SkSVGLength(50, SkSVGLength::Unit::kPercentage))
    SVG_OPTIONAL_ATTR(Fx, SkSVGLength)
    SVG_OPTIONAL_ATTR(Fy, SkSVGLength)

protected:
    bool parseAndSetAttribute(const char*, const char*) override;

    sk_sp<SkShader> onMakeShader(const SkSVGRenderContext&,
                                 const SkColor4f*, const SkScalar*, int count,
                                 SkTileMode, const SkMatrix&) const override;
private:
    SkSVGRadialGradient();

    using INHERITED = SkSVGGradient;
};

#endif // SkSVGRadialGradient_DEFINED
