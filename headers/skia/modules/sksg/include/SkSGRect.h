/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGRect_DEFINED
#define SkSGRect_DEFINED

#include "CZ/skia/core/SkPath.h"
#include "CZ/skia/core/SkPathTypes.h"
#include "CZ/skia/core/SkRRect.h"
#include "CZ/skia/core/SkRect.h"
#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/core/SkScalar.h"
#include "CZ/skia/private/base/SkTo.h"
#include "CZ/skia/modules/sksg/include/SkSGGeometryNode.h"
#include "CZ/skia/modules/sksg/include/SkSGNode.h"

#include <cstdint>

class SkCanvas;
class SkMatrix;
class SkPaint;
struct SkPoint;

namespace sksg {
class InvalidationController;

/**
 * Concrete Geometry node, wrapping an SkRect.
 */
class Rect final : public GeometryNode {
public:
    static sk_sp<Rect> Make()                { return sk_sp<Rect>(new Rect(SkRect::MakeEmpty())); }
    static sk_sp<Rect> Make(const SkRect& r) { return sk_sp<Rect>(new Rect(r)); }

    SG_ATTRIBUTE(L, SkScalar, fRect.fLeft  )
    SG_ATTRIBUTE(T, SkScalar, fRect.fTop   )
    SG_ATTRIBUTE(R, SkScalar, fRect.fRight )
    SG_ATTRIBUTE(B, SkScalar, fRect.fBottom)

    SG_MAPPED_ATTRIBUTE(Direction        , SkPathDirection, fAttrContaier)
    SG_MAPPED_ATTRIBUTE(InitialPointIndex, uint8_t        , fAttrContaier)

protected:
    void onClip(SkCanvas*, bool antiAlias) const override;
    void onDraw(SkCanvas*, const SkPaint&) const override;
    bool onContains(const SkPoint&)        const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    explicit Rect(const SkRect&);

    SkRect   fRect;

    struct AttrContainer {
        uint8_t fDirection         : 1;
        uint8_t fInitialPointIndex : 2;

        SkPathDirection getDirection() const {
            return static_cast<SkPathDirection>(fDirection);
        }
        void setDirection(SkPathDirection dir) { fDirection = SkTo<uint8_t>(dir); }

        uint8_t getInitialPointIndex() const { return fInitialPointIndex; }
        void setInitialPointIndex(uint8_t idx) { fInitialPointIndex = idx; }
    };
    AttrContainer fAttrContaier = { (int)SkPathDirection::kCW, 0 };

    using INHERITED = GeometryNode;
};

/**
 * Concrete Geometry node, wrapping an SkRRect.
 */
class RRect final : public GeometryNode {
public:
    static sk_sp<RRect> Make()                  { return sk_sp<RRect>(new RRect(SkRRect())); }
    static sk_sp<RRect> Make(const SkRRect& rr) { return sk_sp<RRect>(new RRect(rr)); }

    SG_ATTRIBUTE(RRect, SkRRect, fRRect)

    SG_MAPPED_ATTRIBUTE(Direction        , SkPathDirection, fAttrContaier)
    SG_MAPPED_ATTRIBUTE(InitialPointIndex, uint8_t          , fAttrContaier)

protected:
    void onClip(SkCanvas*, bool antiAlias) const override;
    void onDraw(SkCanvas*, const SkPaint&) const override;
    bool onContains(const SkPoint&)        const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    explicit RRect(const SkRRect&);

    SkRRect fRRect;

    struct AttrContainer {
        uint8_t fDirection         : 1;
        uint8_t fInitialPointIndex : 2;

        SkPathDirection getDirection() const {
            return static_cast<SkPathDirection>(fDirection);
        }
        void setDirection(SkPathDirection dir) { fDirection = SkTo<uint8_t>(dir); }

        uint8_t getInitialPointIndex() const { return fInitialPointIndex; }
        void setInitialPointIndex(uint8_t idx) { fInitialPointIndex = idx; }
    };
    AttrContainer fAttrContaier = { (int)SkPathDirection::kCW, 0 };

    using INHERITED = GeometryNode;
};

} // namespace sksg

#endif // SkSGRect_DEFINED
