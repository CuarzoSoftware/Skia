/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextThreadSafeProxy_DEFINED
#define GrContextThreadSafeProxy_DEFINED

#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/core/SkTypes.h"
#include "CZ/skia/gpu/GpuTypes.h"
#include "CZ/skia/gpu/ganesh/GrContextOptions.h"
#include "CZ/skia/gpu/ganesh/GrTypes.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>

class GrBackendFormat;
class GrCaps;
class GrContextThreadSafeProxyPriv;
class GrSurfaceCharacterization;
class GrThreadSafeCache;
class GrThreadSafePipelineBuilder;
class SkSurfaceProps;
enum SkColorType : int;
enum class SkTextureCompressionType;
struct SkImageInfo;

namespace sktext::gpu { class TextBlobRedrawCoordinator; }

/**
 * Can be used to perform actions related to the generating GrContext in a thread safe manner. The
 * proxy does not access the 3D API (e.g. OpenGL) that backs the generating GrContext.
 */
class SK_API GrContextThreadSafeProxy : public SkNVRefCnt<GrContextThreadSafeProxy> {
public:
    virtual ~GrContextThreadSafeProxy();

    /**
     *  Create a surface characterization for a DDL that will be replayed into the GrContext
     *  that created this proxy. On failure the resulting characterization will be invalid (i.e.,
     *  "!c.isValid()").
     *
     *  @param cacheMaxResourceBytes           The max resource bytes limit that will be in effect
     *                                         when the DDL created with this characterization is
     *                                         replayed.
     *                                         Note: the contract here is that the DDL will be
     *                                         created as if it had a full 'cacheMaxResourceBytes'
     *                                         to use. If replayed into a GrContext that already has
     *                                         locked GPU memory, the replay can exceed the budget.
     *                                         To rephrase, all resource allocation decisions are
     *                                         made at record time and at playback time the budget
     *                                         limits will be ignored.
     *  @param ii                              The image info specifying properties of the SkSurface
     *                                         that the DDL created with this characterization will
     *                                         be replayed into.
     *                                         Note: Ganesh doesn't make use of the SkImageInfo's
     *                                         alphaType
     *  @param backendFormat                   Information about the format of the GPU surface that
     *                                         will back the SkSurface upon replay
     *  @param sampleCount                     The sample count of the SkSurface that the DDL
     *                                         created with this characterization will be replayed
     *                                         into
     *  @param origin                          The origin of the SkSurface that the DDL created with
     *                                         this characterization will be replayed into
     *  @param surfaceProps                    The surface properties of the SkSurface that the DDL
     *                                         created with this characterization will be replayed
     *                                         into
     *  @param isMipmapped                     Will the surface the DDL will be replayed into have
     *                                         space allocated for mipmaps?
     *  @param willUseGLFBO0                   Will the surface the DDL will be replayed into be
     *                                         backed by GL FBO 0. This flag is only valid if using
     *                                         an GL backend.
     *  @param isTextureable                   Will the surface be able to act as a texture?
     *  @param isProtected                     Will the (Vulkan) surface be DRM protected?
     *  @param vkRTSupportsInputAttachment     Can the vulkan surface be used as in input
                                               attachment?
     *  @param forVulkanSecondaryCommandBuffer Will the surface be wrapping a vulkan secondary
     *                                         command buffer via a GrVkSecondaryCBDrawContext? If
     *                                         this is true then the following is required:
     *                                         isTexureable = false
     *                                         isMipmapped = false
     *                                         willUseGLFBO0 = false
     *                                         vkRTSupportsInputAttachment = false
     */
    GrSurfaceCharacterization createCharacterization(
            size_t cacheMaxResourceBytes,
            const SkImageInfo& ii,
            const GrBackendFormat& backendFormat,
            int sampleCount,
            GrSurfaceOrigin origin,
            const SkSurfaceProps& surfaceProps,
            skgpu::Mipmapped isMipmapped,
            bool willUseGLFBO0 = false,
            bool isTextureable = true,
            skgpu::Protected isProtected = GrProtected::kNo,
            bool vkRTSupportsInputAttachment = false,
            bool forVulkanSecondaryCommandBuffer = false);

    /*
     * Retrieve the default GrBackendFormat for a given SkColorType and renderability.
     * It is guaranteed that this backend format will be the one used by the following
     * SkColorType and GrSurfaceCharacterization-based createBackendTexture methods.
     *
     * The caller should check that the returned format is valid.
     */
    GrBackendFormat defaultBackendFormat(SkColorType ct, GrRenderable renderable) const;

    /**
     * Retrieve the GrBackendFormat for a given SkTextureCompressionType. This is
     * guaranteed to match the backend format used by the following
     * createCompressedBackendTexture methods that take a CompressionType.
     *
     * The caller should check that the returned format is valid.
     */
    GrBackendFormat compressedBackendFormat(SkTextureCompressionType c) const;

    /**
     * Gets the maximum supported sample count for a color type. 1 is returned if only non-MSAA
     * rendering is supported for the color type. 0 is returned if rendering to this color type
     * is not supported at all.
     */
    int maxSurfaceSampleCountForColorType(SkColorType colorType) const;

    bool isValid() const { return nullptr != fCaps; }

    bool operator==(const GrContextThreadSafeProxy& that) const {
        // Each GrContext should only ever have a single thread-safe proxy.
        SkASSERT((this == &that) == (this->fContextID == that.fContextID));
        return this == &that;
    }

    bool operator!=(const GrContextThreadSafeProxy& that) const { return !(*this == that); }

    // Provides access to functions that aren't part of the public API.
    GrContextThreadSafeProxyPriv priv();
    const GrContextThreadSafeProxyPriv priv() const;  // NOLINT(readability-const-return-type)

protected:
    // DDL TODO: need to add unit tests for backend & maybe options
    GrContextThreadSafeProxy(GrBackendApi, const GrContextOptions&);

private:
    friend class GrContextThreadSafeProxyPriv;  // for ctor and hidden methods

    void abandonContext();
    bool abandoned() const;

    // TODO: This should be part of the constructor but right now we have a chicken-and-egg problem
    // with GrContext where we get the caps by creating a GPU which requires a context (see the
    // `init` method on GrContext_Base).
    void init(sk_sp<const GrCaps>, sk_sp<GrThreadSafePipelineBuilder>);

    virtual bool isValidCharacterizationForVulkan(sk_sp<const GrCaps>,
                                                  bool isTextureable,
                                                  skgpu::Mipmapped isMipmapped,
                                                  skgpu::Protected isProtected,
                                                  bool vkRTSupportsInputAttachment,
                                                  bool forVulkanSecondaryCommandBuffer);

    const GrBackendApi                                      fBackend;
    const GrContextOptions                                  fOptions;
    const uint32_t                                          fContextID;
    sk_sp<const GrCaps>                                     fCaps;
    std::unique_ptr<sktext::gpu::TextBlobRedrawCoordinator> fTextBlobRedrawCoordinator;
    std::unique_ptr<GrThreadSafeCache>                      fThreadSafeCache;
    sk_sp<GrThreadSafePipelineBuilder>                      fPipelineBuilder;
    std::atomic<bool>                                       fAbandoned{false};
};

#endif
