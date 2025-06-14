/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScanPriv_DEFINED
#define SkScanPriv_DEFINED

#include "CZ/skia/core/SkPath.h"
#include "CZ/skia/src/core/SkBlitter.h"
#include "CZ/skia/src/core/SkScan.h"

// controls how much we super-sample (when we use that scan convertion)
#define SK_SUPERSAMPLE_SHIFT    2

class SkScanClipper {
public:
    SkScanClipper(SkBlitter* blitter, const SkRegion* clip, const SkIRect& bounds,
                  bool skipRejectTest = false, bool boundsPreClipped = false);

    SkBlitter*      getBlitter() const { return fBlitter; }
    const SkIRect*  getClipRect() const { return fClipRect; }

private:
    SkRectClipBlitter   fRectBlitter;
    SkRgnClipBlitter    fRgnBlitter;
#ifdef SK_DEBUG
    SkRectClipCheckBlitter fRectClipCheckBlitter;
#endif
    SkBlitter*          fBlitter;
    const SkIRect*      fClipRect;
};

// blit the rects above and below avoid, clipped to clip
void sk_blit_above(SkBlitter*, const SkIRect& avoid, const SkRegion& clip);
void sk_blit_below(SkBlitter*, const SkIRect& avoid, const SkRegion& clip);

template<class EdgeType>
static inline void remove_edge(EdgeType* edge) {
    edge->fPrev->fNext = edge->fNext;
    edge->fNext->fPrev = edge->fPrev;
}

template<class EdgeType>
static inline void insert_edge_after(EdgeType* edge, EdgeType* afterMe) {
    edge->fPrev = afterMe;
    edge->fNext = afterMe->fNext;
    afterMe->fNext->fPrev = edge;
    afterMe->fNext = edge;
}

template<class EdgeType>
void backward_insert_edge_based_on_x(EdgeType* edge) {
    SkFixed x = edge->fX;
    EdgeType* prev = edge->fPrev;
    while (prev->fPrev && prev->fX > x) {
        prev = prev->fPrev;
    }
    if (prev->fNext != edge) {
        remove_edge(edge);
        insert_edge_after(edge, prev);
    }
}

// Start from the right side, searching backwards for the point to begin the new edge list
// insertion, marching forwards from here. The implementation could have started from the left
// of the prior insertion, and search to the right, or with some additional caching, binary
// search the starting point. More work could be done to determine optimal new edge insertion.
template<class EdgeType>
EdgeType* backward_insert_start(EdgeType* prev, SkFixed x) {
    while (prev->fPrev && prev->fX > x) {
        prev = prev->fPrev;
    }
    return prev;
}

#endif
