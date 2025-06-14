/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitter_DEFINED
#define SkBlitter_DEFINED

#include "CZ/skia/core/SkColor.h"
#include "CZ/skia/core/SkPixmap.h"
#include "CZ/skia/core/SkRect.h"
#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/core/SkRegion.h"
#include "CZ/skia/private/base/SkAssert.h"
#include "CZ/skia/private/base/SkCPUTypes.h"
#include "CZ/skia/private/base/SkDebug.h"
#include "CZ/skia/private/base/SkTo.h"
#include "CZ/skia/src/base/SkAutoMalloc.h"

#include <cstddef>
#include <cstdint>
#include <optional>

class SkArenaAlloc;
class SkMatrix;
class SkPaint;
class SkShader;
class SkSurfaceProps;
struct SkMask;
enum class SkDrawCoverage : bool;

/** SkBlitter and its subclasses are responsible for actually writing pixels
    into memory. Besides efficiency, they handle clipping and antialiasing.
    A SkBlitter subclass contains all the context needed to generate pixels
    for the destination and how src/generated pixels map to the destination.
    The coordinates passed to the blitX calls are in destination pixel space.
*/
class SkBlitter {
public:
    virtual ~SkBlitter();
    SkBlitter() = default;
    SkBlitter(const SkBlitter&) = delete;
    SkBlitter(SkBlitter&&) = delete;
    SkBlitter& operator=(const SkBlitter&) = delete;
    SkBlitter& operator=(SkBlitter&&) = delete;

    /// Blit a horizontal run of one or more pixels.
    virtual void blitH(int x, int y, int width) = 0;

    /// Blit a horizontal run of antialiased pixels; runs[] is a *sparse*
    /// zero-terminated run-length encoding of spans of constant alpha values.
    /// The runs[] and antialias[] work together to represent long runs of pixels with the same
    /// alphas. The runs[] contains the number of pixels with the same alpha, and antialias[]
    /// contain the coverage value for that number of pixels. The runs[] (and antialias[]) are
    /// encoded in a clever way. The runs array is zero terminated, and has enough entries for
    /// each pixel plus one, in most cases some of the entries will not contain valid data. An entry
    /// in the runs array contains the number of pixels (np) that have the same alpha value. The
    /// next np value is found np entries away. For example, if runs[0] = 7, then the next valid
    /// entry will by at runs[7]. The runs array and antialias[] are coupled by index. So, if the
    /// np entry is at runs[45] = 12 then the alpha value can be found at antialias[45] = 0x88.
    /// This would mean to use an alpha value of 0x88 for the next 12 pixels starting at pixel 45.
    virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) = 0;

    /// Blit a vertical run of pixels with a constant alpha value.
    virtual void blitV(int x, int y, int height, SkAlpha alpha);

    /// Blit a solid rectangle one or more pixels wide.
    virtual void blitRect(int x, int y, int width, int height);

    /** Blit a rectangle with one alpha-blended column on the left,
        width (zero or more) opaque pixels, and one alpha-blended column
        on the right.
        The result will always be at least two pixels wide.
    */
    virtual void blitAntiRect(int x, int y, int width, int height,
                              SkAlpha leftAlpha, SkAlpha rightAlpha);

    // Blit a rect in AA with size at least 3 x 3 (small rect has too many edge cases...)
    void blitFatAntiRect(const SkRect& rect);

    /// Blit a pattern of pixels defined by a rectangle-clipped mask;
    /// typically used for text.
    virtual void blitMask(const SkMask&, const SkIRect& clip);

    // (x, y), (x + 1, y)
    virtual void blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) {
        int16_t runs[3];
        uint8_t aa[2];

        runs[0] = 1;
        runs[1] = 1;
        runs[2] = 0;
        aa[0] = SkToU8(a0);
        aa[1] = SkToU8(a1);
        this->blitAntiH(x, y, aa, runs);
    }

    // (x, y), (x, y + 1)
    virtual void blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) {
        int16_t runs[2];
        uint8_t aa[1];

        runs[0] = 1;
        runs[1] = 0;
        aa[0] = SkToU8(a0);
        this->blitAntiH(x, y, aa, runs);
        // reset in case the clipping blitter modified runs
        runs[0] = 1;
        runs[1] = 0;
        aa[0] = SkToU8(a1);
        this->blitAntiH(x, y + 1, aa, runs);
    }

    /**
     * Special methods for blitters that can blit more than one row at a time.
     * This function returns the number of rows that this blitter could optimally
     * process at a time. It is still required to support blitting one scanline
     * at a time.
     */
    virtual int requestRowsPreserved() const { return 1; }


    struct DirectBlit {
        SkPixmap pm;
        uint64_t value; // low bits match pixmap's bitdepth
    };
    virtual std::optional<DirectBlit> canDirectBlit() { return {}; }

    /**
     * This function allocates memory for the blitter that the blitter then owns.
     * The memory can be used by the calling function at will, but it will be
     * released when the blitter's destructor is called. This function returns
     * nullptr if no persistent memory is needed by the blitter.
     */
    virtual void* allocBlitMemory(size_t sz) {
        return fBlitMemory.reset(sz, SkAutoMalloc::kReuse_OnShrink);
    }

    ///@name non-virtual helpers
    void blitMaskRegion(const SkMask& mask, const SkRegion& clip);
    void blitRectRegion(const SkIRect& rect, const SkRegion& clip);
    void blitRegion(const SkRegion& clip);
    ///@}

    /** @name Factories
        Return the correct blitter to use given the specified context.
     */
    static SkBlitter* Choose(const SkPixmap& dst,
                             const SkMatrix& ctm,
                             const SkPaint& paint,
                             SkArenaAlloc*,
                             SkDrawCoverage,
                             sk_sp<SkShader> clipShader,
                             const SkSurfaceProps& props);

    static SkBlitter* ChooseSprite(const SkPixmap& dst,
                                   const SkPaint&,
                                   const SkPixmap& src,
                                   int left, int top,
                                   SkArenaAlloc*, sk_sp<SkShader> clipShader);
    ///@}

    static bool UseLegacyBlitter(const SkPixmap&, const SkPaint&, const SkMatrix&);

