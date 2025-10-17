/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGRect_DEFINED
#define SkSVGRect_DEFINED

#include "CZ/skia/core/SkPath.h"
#include "CZ/skia/core/SkRect.h"
#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/private/base/SkAPI.h"
#include "CZ/skia/modules/svg/include/SkSVGNode.h"
#include "CZ/skia/modules/svg/include/SkSVGShape.h"
#include "CZ/skia/modules/svg/include/SkSVGTypes.h"
#include "CZ/skia/src/base/SkTLazy.h"

class SkCanvas;
class SkPaint;
class SkRRect;
class SkSVGLengthContext;
class SkSVGRenderContext;
enum class SkPathFillType;

class SK_API SkSVGRect final : public SkSVGShape {
public:
    static sk_sp<SkSVGRect> Make() { return sk_sp<SkSVGRect>(new SkSVGRect()); }

    SVG_ATTR(X     , SkSVGLength, SkSVGLength(0))
    SVG_ATTR(Y     , SkSVGLength, SkSVGLength(0))
    SVG_ATTR(Width , SkSVGLength, SkSVGLength(0))
    SVG_ATTR(Height, SkSVGLength, SkSVGLength(0))

    SVG_OPTIONAL_ATTR(Rx, SkSVGLength)
    SVG_OPTIONAL_ATTR(Ry, SkSVGLength)

protected:
    bool parseAndSetAttribute(const char*, const char*) override;

    void onDraw(SkCanvas*, const SkSVGLengthContext&, const SkPaint&,
                SkPathFillType) const override;

    SkPath onAsPath(const SkSVGRenderContext&) const override;

    SkRect onTransformableObjectBoundingBox(const SkSVGRenderContext&) const override;

private:
    SkSVGRect();

    SkRRect resolve(const SkSVGLengthContext&) const;

    using INHERITED = SkSVGShape;
};

#endif // SkSVGRect_DEFINED
