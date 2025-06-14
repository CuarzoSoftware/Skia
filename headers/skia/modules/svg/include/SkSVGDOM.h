/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGDOM_DEFINED
#define SkSVGDOM_DEFINED

#include "CZ/skia/core/SkFontMgr.h"
#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/core/SkSize.h"
#include "CZ/skia/private/base/SkAPI.h"
#include "CZ/skia/modules/skresources/include/SkResources.h"
#include "CZ/skia/modules/skshaper/include/SkShaper_factory.h"
#include "CZ/skia/modules/svg/include/SkSVGIDMapper.h"
#include "CZ/skia/modules/svg/include/SkSVGSVG.h"

class SkCanvas;
class SkSVGNode;
class SkStream;
struct SkSVGPresentationContext;

class SK_API SkSVGDOM : public SkRefCnt {
public:
    class Builder final {
    public:
        /**
         * Specify a font manager for loading fonts (e.g. from the system) to render <text>
         * SVG nodes.
         * If this is not set, but a font is required as part of rendering, the text will
         * not be displayed.
         */
        Builder& setFontManager(sk_sp<SkFontMgr>);

        /**
         * Specify a resource provider for loading images etc.
         */
        Builder& setResourceProvider(sk_sp<skresources::ResourceProvider>);

        /**
         * Specify the callbacks for dealing with shaping text. See also
         * modules/skshaper/utils/FactoryHelpers.h
         */
        Builder& setTextShapingFactory(sk_sp<SkShapers::Factory>);

        sk_sp<SkSVGDOM> make(SkStream&) const;

    private:
        sk_sp<SkFontMgr>                             fFontMgr;
        sk_sp<skresources::ResourceProvider>         fResourceProvider;
        sk_sp<SkShapers::Factory>                    fTextShapingFactory;
    };

    static sk_sp<SkSVGDOM> MakeFromStream(SkStream& str);

    /**
     * Returns the root (outermost) SVG element.
     */
    SkSVGSVG* getRoot() const { return fRoot.get(); }

    /**
     * Specify a "container size" for the SVG dom.
     *
     * This is used to resolve the initial viewport when the root SVG width/height are specified
     * in relative units.
     *
     * If the root dimensions are in absolute units, then the container size has no effect since
     * the initial viewport is fixed.
     */
    void setContainerSize(const SkSize&);

    /**
     * DEPRECATED: use getRoot()->intrinsicSize() to query the root element intrinsic size.
     *
     * Returns the SVG dom container size.
     *
     * If the client specified a container size via setContainerSize(), then the same size is
     * returned.
     *
     * When unspecified by clients, this returns the intrinsic size of the root element, as defined
     * by its width/height attributes.  If either width or height is specified in relative units
     * (e.g. "100%"), then the corresponding intrinsic size dimension is zero.
     */
    const SkSize& containerSize() const;

    // Returns the node with the given id, or nullptr if not found.
    sk_sp<SkSVGNode>* findNodeById(const char* id);

    void render(SkCanvas*) const;

    /** Render the node with the given id as if it were the only child of the root. */
    void renderNode(SkCanvas*, SkSVGPresentationContext&, const char* id) const;

private:
    SkSVGDOM(sk_sp<SkSVGSVG>,
             sk_sp<SkFontMgr>,
             sk_sp<skresources::ResourceProvider>,
             SkSVGIDMapper&&,
             sk_sp<SkShapers::Factory>);

    const sk_sp<SkSVGSVG>                       fRoot;
    const sk_sp<SkFontMgr>                      fFontMgr;
    const sk_sp<SkShapers::Factory>             fTextShapingFactory;
    const sk_sp<skresources::ResourceProvider>  fResourceProvider;
    const SkSVGIDMapper                         fIDMapper;
    SkSize                                      fContainerSize;
};

#endif // SkSVGDOM_DEFINED
