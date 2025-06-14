/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_hardcoded_DEFINED
#define SkUnicode_hardcoded_DEFINED

#include "cz/skia/core/SkTypes.h"
#include "cz/skia/modules/skunicode/include/SkUnicode.h"
#include "cz/skia/src/base/SkUTF.h"

class SKUNICODE_API SkUnicodeHardCodedCharProperties : public SkUnicode {
public:
    bool isControl(SkUnichar utf8) override;
    bool isWhitespace(SkUnichar utf8) override;
    bool isSpace(SkUnichar utf8) override;
    bool isTabulation(SkUnichar utf8) override;
    bool isHardBreak(SkUnichar utf8) override;
    bool isEmoji(SkUnichar utf8) override;
    bool isEmojiComponent(SkUnichar utf8) override;
    bool isEmojiModifierBase(SkUnichar utf8) override;
    bool isEmojiModifier(SkUnichar utf8) override;
    bool isRegionalIndicator(SkUnichar utf8) override;
    bool isIdeographic(SkUnichar utf8) override;
};

#endif // SkUnicode_hardcoded_DEFINED
