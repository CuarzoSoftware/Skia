/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLMakeEpoxyEGLInterface_DEFINED
#define GrGLMakeEpoxyEGLInterface_DEFINED

#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/private/base/SkAPI.h"

struct GrGLInterface;

namespace GrGLInterfaces {
SK_API sk_sp<const GrGLInterface> MakeEpoxyEGL();
}

#endif  // GrGLMakeEpoxyEGLInterface_DEFINED
