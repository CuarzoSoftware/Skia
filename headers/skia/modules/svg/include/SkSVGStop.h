/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGStop_DEFINED
#define SkSVGStop_DEFINED

#include "CZ/skia/core/SkRefCnt.h"
#include "CZ/skia/private/base/SkAPI.h"
#include "CZ/skia/modules/svg/include/SkSVGHiddenContainer.h"
#include "CZ/skia/modules/svg/include/SkSVGNode.h"
#include "CZ/skia/modules/svg/include/SkSVGTypes.h"

class SK_API SkSVGStop : public SkSVGHiddenContainer {
public:
    static constexpr SkSVGTag tag = SkSVGTag::kStop;

    static sk_sp<SkSVGStop> Make() {
        return sk_sp<SkSVGStop>(new SkSVGStop());
    }

    SVG_ATTR(Offset, SkSVGLength, SkSVGLength(0, SkSVGLength::Unit::kPercentage))

protected:
    bool parseAndSetAttribute(const char*, const char*) override;

private:
    SkSVGStop();

    using INHERITED = SkSVGHiddenContainer;
};

#endif // SkSVGStop_DEFINED
