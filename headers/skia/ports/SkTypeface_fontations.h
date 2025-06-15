/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_fontations_DEFINED
#define SkTypeface_fontations_DEFINED

#include "CZ/skia/core/SkFontArguments.h"
#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/core/SkTypeface.h"
#include "CZ/skia/core/SkTypes.h"

#include <memory>

SK_API sk_sp<SkTypeface> SkTypeface_Make_Fontations(std::unique_ptr<SkStreamAsset> fontData,
                                                    const SkFontArguments& args);

SK_API sk_sp<SkTypeface> SkTypeface_Make_Fontations(sk_sp<SkData> fontData,
                                                    const SkFontArguments& args);

#endif
