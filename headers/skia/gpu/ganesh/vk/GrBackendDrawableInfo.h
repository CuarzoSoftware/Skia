/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendDrawableInfo_DEFINED
#define GrBackendDrawableInfo_DEFINED

#include "CZ/skia/gpu/ganesh/GrTypes.h"

#include "CZ/skia/gpu/ganesh/vk/GrVkTypes.h"

// If necessary, this could be pulled into a generic interface, but at this point, we only expect
// it to be used by the Ganesh Vulkan backend.
class SK_API GrBackendDrawableInfo {
public:
    // Creates an invalid backend drawable info.
    GrBackendDrawableInfo() : fIsValid(false) {}

    GrBackendDrawableInfo(const GrVkDrawableInfo& info)
            : fIsValid(true)
            , fBackend(GrBackendApi::kVulkan)
            , fVkInfo(info) {}

    // Returns true if the backend texture has been initialized.
    bool isValid() const { return fIsValid; }

    GrBackendApi backend() const { return fBackend; }

    bool getVkDrawableInfo(GrVkDrawableInfo* outInfo) const {
        if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
            *outInfo = fVkInfo;
            return true;
        }
        return false;
    }

private:
    bool             fIsValid;
    GrBackendApi     fBackend;
    GrVkDrawableInfo fVkInfo;
};

#endif
