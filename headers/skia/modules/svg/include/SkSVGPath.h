/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGPath_DEFINED
#define SkSVGPath_DEFINED

#include "cz/skia/core/SkPath.h"
#include "cz/skia/core/SkRect.h"
#include "cz/skia/core/SkRefCnt.h"
#include "cz/skia/private/base/SkAPI.h"
#include "cz/skia/modules/svg/include/SkSVGNode.h"
#include "cz/skia/modules/svg/include/SkSVGShape.h"

class SkCanvas;
class SkPaint;
class SkSVGLengthContext;
class SkSVGRenderContext;
enum class SkPathFillType;

class SK_API SkSVGPath final : public SkSVGShape {
public:
    static sk_sp<SkSVGPath> Make() { return sk_sp<SkSVGPath>(new SkSVGPath()); }

    SVG_ATTR(Path, SkPath, SkPath())

protected:
    bool parseAndSetAttribute(const char*, const char*) override;

    void onDraw(SkCanvas*, const SkSVGLengthContext&, const SkPaint&,
                SkPathFillType) const override;

    SkPath onAsPath(const SkSVGRenderContext&) const override;

    SkRect onTransformableObjectBoundingBox(const SkSVGRenderContext&) const override;

private:
    SkSVGPath();

    using INHERITED = SkSVGShape;
};

#endif // SkSVGPath_DEFINED
