/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGImage_DEFINED
#define SkSVGImage_DEFINED

#include "cz/skia/core/SkImage.h"
#include "cz/skia/core/SkPath.h"
#include "cz/skia/core/SkRect.h"
#include "cz/skia/core/SkRefCnt.h"
#include "cz/skia/private/base/SkAPI.h"
#include "cz/skia/private/base/SkDebug.h"
#include "cz/skia/modules/svg/include/SkSVGNode.h"
#include "cz/skia/modules/svg/include/SkSVGTransformableNode.h"
#include "cz/skia/modules/svg/include/SkSVGTypes.h"

class SkSVGRenderContext;

namespace skresources {
class ResourceProvider;
}

class SK_API SkSVGImage final : public SkSVGTransformableNode {
public:
    static sk_sp<SkSVGImage> Make() {
        return sk_sp<SkSVGImage>(new SkSVGImage());
    }

    void appendChild(sk_sp<SkSVGNode>) override {
        SkDEBUGF("cannot append child nodes to this element.\n");
    }

    bool onPrepareToRender(SkSVGRenderContext*) const override;
    void onRender(const SkSVGRenderContext&) const override;
    SkPath onAsPath(const SkSVGRenderContext&) const override;
    SkRect onTransformableObjectBoundingBox(const SkSVGRenderContext&) const override;

    struct ImageInfo {
        sk_sp<SkImage> fImage;
        SkRect         fDst;
    };
    static ImageInfo LoadImage(const sk_sp<skresources::ResourceProvider>&,
                               const SkSVGIRI&,
                               const SkRect&,
                               SkSVGPreserveAspectRatio);

    SVG_ATTR(X                  , SkSVGLength             , SkSVGLength(0))
    SVG_ATTR(Y                  , SkSVGLength             , SkSVGLength(0))
    SVG_ATTR(Width              , SkSVGLength             , SkSVGLength(0))
    SVG_ATTR(Height             , SkSVGLength             , SkSVGLength(0))
    SVG_ATTR(Href               , SkSVGIRI                , SkSVGIRI())
    SVG_ATTR(PreserveAspectRatio, SkSVGPreserveAspectRatio, SkSVGPreserveAspectRatio())

protected:
    bool parseAndSetAttribute(const char*, const char*) override;

private:
    SkSVGImage() : INHERITED(SkSVGTag::kImage) {}

    using INHERITED = SkSVGTransformableNode;
};

#endif  // SkSVGImage_DEFINED
