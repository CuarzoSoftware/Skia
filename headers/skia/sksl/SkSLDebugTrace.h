/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DEBUG_TRACE
#define SKSL_DEBUG_TRACE

#include "CZ/skia/core/SkRefCnt.h"

class SkWStream;

namespace SkSL {

class DebugTrace : public SkRefCnt {
public:
    /** Generates a human-readable dump of the debug trace. */
    virtual void dump(SkWStream* o) const = 0;
};

} // namespace SkSL

#endif
