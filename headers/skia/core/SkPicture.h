/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPicture_DEFINED
#define SkPicture_DEFINED

#include "CZ/skia/core/SkRect.h"
#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/core/SkShader.h"  // IWYU pragma: keep
#include "CZ/skia/core/SkTypes.h"

#include <atomic>
#include <cstddef>
#include <cstdint>

class SkCanvas;
class SkData;
class SkMatrix;
class SkStream;
class SkWStream;
enum class SkFilterMode;
struct SkDeserialProcs;
struct SkSerialProcs;

// TODO(kjlubick) Remove this after cleaning up clients
#include "CZ/skia/core/SkTileMode.h"  // IWYU pragma: keep

/** \class SkPicture
    SkPicture records drawing commands made to SkCanvas. The command stream may be
    played in whole or in part at a later time.

    SkPicture is an abstract class. SkPicture may be generated by SkPictureRecorder
    or SkDrawable, or from SkPicture previously saved to SkData or SkStream.

    SkPicture may contain any SkCanvas drawing command, as well as one or more
    SkCanvas matrix or SkCanvas clip. SkPicture has a cull SkRect, which is used as
    a bounding box hint. To limit SkPicture bounds, use SkCanvas clip when
    recording or drawing SkPicture.
*/
class SK_API SkPicture : public SkRefCnt {
public:
    ~SkPicture() override;

    /** Recreates SkPicture that was serialized into a stream. Returns constructed SkPicture
        if successful; otherwise, returns nullptr. Fails if data does not permit
        constructing valid SkPicture.

        procs->fPictureProc permits supplying a custom function to decode SkPicture.
        If procs->fPictureProc is nullptr, default decoding is used. procs->fPictureCtx
        may be used to provide user context to procs->fPictureProc; procs->fPictureProc
        is called with a pointer to data, data byte length, and user context.

        @param stream  container for serial data
        @param procs   custom serial data decoders; may be nullptr
        @return        SkPicture constructed from stream data
    */
    static sk_sp<SkPicture> MakeFromStream(SkStream* stream,
                                           const SkDeserialProcs* procs = nullptr);

    /** Recreates SkPicture that was serialized into data. Returns constructed SkPicture
        if successful; otherwise, returns nullptr. Fails if data does not permit
        constructing valid SkPicture.

        procs->fPictureProc permits supplying a custom function to decode SkPicture.
        If procs->fPictureProc is nullptr, default decoding is used. procs->fPictureCtx
        may be used to provide user context to procs->fPictureProc; procs->fPictureProc
        is called with a pointer to data, data byte length, and user context.

        @param data   container for serial data
        @param procs  custom serial data decoders; may be nullptr
        @return       SkPicture constructed from data
    */
    static sk_sp<SkPicture> MakeFromData(const SkData* data,
                                         const SkDeserialProcs* procs = nullptr);

    /**

        @param data   pointer to serial data
        @param size   size of data
        @param procs  custom serial data decoders; may be nullptr
        @return       SkPicture constructed from data
    */
    static sk_sp<SkPicture> MakeFromData(const void* data, size_t size,
                                         const SkDeserialProcs* procs = nullptr);

    /** \class SkPicture::AbortCallback
        AbortCallback is an abstract class. An implementation of AbortCallback may
        passed as a parameter to SkPicture::playback, to stop it before all drawing
        commands have been processed.

        If AbortCallback::abort returns true, SkPicture::playback is interrupted.
    */
    class SK_API AbortCallback {
    public:
        /** Has no effect.
        */
        virtual ~AbortCallback() = default;

        /** Stops SkPicture playback when some condition is met. A subclass of
            AbortCallback provides an override for abort() that can stop SkPicture::playback.

            The part of SkPicture drawn when aborted is undefined. SkPicture instantiations are
            free to stop drawing at different points during playback.

            If the abort happens inside one or more calls to SkCanvas::save(), stack
            of SkCanvas matrix and SkCanvas clip values is restored to its state before
            SkPicture::playback was called.

            @return  true to stop playback

        example: https://fiddle.skia.org/c/@Picture_AbortCallback_abort
        */
        virtual bool abort() = 0;

    protected:
        AbortCallback() = default;
        AbortCallback(const AbortCallback&) = delete;
        AbortCallback& operator=(const AbortCallback&) = delete;
    };

    /** Replays the drawing commands on the specified canvas. In the case that the
        commands are recorded, each command in the SkPicture is sent separately to canvas.

        To add a single command to draw SkPicture to recording canvas, call
        SkCanvas::drawPicture instead.

        @param canvas    receiver of drawing commands
        @param callback  allows interruption of playback

        example: https://fiddle.skia.org/c/@Picture_playback
    */
    virtual void playback(SkCanvas* canvas, AbortCallback* callback = nullptr) const = 0;

    /** Returns cull SkRect for this picture, passed in when SkPicture was created.
        Returned SkRect does not specify clipping SkRect for SkPicture; cull is hint
        of SkPicture bounds.

        SkPicture is free to discard recorded drawing commands that fall outside
        cull.

        @return  bounds passed when SkPicture was created

        example: https://fiddle.skia.org/c/@Picture_cullRect
    */
    virtual SkRect cullRect() const = 0;

    /** Returns a non-zero value unique among SkPicture in Skia process.

        @return  identifier for SkPicture
    */
    uint32_t uniqueID() const { return fUniqueID; }

