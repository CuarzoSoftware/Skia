/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShader_DEFINED
#define SkShader_DEFINED

#include "CZ/skia/core/SkColor.h"
#include "CZ/skia/core/SkFlattenable.h"
#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/private/base/SkAPI.h"

class SkBlender;
class SkColorFilter;
class SkColorSpace;
class SkImage;
class SkMatrix;
enum class SkBlendMode;
enum class SkTileMode;
struct SkRect;
struct SkSamplingOptions;

/** \class SkShader
 *
 *  Shaders specify the premultiplied source color(s) for what is being drawn.
 *  If a paint has no shader, then the paint's color is used. If the paint has a
 *  shader, then the shader's color(s) are used instead, but they are
 *  modulated by the paint's alpha. This makes it easy to create a shader
 *  once (e.g. bitmap tiling or gradient) and then change its transparency
 *  w/o having to modify the original shader... only the paint's alpha needs
 *  to be modified.
 */
class SK_API SkShader : public SkFlattenable {
public:
    /**
     *  Returns true if the shader is guaranteed to produce only opaque
     *  colors, subject to the SkPaint using the shader to apply an opaque
     *  alpha value. Subclasses should override this to allow some
     *  optimizations.
     */
    virtual bool isOpaque() const { return false; }

    /**
     *  Iff this shader is backed by a single SkImage, return its ptr (the caller must ref this
     *  if they want to keep it longer than the lifetime of the shader). If not, return nullptr.
     */
    SkImage* isAImage(SkMatrix* localMatrix, SkTileMode xy[2]) const;

    bool isAImage() const {
        return this->isAImage(nullptr, (SkTileMode*)nullptr) != nullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //  Methods to create combinations or variants of shaders

    /**
     *  Return a shader that will apply the specified localMatrix to this shader.
     *  The specified matrix will be applied before any matrix associated with this shader.
     */
    sk_sp<SkShader> makeWithLocalMatrix(const SkMatrix&) const;

    /**
     *  Create a new shader that produces the same colors as invoking this shader and then applying
     *  the colorfilter.
     */
    sk_sp<SkShader> makeWithColorFilter(sk_sp<SkColorFilter>) const;

    /**
     *  Return a shader that will compute this shader in a specific color space.
     *  By default, all shaders operate in the destination (surface) color space.
     *  The results of a shader are still always converted to the destination - this
     *  API has no impact on simple shaders or images. Primarily, it impacts shaders
     *  that perform mathematical operations, like Blend shaders, or runtime shaders.
     */
    sk_sp<SkShader> makeWithWorkingColorSpace(sk_sp<SkColorSpace>) const;

private:
    SkShader() = default;
    friend class SkShaderBase;

    using INHERITED = SkFlattenable;
};

namespace SkShaders {
SK_API sk_sp<SkShader> Empty();
SK_API sk_sp<SkShader> Color(SkColor);
SK_API sk_sp<SkShader> Color(const SkColor4f&, sk_sp<SkColorSpace>);
SK_API sk_sp<SkShader> Blend(SkBlendMode mode, sk_sp<SkShader> dst, sk_sp<SkShader> src);
SK_API sk_sp<SkShader> Blend(sk_sp<SkBlender>, sk_sp<SkShader> dst, sk_sp<SkShader> src);
SK_API sk_sp<SkShader> CoordClamp(sk_sp<SkShader>, const SkRect& subset);

/*
 * Create an SkShader that will sample the 'image'. This is equivalent to SkImage::makeShader.
 */
SK_API sk_sp<SkShader> Image(sk_sp<SkImage> image,
                             SkTileMode tmx, SkTileMode tmy,
                             const SkSamplingOptions& options,
                             const SkMatrix* localMatrix = nullptr);
/*
 * Create an SkShader that will sample 'image' with minimal processing. This is equivalent to
 * SkImage::makeRawShader.
 */
SK_API sk_sp<SkShader> RawImage(sk_sp<SkImage> image,
                                SkTileMode tmx, SkTileMode tmy,
                                const SkSamplingOptions& options,
                                const SkMatrix* localMatrix = nullptr);
}

#endif
