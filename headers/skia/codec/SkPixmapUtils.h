/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixmapUtils_DEFINED
#define SkPixmapUtils_DEFINED

#include "CZ/skia/codec/SkEncodedOrigin.h"
#include "CZ/skia/core/SkImageInfo.h"
#include "CZ/skia/private/base/SkAPI.h"

class SkPixmap;

namespace SkPixmapUtils {
/**
 *  Copy the pixels in src into dst, applying the orientation transformations specified
 *  by origin. If the inputs are invalid, this returns false and no copy is made.
 */
SK_API bool Orient(const SkPixmap& dst, const SkPixmap& src, SkEncodedOrigin origin);

/**
 *  Return a copy of the provided ImageInfo with the width and height swapped.
 */
SK_API SkImageInfo SwapWidthHeight(const SkImageInfo& info);

}  // namespace SkPixmapUtils

#endif // SkPixmapUtils_DEFINED
