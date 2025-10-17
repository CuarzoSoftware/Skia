/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLMakeMacInterface_DEFINED
#define GrGLMakeMacInterface_DEFINED

#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/private/base/SkAPI.h"

struct GrGLInterface;

namespace GrGLInterfaces {
SK_API sk_sp<const GrGLInterface> MakeMac();
}

#endif  // GrGLMakeMacInterface_DEFINED
