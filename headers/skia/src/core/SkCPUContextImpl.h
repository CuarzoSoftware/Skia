/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skcpu_ContextImpl_DEFINED
#define skcpu_ContextImpl_DEFINED

#include "CZ/skia/core/SkCPUContext.h"
#include "CZ/skia/core/SkSurfaceProps.h"
#include "CZ/skia/src/core/SkResourceCache.h"

namespace skcpu {
class ContextImpl final : public Context {
public:
    ContextImpl() = default;

    static const ContextImpl* TODO();
};
}  // namespace skcpu

#endif
