/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMD5_DEFINED
#define SkMD5_DEFINED

#include "CZ/skia/core/SkStream.h"
#include "CZ/skia/core/SkString.h"
#include "CZ/skia/private/base/SkTo.h"

#include <cstdint>
#include <cstring>

/* Calculate a 128-bit MD5 message-digest of the bytes sent to this stream. */
class SkMD5 : public SkWStream {
public:
    SkMD5();

    /** Processes input, adding it to the digest.
        Calling this after finish is undefined.  */
    bool write(const void* buffer, size_t size) final;

    size_t bytesWritten() const final { return SkToSizeT(this->byteCount); }

    struct Digest {
        SkString toHexString() const;
        SkString toLowercaseHexString() const;
        bool operator==(Digest const& other) const {
            return 0 == memcmp(data, other.data, sizeof(data));
        }
        bool operator!=(Digest const& other) const {
            return !(*this == other);
        }

        uint8_t data[16];
    };

    /** Computes and returns the digest. */
    Digest finish();

private:
    uint64_t byteCount;  // number of bytes, modulo 2^64
    uint32_t state[4];   // state (ABCD)
    uint8_t buffer[64];  // input buffer
};

#endif
