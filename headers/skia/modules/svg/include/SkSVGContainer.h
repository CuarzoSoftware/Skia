/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGContainer_DEFINED
#define SkSVGContainer_DEFINED

#include "cz/skia/core/SkPath.h"
#include "cz/skia/core/SkRect.h"
#include "cz/skia/core/SkRefCnt.h"
#include "cz/skia/private/base/SkAPI.h"
#include "cz/skia/private/base/SkTArray.h"
#include "cz/skia/modules/svg/include/SkSVGNode.h"
#include "cz/skia/modules/svg/include/SkSVGTransformableNode.h"

class SkSVGRenderContext;

class SK_API SkSVGContainer : public SkSVGTransformableNode {
public:
    void appendChild(sk_sp<SkSVGNode>) override;

protected:
    explicit SkSVGContainer(SkSVGTag);

    void onRender(const SkSVGRenderContext&) const override;

    SkPath onAsPath(const SkSVGRenderContext&) const override;

    SkRect onTransformableObjectBoundingBox(const SkSVGRenderContext&) const final;

    bool hasChildren() const final;

    template <typename NodeType, typename Func>
    void forEachChild(Func func) const {
        for (const auto& child : fChildren) {
            if (child->tag() == NodeType::tag) {
                func(static_cast<const NodeType*>(child.get()));
            }
        }
    }

    // TODO: convert remaining direct users to iterators, and make the container private.
    skia_private::STArray<1, sk_sp<SkSVGNode>, true> fChildren;

private:
    using INHERITED = SkSVGTransformableNode;
};

#endif // SkSVGContainer_DEFINED
