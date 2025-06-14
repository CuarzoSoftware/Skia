/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceCharacterization_DEFINED
#define GrSurfaceCharacterization_DEFINED

#include "CZ/skia/core/SkColorSpace.h" // IWYU pragma: keep
#include "CZ/skia/core/SkColorType.h"
#include "CZ/skia/core/SkImageInfo.h"
#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/core/SkSize.h"
#include "CZ/skia/core/SkSurfaceProps.h"
#include "CZ/skia/core/SkTypes.h"
#include "CZ/skia/gpu/GpuTypes.h"
#include "CZ/skia/gpu/ganesh/GrBackendSurface.h"
#include "CZ/skia/gpu/ganesh/GrContextThreadSafeProxy.h"
#include "CZ/skia/gpu/ganesh/GrTypes.h"
#include "CZ/skia/private/base/SkDebug.h"

#include <cstddef>
#include <utility>

/** \class GrSurfaceCharacterization
    A surface characterization contains all the information Ganesh requires to makes its internal
    rendering decisions. When passed into a GrDeferredDisplayListRecorder it will copy the
    data and pass it on to the GrDeferredDisplayList if/when it is created. Note that both of
    those objects (the Recorder and the DisplayList) will take a ref on the
    GrContextThreadSafeProxy and SkColorSpace objects.
*/
class SK_API GrSurfaceCharacterization {
public:
    enum class Textureable : bool { kNo = false, kYes = true };
    enum class UsesGLFBO0 : bool { kNo = false, kYes = true };
    // This flag indicates that the backing VkImage for this Vulkan surface will have the
    // VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT set. This bit allows skia to handle advanced blends
    // more optimally in a shader by being able to directly read the dst values.
    enum class VkRTSupportsInputAttachment : bool { kNo = false, kYes = true };
    // This flag indicates if the surface is wrapping a raw Vulkan secondary command buffer.
    enum class VulkanSecondaryCBCompatible : bool { kNo = false, kYes = true };

    GrSurfaceCharacterization()
            : fCacheMaxResourceBytes(0)
            , fOrigin(kBottomLeft_GrSurfaceOrigin)
            , fSampleCnt(0)
            , fIsTextureable(Textureable::kYes)
            , fIsMipmapped(skgpu::Mipmapped::kYes)
            , fUsesGLFBO0(UsesGLFBO0::kNo)
            , fVulkanSecondaryCBCompatible(VulkanSecondaryCBCompatible::kNo)
            , fIsProtected(skgpu::Protected::kNo)
            , fSurfaceProps() {}

    GrSurfaceCharacterization(GrSurfaceCharacterization&&) = default;
    GrSurfaceCharacterization& operator=(GrSurfaceCharacterization&&) = default;

    GrSurfaceCharacterization(const GrSurfaceCharacterization&) = default;
    GrSurfaceCharacterization& operator=(const GrSurfaceCharacterization& other) = default;
    bool operator==(const GrSurfaceCharacterization& other) const;
    bool operator!=(const GrSurfaceCharacterization& other) const {
        return !(*this == other);
    }

    /*
     * Return a new surface characterization with the only difference being a different width
     * and height
     */
    GrSurfaceCharacterization createResized(int width, int height) const;

    /*
     * Return a new surface characterization with only a replaced color space
     */
    GrSurfaceCharacterization createColorSpace(sk_sp<SkColorSpace>) const;

    /*
     * Return a new surface characterization with the backend format replaced. A colorType
     * must also be supplied to indicate the interpretation of the new format.
     */
    GrSurfaceCharacterization createBackendFormat(SkColorType colorType,
                                                  const GrBackendFormat& backendFormat) const;

    /*
     * Return a new surface characterization with just a different use of FBO0 (in GL)
     */
    GrSurfaceCharacterization createFBO0(bool usesGLFBO0) const;

    GrContextThreadSafeProxy* contextInfo() const { return fContextInfo.get(); }
    sk_sp<GrContextThreadSafeProxy> refContextInfo() const { return fContextInfo; }
    size_t cacheMaxResourceBytes() const { return fCacheMaxResourceBytes; }

    bool isValid() const { return kUnknown_SkColorType != fImageInfo.colorType(); }

