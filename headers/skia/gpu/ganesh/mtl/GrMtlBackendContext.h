/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlBackendContext_DEFINED
#define GrMtlBackendContext_DEFINED

#include "CZ/skia/gpu/ganesh/mtl/GrMtlTypes.h"
#include "CZ/skia/ports/SkCFObject.h"

// The BackendContext contains all of the base Metal objects needed by the GrMtlGpu. The assumption
// is that the client will set these up and pass them to the GrMtlGpu constructor.
struct SK_API GrMtlBackendContext {
    sk_cfp<GrMTLHandle> fDevice;
    sk_cfp<GrMTLHandle> fQueue;
};

#endif
