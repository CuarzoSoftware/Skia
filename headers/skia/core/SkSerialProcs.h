/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSerialProcs_DEFINED
#define SkSerialProcs_DEFINED

#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/private/base/SkAPI.h"

#include <cstddef>
#include <optional>

class SkData;
class SkImage;
class SkPicture;
class SkTypeface;
class SkReadBuffer;
enum SkAlphaType : int;
namespace sktext::gpu {
    class Slug;
}

/**
 *  A serial-proc is asked to serialize the specified object (e.g. picture or image).
 *  If a data object is returned, it will be used (even if it is zero-length).
 *  If null is returned, then Skia will take its default action.
 *
 *  The default action for pictures is to use Skia's internal format.
 *  The default action for images is to encode either in its native format or PNG.
 *  The default action for typefaces is to use Skia's internal format.
 */

using SkSerialPictureProc = sk_sp<SkData> (*)(SkPicture*, void* ctx);
using SkSerialImageProc = sk_sp<SkData> (*)(SkImage*, void* ctx);
using SkSerialTypefaceProc = sk_sp<SkData> (*)(SkTypeface*, void* ctx);

/**
 *  Called with the encoded form of a picture (previously written with a custom
 *  SkSerialPictureProc proc). Return a picture object, or nullptr indicating failure.
 */
using SkDeserialPictureProc = sk_sp<SkPicture> (*)(const void* data, size_t length, void* ctx);

/**
 *  Called with the encoded form of an image. The proc can return an image object, or if it
 *  returns nullptr, then Skia will take its default action to try to create an image from the data.
 *
 *  This will also be used to decode the internal mipmap layers that are saved on some images.
 *
 *  An explicit SkAlphaType may have been encoded in the bytestream; if not, then the passed in
 *  optional will be not present.
 *
 *  Clients should set at least SkDeserialImageProc; SkDeserialImageFromDataProc may be called
 *  if the internal implementation has a SkData copy already. Implementations of SkDeserialImageProc
 *  must make a copy of any data they needed after the proc finishes, since the data will go away
 *  after serialization ends.
 */
#if !defined(SK_LEGACY_DESERIAL_IMAGE_PROC)
using SkDeserialImageProc = sk_sp<SkImage> (*)(const void* data, size_t length, void* ctx);
#else
using SkDeserialImageProc = sk_sp<SkImage> (*)(const void* data,
                                               size_t length,
                                               std::optional<SkAlphaType>,
                                               void* ctx);
#endif
using SkDeserialImageFromDataProc = sk_sp<SkImage> (*)(sk_sp<SkData>,
                                                       std::optional<SkAlphaType>,
                                                       void* ctx);

/**
 * Slugs are currently only deserializable with a GPU backend. Clients will not be able to
 * provide a custom mechanism here, but can enable Slug deserialization by calling
 * sktext::gpu::AddDeserialProcs to add Skia's implementation.
 */
using SkSlugProc = sk_sp<sktext::gpu::Slug> (*)(SkReadBuffer&, void* ctx);

/**
 *  Called with the encoded form of a typeface (previously written with a custom
 *  SkSerialTypefaceProc proc). Return a typeface object, or nullptr indicating failure.
 */
using SkDeserialTypefaceProc = sk_sp<SkTypeface> (*)(const void* data, size_t length, void* ctx);

struct SK_API SkSerialProcs {
    SkSerialPictureProc fPictureProc = nullptr;
    void*               fPictureCtx = nullptr;

    SkSerialImageProc   fImageProc = nullptr;
    void*               fImageCtx = nullptr;

    SkSerialTypefaceProc fTypefaceProc = nullptr;
    void*                fTypefaceCtx = nullptr;
};

struct SK_API SkDeserialProcs {
    SkDeserialPictureProc        fPictureProc = nullptr;
    void*                        fPictureCtx = nullptr;

    SkDeserialImageProc          fImageProc = nullptr;
    SkDeserialImageFromDataProc  fImageDataProc = nullptr;
    void*                        fImageCtx = nullptr;

    SkSlugProc                   fSlugProc = nullptr;
    void*                        fSlugCtx = nullptr;

    SkDeserialTypefaceProc       fTypefaceProc = nullptr;
    void*                        fTypefaceCtx = nullptr;

    // This looks like a flag, but it could be considered a proc as well (one that takes no
    // parameters and returns a bool). Given that there are only two valid implementations of that
    // proc, we just insert the bool directly.
    bool                         fAllowSkSL = true;
};

#endif
