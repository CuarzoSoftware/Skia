/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecordCanvas_DEFINED
#define SkRecordCanvas_DEFINED

#include "CZ/skia/core/SkCPURecorder.h"
#include "CZ/skia/core/SkCanvasVirtualEnforcer.h"
#include "CZ/skia/core/SkColor.h"
#include "CZ/skia/core/SkM44.h"
#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/core/SkSamplingOptions.h"
#include "CZ/skia/core/SkScalar.h"
#include "CZ/skia/core/SkTypes.h"
#include "CZ/skia/private/base/SkNoncopyable.h"
#include "CZ/skia/private/base/SkTDArray.h"
#include "CZ/skia/utils/SkNoDrawCanvas.h"
#include "CZ/skia/src/core/SkBigPicture.h"

#include <cstddef>
#include <memory>
#include <utility>

class SkBlender;
class SkData;
class SkDrawable;
class SkImage;
class SkMatrix;
class SkMesh;
class SkPaint;
class SkPath;
class SkPicture;
class SkRRect;
class SkRecord;
class SkRecorder;
class SkRegion;
class SkShader;
class SkSurface;
class SkSurfaceProps;
class SkTextBlob;
class SkVertices;
enum class SkBlendMode;
enum class SkClipOp;
struct SkDrawShadowRec;
struct SkImageInfo;
struct SkPoint;
struct SkRSXform;
struct SkRect;

namespace sktext {
class GlyphRunList;
namespace gpu {
class Slug;
}
}  // namespace sktext

class SkDrawableList : SkNoncopyable {
public:
    SkDrawableList() {}
    ~SkDrawableList();

    int count() const { return fArray.size(); }
    SkDrawable* const* begin() const { return fArray.begin(); }
    SkDrawable* const* end() const { return fArray.end(); }

    void append(SkDrawable* drawable);

    // Return a new or ref'd array of pictures that were snapped from our drawables.
    SkBigPicture::SnapshotArray* newDrawableSnapshot();

private:
    SkTDArray<SkDrawable*> fArray;
};

// SkRecordCanvas provides an SkCanvas interface for recording into an SkRecord.

class SkRecordCanvas final : public SkCanvasVirtualEnforcer<SkNoDrawCanvas> {
public:
    // Does not take ownership of the SkRecord.
    SkRecordCanvas(SkRecord*, int width, int height);  // TODO: remove
    SkRecordCanvas(SkRecord*, const SkRect& bounds);

    void reset(SkRecord*, const SkRect& bounds);

    size_t approxBytesUsedBySubPictures() const { return fApproxBytesUsedBySubPictures; }

    SkDrawableList* getDrawableList() const { return fDrawableList.get(); }
    std::unique_ptr<SkDrawableList> detachDrawableList() { return std::move(fDrawableList); }

    // Make SkRecordCanvas forget entirely about its SkRecord*; all calls to SkRecordCanvas will
    // fail.
    void forgetRecord();

    void willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    bool onDoSaveBehind(const SkRect*) override;
    void willRestore() override {}
    void didRestore() override;
    SkRecorder* baseRecorder() const override {
        // TODO(kjlubick) this class should implement SkRecorder (or maybe Record should).
        return skcpu::Recorder::TODO();
    }

    void didConcat44(const SkM44&) override;
    void didSetM44(const SkM44&) override;
    void didScale(SkScalar, SkScalar) override;
    void didTranslate(SkScalar, SkScalar) override;

    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
    void onDrawTextBlob(const SkTextBlob* blob,
                        SkScalar x,
                        SkScalar y,
                        const SkPaint& paint) override;
    void onDrawSlug(const sktext::gpu::Slug* slug, const SkPaint& paint) override;
    void onDrawGlyphRunList(const sktext::GlyphRunList& glyphRunList,
                            const SkPaint& paint) override;
    void onDrawPatch(const SkPoint cubics[12],
                     const SkColor colors[4],
                     const SkPoint texCoords[4],
                     SkBlendMode,
                     const SkPaint& paint) override;

    void onDrawPaint(const SkPaint&) override;
    void onDrawBehind(const SkPaint&) override;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawRegion(const SkRegion&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;

    void onDrawImage2(
            const SkImage*, SkScalar, SkScalar, const SkSamplingOptions&, const SkPaint*) override;
    void onDrawImageRect2(const SkImage*,
                          const SkRect&,
                          const SkRect&,
                          const SkSamplingOptions&,
                          const SkPaint*,
                          SrcRectConstraint) override;
    void onDrawImageLattice2(
            const SkImage*, const Lattice&, const SkRect&, SkFilterMode, const SkPaint*) override;
    void onDrawAtlas2(const SkImage*,
                      const SkRSXform[],
                      const SkRect[],
                      const SkColor[],
                      int,
                      SkBlendMode,
                      const SkSamplingOptions&,
                      const SkRect*,
                      const SkPaint*) override;

    void onDrawVerticesObject(const SkVertices*, SkBlendMode, const SkPaint&) override;

    void onDrawMesh(const SkMesh&, sk_sp<SkBlender>, const SkPaint&) override;

    void onDrawShadowRec(const SkPath&, const SkDrawShadowRec&) override;

    void onClipRect(const SkRect& rect, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect& rrect, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath& path, SkClipOp, ClipEdgeStyle) override;
    void onClipShader(sk_sp<SkShader>, SkClipOp) override;
    void onClipRegion(const SkRegion& deviceRgn, SkClipOp) override;
    void onResetClip() override;

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

    void onDrawAnnotation(const SkRect&, const char[], SkData*) override;

    void onDrawEdgeAAQuad(
            const SkRect&, const SkPoint[4], QuadAAFlags, const SkColor4f&, SkBlendMode) override;
    void onDrawEdgeAAImageSet2(const ImageSetEntry[],
                               int count,
                               const SkPoint[],
                               const SkMatrix[],
                               const SkSamplingOptions&,
                               const SkPaint*,
                               SrcRectConstraint) override;

    sk_sp<SkSurface> onNewSurface(const SkImageInfo&, const SkSurfaceProps&) override;

private:
    template <typename T> T* copy(const T*);

    template <typename T> T* copy(const T[], size_t count);

    template <typename T, typename... Args> void append(Args&&...);

    size_t fApproxBytesUsedBySubPictures;
    SkRecord* fRecord;
    std::unique_ptr<SkDrawableList> fDrawableList;
};

#endif  // SkRecordCanvas_DEFINED