    /** Returns storage containing SkData describing SkPicture, using optional custom
        encoders.

        procs->fPictureProc permits supplying a custom function to encode SkPicture.
        If procs->fPictureProc is nullptr, default encoding is used. procs->fPictureCtx
        may be used to provide user context to procs->fPictureProc; procs->fPictureProc
        is called with a pointer to SkPicture and user context.

        The default behavior for serializing SkImages is to encode a nullptr. Should
        clients want to, for example, encode these SkImages as PNGs so they can be
        deserialized, they must provide SkSerialProcs with the fImageProc set to do so.

        @param procs  custom serial data encoders; may be nullptr
        @return       storage containing serialized SkPicture

        example: https://fiddle.skia.org/c/@Picture_serialize
    */
    sk_sp<SkData> serialize(const SkSerialProcs* procs = nullptr) const;

    /** Writes picture to stream, using optional custom encoders.

        procs->fPictureProc permits supplying a custom function to encode SkPicture.
        If procs->fPictureProc is nullptr, default encoding is used. procs->fPictureCtx
        may be used to provide user context to procs->fPictureProc; procs->fPictureProc
        is called with a pointer to SkPicture and user context.

        The default behavior for serializing SkImages is to encode a nullptr. Should
        clients want to, for example, encode these SkImages as PNGs so they can be
        deserialized, they must provide SkSerialProcs with the fImageProc set to do so.

        @param stream  writable serial data stream
        @param procs   custom serial data encoders; may be nullptr

        example: https://fiddle.skia.org/c/@Picture_serialize_2
    */
    void serialize(SkWStream* stream, const SkSerialProcs* procs = nullptr) const;

    /** Returns a placeholder SkPicture. Result does not draw, and contains only
        cull SkRect, a hint of its bounds. Result is immutable; it cannot be changed
        later. Result identifier is unique.

        Returned placeholder can be intercepted during playback to insert other
        commands into SkCanvas draw stream.

        @param cull  placeholder dimensions
        @return      placeholder with unique identifier

        example: https://fiddle.skia.org/c/@Picture_MakePlaceholder
    */
    static sk_sp<SkPicture> MakePlaceholder(SkRect cull);

    /** Returns the approximate number of operations in SkPicture. Returned value
        may be greater or less than the number of SkCanvas calls
        recorded: some calls may be recorded as more than one operation, other
        calls may be optimized away.

        @param nested  if true, include the op-counts of nested pictures as well, else
                       just return count the ops in the top-level picture.
        @return  approximate operation count

        example: https://fiddle.skia.org/c/@Picture_approximateOpCount
    */
    virtual int approximateOpCount(bool nested = false) const = 0;

    /** Returns the approximate byte size of SkPicture. Does not include large objects
        referenced by SkPicture.

        @return  approximate size

        example: https://fiddle.skia.org/c/@Picture_approximateBytesUsed
    */
    virtual size_t approximateBytesUsed() const = 0;

    /** Return a new shader that will draw with this picture.
     *
     *  @param tmx  The tiling mode to use when sampling in the x-direction.
     *  @param tmy  The tiling mode to use when sampling in the y-direction.
     *  @param mode How to filter the tiles
     *  @param localMatrix Optional matrix used when sampling
     *  @param tileRect The tile rectangle in picture coordinates: this represents the subset
     *                  (or superset) of the picture used when building a tile. It is not
     *                  affected by localMatrix and does not imply scaling (only translation
     *                  and cropping). If null, the tile rect is considered equal to the picture
     *                  bounds.
     *  @return     Returns a new shader object. Note: this function never returns null.
     */
    sk_sp<SkShader> makeShader(SkTileMode tmx, SkTileMode tmy, SkFilterMode mode,
                               const SkMatrix* localMatrix, const SkRect* tileRect) const;

    sk_sp<SkShader> makeShader(SkTileMode tmx, SkTileMode tmy, SkFilterMode mode) const {
        return this->makeShader(tmx, tmy, mode, nullptr, nullptr);
    }

private:
    // Allowed subclasses.
    SkPicture();
    friend class SkBigPicture;
    friend class SkEmptyPicture;
    friend class SkPicturePriv;

    void serialize(SkWStream*, const SkSerialProcs*, class SkRefCntSet* typefaces,
        bool textBlobsOnly=false) const;
    static sk_sp<SkPicture> MakeFromStreamPriv(SkStream*, const SkDeserialProcs*,
                                               class SkTypefacePlayback*,
                                               int recursionLimit);
    friend class SkPictureData;

    /** Return true if the SkStream/Buffer represents a serialized picture, and
     fills out SkPictInfo. After this function returns, the data source is not
     rewound so it will have to be manually reset before passing to
     MakeFromStream or MakeFromBuffer. Note, MakeFromStream and
     MakeFromBuffer perform this check internally so these entry points are
     intended for stand alone tools.
     If false is returned, SkPictInfo is unmodified.
     */
    static bool StreamIsSKP(SkStream*, struct SkPictInfo*);
    static bool BufferIsSKP(class SkReadBuffer*, struct SkPictInfo*);
    friend bool SkPicture_StreamIsSKP(SkStream*, struct SkPictInfo*);

    // Returns NULL if this is not an SkBigPicture.
    virtual const class SkBigPicture* asSkBigPicture() const { return nullptr; }

    static bool IsValidPictInfo(const struct SkPictInfo& info);
    static sk_sp<SkPicture> Forwardport(const struct SkPictInfo&,
                                        const class SkPictureData*,
                                        class SkReadBuffer* buffer);

    struct SkPictInfo createHeader() const;
    class SkPictureData* backport() const;

    uint32_t fUniqueID;
    mutable std::atomic<bool> fAddedToCache{false};
};

#endif