    const SkImageInfo& imageInfo() const { return fImageInfo; }
    const GrBackendFormat& backendFormat() const { return fBackendFormat; }
    GrSurfaceOrigin origin() const { return fOrigin; }
    SkISize dimensions() const { return fImageInfo.dimensions(); }
    int width() const { return fImageInfo.width(); }
    int height() const { return fImageInfo.height(); }
    SkColorType colorType() const { return fImageInfo.colorType(); }
    int sampleCount() const { return fSampleCnt; }
    bool isTextureable() const { return Textureable::kYes == fIsTextureable; }
    bool isMipMapped() const { return skgpu::Mipmapped::kYes == fIsMipmapped; }
    bool usesGLFBO0() const { return UsesGLFBO0::kYes == fUsesGLFBO0; }
    bool vkRTSupportsInputAttachment() const {
        return VkRTSupportsInputAttachment::kYes == fVkRTSupportsInputAttachment;
    }
    bool vulkanSecondaryCBCompatible() const {
        return VulkanSecondaryCBCompatible::kYes == fVulkanSecondaryCBCompatible;
    }
    skgpu::Protected isProtected() const { return fIsProtected; }
    SkColorSpace* colorSpace() const { return fImageInfo.colorSpace(); }
    sk_sp<SkColorSpace> refColorSpace() const { return fImageInfo.refColorSpace(); }
    const SkSurfaceProps& surfaceProps()const { return fSurfaceProps; }

private:
    friend class SkSurface_Ganesh;           // for 'set' & 'config'
    friend class GrVkSecondaryCBDrawContext; // for 'set' & 'config'
    friend class GrContextThreadSafeProxy; // for private ctor
    friend class GrVkContextThreadSafeProxy;    // for private ctor
    friend class GrDeferredDisplayListRecorder; // for 'config'
    friend class SkSurface; // for 'config'

    SkDEBUGCODE(void validate() const;)

            GrSurfaceCharacterization(sk_sp<GrContextThreadSafeProxy> contextInfo,
                                      size_t cacheMaxResourceBytes,
                                      const SkImageInfo& ii,
                                      const GrBackendFormat& backendFormat,
                                      GrSurfaceOrigin origin,
                                      int sampleCnt,
                                      Textureable isTextureable,
                                      skgpu::Mipmapped isMipmapped,
                                      UsesGLFBO0 usesGLFBO0,
                                      VkRTSupportsInputAttachment vkRTSupportsInputAttachment,
                                      VulkanSecondaryCBCompatible vulkanSecondaryCBCompatible,
                                      skgpu::Protected isProtected,
                                      const SkSurfaceProps& surfaceProps)
            : fContextInfo(std::move(contextInfo))
            , fCacheMaxResourceBytes(cacheMaxResourceBytes)
            , fImageInfo(ii)
            , fBackendFormat(std::move(backendFormat))
            , fOrigin(origin)
            , fSampleCnt(sampleCnt)
            , fIsTextureable(isTextureable)
            , fIsMipmapped(isMipmapped)
            , fUsesGLFBO0(usesGLFBO0)
            , fVkRTSupportsInputAttachment(vkRTSupportsInputAttachment)
            , fVulkanSecondaryCBCompatible(vulkanSecondaryCBCompatible)
            , fIsProtected(isProtected)
            , fSurfaceProps(surfaceProps) {
        if (fSurfaceProps.flags() & SkSurfaceProps::kDynamicMSAA_Flag) {
            // Dynamic MSAA is not currently supported with DDL.
            *this = {};
        }
        SkDEBUGCODE(this->validate());
    }

    void set(sk_sp<GrContextThreadSafeProxy> contextInfo,
             size_t cacheMaxResourceBytes,
             const SkImageInfo& ii,
             const GrBackendFormat& backendFormat,
             GrSurfaceOrigin origin,
             int sampleCnt,
             Textureable isTextureable,
             skgpu::Mipmapped isMipmapped,
             UsesGLFBO0 usesGLFBO0,
             VkRTSupportsInputAttachment vkRTSupportsInputAttachment,
             VulkanSecondaryCBCompatible vulkanSecondaryCBCompatible,
             skgpu::Protected isProtected,
             const SkSurfaceProps& surfaceProps) {
        if (surfaceProps.flags() & SkSurfaceProps::kDynamicMSAA_Flag) {
            // Dynamic MSAA is not currently supported with DDL.
            *this = {};
        } else {
            fContextInfo = std::move(contextInfo);
            fCacheMaxResourceBytes = cacheMaxResourceBytes;

            fImageInfo = ii;
            fBackendFormat = std::move(backendFormat);
            fOrigin = origin;
            fSampleCnt = sampleCnt;
            fIsTextureable = isTextureable;
            fIsMipmapped = isMipmapped;
            fUsesGLFBO0 = usesGLFBO0;
            fVkRTSupportsInputAttachment = vkRTSupportsInputAttachment;
            fVulkanSecondaryCBCompatible = vulkanSecondaryCBCompatible;
            fIsProtected = isProtected;
            fSurfaceProps = surfaceProps;
        }
        SkDEBUGCODE(this->validate());
    }

    sk_sp<GrContextThreadSafeProxy> fContextInfo;
    size_t                          fCacheMaxResourceBytes;

    SkImageInfo                     fImageInfo;
    GrBackendFormat                 fBackendFormat;
    GrSurfaceOrigin                 fOrigin;
    int                             fSampleCnt;
    Textureable                     fIsTextureable;
    skgpu::Mipmapped                fIsMipmapped;
    UsesGLFBO0                      fUsesGLFBO0;
    VkRTSupportsInputAttachment     fVkRTSupportsInputAttachment;
    VulkanSecondaryCBCompatible     fVulkanSecondaryCBCompatible;
    skgpu::Protected                fIsProtected;
    SkSurfaceProps                  fSurfaceProps;
};

#endif
