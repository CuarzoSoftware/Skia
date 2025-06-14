/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGEllipse_DEFINED
#define SkSVGEllipse_DEFINED

#include "cz/skia/core/SkPath.h"
#include "cz/skia/core/SkRefCnt.h"
#include "cz/skia/private/base/SkAPI.h"
#include "cz/skia/modules/svg/include/SkSVGNode.h"
#include "cz/skia/modules/svg/include/SkSVGShape.h"
#include "cz/skia/modules/svg/include/SkSVGTypes.h"
#include "cz/skia/src/base/SkTLazy.h"

class SkCanvas;
class SkPaint;
class SkSVGLengthContext;
class SkSVGRenderContext;
enum class SkPathFillType;
struct SkRect;

class SK_API SkSVGEllipse final : public SkSVGShape {
public:
    static sk_sp<SkSVGEllipse> Make() { return sk_sp<SkSVGEllipse>(new SkSVGEllipse()); }

    SVG_ATTR(Cx, SkSVGLength, SkSVGLength(0))
    SVG_ATTR(Cy, SkSVGLength, SkSVGLength(0))

    SVG_OPTIONAL_ATTR(Rx, SkSVGLength)
    SVG_OPTIONAL_ATTR(Ry, SkSVGLength)

protected:
    bool parseAndSetAttribute(const char*, const char*) override;

    void onDraw(SkCanvas*, const SkSVGLengthContext&, const SkPaint&,
                SkPathFillType) const override;

    SkPath onAsPath(const SkSVGRenderContext&) const override;

private:
    SkSVGEllipse();

    SkRect resolve(const SkSVGLengthContext&) const;

    using INHERITED = SkSVGShape;
};

#endif // SkSVGEllipse_DEFINED