protected:
    SkAutoMalloc fBlitMemory;
};

/** This blitter silently never draws anything.
*/
class SkNullBlitter final : public SkBlitter {
public:
    void blitH(int x, int y, int width) override {}
    void blitAntiH(int x, int y, const SkAlpha[], const int16_t runs[]) override {}
    void blitV(int x, int y, int height, SkAlpha alpha) override {}
    void blitRect(int x, int y, int width, int height) override {}
    void blitMask(const SkMask&, const SkIRect& clip) override {}
};

/** Wraps another (real) blitter, and ensures that the real blitter is only
    called with coordinates that have been clipped by the specified clipRect.
    This means the caller need not perform the clipping ahead of time.
*/
class SkRectClipBlitter final : public SkBlitter {
public:
    void init(SkBlitter* blitter, const SkIRect& clipRect) {
        SkASSERT(!clipRect.isEmpty());
        fBlitter = blitter;
        fClipRect = clipRect;
    }

    void blitH(int x, int y, int width) override;
    void blitAntiH(int x, int y, const SkAlpha[], const int16_t runs[]) override;
    void blitV(int x, int y, int height, SkAlpha alpha) override;
    void blitRect(int x, int y, int width, int height) override;
    void blitAntiRect(int x, int y, int width, int height,
                      SkAlpha leftAlpha, SkAlpha rightAlpha) override;
    void blitMask(const SkMask&, const SkIRect& clip) override;

    int requestRowsPreserved() const override {
        return fBlitter->requestRowsPreserved();
    }

    void* allocBlitMemory(size_t sz) override {
        return fBlitter->allocBlitMemory(sz);
    }

private:
    SkBlitter*  fBlitter;
    SkIRect     fClipRect;
};

/** Wraps another (real) blitter, and ensures that the real blitter is only
    called with coordinates that have been clipped by the specified clipRgn.
    This means the caller need not perform the clipping ahead of time.
*/
class SkRgnClipBlitter final : public SkBlitter {
public:
    void init(SkBlitter* blitter, const SkRegion* clipRgn) {
        SkASSERT(clipRgn && !clipRgn->isEmpty());
        fBlitter = blitter;
        fRgn = clipRgn;
    }

    void blitH(int x, int y, int width) override;
    void blitAntiH(int x, int y, const SkAlpha[], const int16_t runs[]) override;
    void blitV(int x, int y, int height, SkAlpha alpha) override;
    void blitRect(int x, int y, int width, int height) override;
    void blitAntiRect(int x, int y, int width, int height,
                      SkAlpha leftAlpha, SkAlpha rightAlpha) override;
    void blitMask(const SkMask&, const SkIRect& clip) override;

    int requestRowsPreserved() const override {
        return fBlitter->requestRowsPreserved();
    }

    void* allocBlitMemory(size_t sz) override {
        return fBlitter->allocBlitMemory(sz);
    }

private:
    SkBlitter*      fBlitter;
    const SkRegion* fRgn;
};

#ifdef SK_DEBUG
class SkRectClipCheckBlitter final : public SkBlitter {
public:
    void init(SkBlitter* blitter, const SkIRect& clipRect) {
        SkASSERT(blitter);
        SkASSERT(!clipRect.isEmpty());
        fBlitter = blitter;
        fClipRect = clipRect;
    }

    void blitH(int x, int y, int width) override;
    void blitAntiH(int x, int y, const SkAlpha[], const int16_t runs[]) override;
    void blitV(int x, int y, int height, SkAlpha alpha) override;
    void blitRect(int x, int y, int width, int height) override;
    void blitAntiRect(int x, int y, int width, int height,
                              SkAlpha leftAlpha, SkAlpha rightAlpha) override;
    void blitMask(const SkMask&, const SkIRect& clip) override;
    void blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) override;
    void blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) override;

    int requestRowsPreserved() const override {
        return fBlitter->requestRowsPreserved();
    }

    void* allocBlitMemory(size_t sz) override {
        return fBlitter->allocBlitMemory(sz);
    }

private:
    SkBlitter*  fBlitter;
    SkIRect     fClipRect;
};
#endif

/** Factory to set up the appropriate most-efficient wrapper blitter
    to apply a clip. Returns a pointer to a member, so lifetime must
    be managed carefully.
*/
class SkBlitterClipper {
public:
    SkBlitter*  apply(SkBlitter* blitter, const SkRegion* clip,
                      const SkIRect* bounds = nullptr);

private:
    SkNullBlitter       fNullBlitter;
    SkRectClipBlitter   fRectBlitter;
    SkRgnClipBlitter    fRgnBlitter;
};

#endif
