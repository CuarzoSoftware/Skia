/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGShape_DEFINED
#define SkSVGShape_DEFINED

#include "CZ/skia/core/SkRect.h"
#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/private/base/SkAPI.h"
#include "CZ/skia/modules/svg/include/SkSVGTransformableNode.h"

class SkCanvas;
class SkPaint;
class SkSVGLengthContext;
class SkSVGNode;
class SkSVGRenderContext;
enum class SkPathFillType;
enum class SkSVGTag;

class SK_API SkSVGShape : public SkSVGTransformableNode {
public:
    void appendChild(sk_sp<SkSVGNode>) override;

protected:
    SkSVGShape(SkSVGTag);

    void onRender(const SkSVGRenderContext&) const final;

    virtual void onDraw(SkCanvas*, const SkSVGLengthContext&, const SkPaint&,
                        SkPathFillType) const = 0;

private:
    using INHERITED = SkSVGTransformableNode;
};

#endif // SkSVGShape_DEFINED
