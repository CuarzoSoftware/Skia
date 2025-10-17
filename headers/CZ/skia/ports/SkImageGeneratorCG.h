/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkImageGeneratorCG_DEFINED
#define SkImageGeneratorCG_DEFINED

// This is needed as clients may override the target platform
// using SkUserConfig
#include "CZ/skia/private/base/SkLoadUserConfig.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "CZ/skia/core/SkData.h"
#include "CZ/skia/core/SkImageGenerator.h"
#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/private/base/SkAPI.h"

#include <memory>

namespace SkImageGeneratorCG {
SK_API std::unique_ptr<SkImageGenerator> MakeFromEncodedCG(sk_sp<SkData>);
}  // namespace SkImageGeneratorCG

#endif  // defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#endif  // SkImageGeneratorCG_DEFINED
