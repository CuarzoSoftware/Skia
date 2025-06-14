/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRuntimeBlender_DEFINED
#define SkRuntimeBlender_DEFINED

#include "cz/skia/core/SkData.h"
#include "cz/skia/core/SkFlattenable.h"
#include "cz/skia/core/SkRefCnt.h"
#include "cz/skia/effects/SkRuntimeEffect.h"
#include "cz/skia/private/base/SkSpan_impl.h"
#include "cz/skia/src/core/SkBlenderBase.h"

#include <utility>
#include <vector>

class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

class SkRuntimeBlender : public SkBlenderBase {
public:
    SkRuntimeBlender(sk_sp<SkRuntimeEffect> effect,
                     sk_sp<const SkData> uniforms,
                     SkSpan<const SkRuntimeEffect::ChildPtr> children)
            : fEffect(std::move(effect))
            , fUniforms(std::move(uniforms))
            , fChildren(children.begin(), children.end()) {}

    SkRuntimeEffect* asRuntimeEffect() const override { return fEffect.get(); }

    BlenderType type() const override { return BlenderType::kRuntime; }

    bool onAppendStages(const SkStageRec& rec) const override;

    void flatten(SkWriteBuffer& buffer) const override;

    SK_FLATTENABLE_HOOKS(SkRuntimeBlender)

    sk_sp<SkRuntimeEffect> effect() const { return fEffect; }
    sk_sp<const SkData> uniforms() const { return fUniforms; }
    SkSpan<const SkRuntimeEffect::ChildPtr> children() const { return fChildren; }

private:
    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<const SkData> fUniforms;
    std::vector<SkRuntimeEffect::ChildPtr> fChildren;
};

#endif
