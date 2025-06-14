/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_Precompile_DEFINED
#define skgpu_graphite_precompile_Precompile_DEFINED

#include "CZ/skia/core/SkColorSpace.h"
#include "CZ/skia/core/SkColorType.h"
#include "CZ/skia/core/SkSpan.h"
#include "CZ/skia/gpu/graphite/GraphiteTypes.h"

namespace skgpu::graphite {

class PaintOptions;
class PrecompileContext;

/**
 *  Describes the required properties of a RenderPass that will be combined with the
 *  other portions of the Precompilation API (i.e., paintOptions and drawTypes) to yield
 *  a pipeline.
 */
struct SK_API RenderPassProperties {
    bool operator==(const RenderPassProperties& other) const {
        return fDSFlags == other.fDSFlags &&
               fDstCT == other.fDstCT &&
               fRequiresMSAA == other.fRequiresMSAA &&
               SkColorSpace::Equals(fDstCS.get(), other.fDstCS.get());
    }
    bool operator!= (const RenderPassProperties& other) const { return !(*this == other); }

    DepthStencilFlags   fDSFlags      = DepthStencilFlags::kNone;
    SkColorType         fDstCT        = kRGBA_8888_SkColorType;
    sk_sp<SkColorSpace> fDstCS        = nullptr;
    bool                fRequiresMSAA = false;
};

/**
 * Precompilation allows clients to create pipelines ahead of time based on what they expect
 * to draw. This can reduce performance hitches, due to inline compilation, during the actual
 * drawing. Graphite will always be able to perform an inline compilation if some SkPaint
 * combination was omitted from precompilation.
 *
 *   @param precompileContext    thread-safe helper holding required portions of the Context
 *   @param paintOptions         captures a set of SkPaints that will be drawn
 *   @param drawTypes            communicates which primitives those paints will be drawn with
 *   @param renderPassProperties describes the RenderPasses needed for the desired Pipelines
 */
void SK_API Precompile(PrecompileContext* precompileContext,
                       const PaintOptions& paintOptions,
                       DrawTypeFlags drawTypes,
                       SkSpan<const RenderPassProperties> renderPassProperties);

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_Precompile_DEFINED
