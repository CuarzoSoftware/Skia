/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMutex_DEFINED
#define SkMutex_DEFINED

#include "CZ/skia/private/base/SkAssert.h"
#include "CZ/skia/private/base/SkDebug.h"
#include "CZ/skia/private/base/SkSemaphore.h"
#include "CZ/skia/private/base/SkThreadAnnotations.h"
#include "CZ/skia/private/base/SkThreadID.h"

/**
 * class SkMutex
 *
 * This allows us to have a mutex without needing the one in
 * the C++ std library which does not work with all clients.
 * go/cstyle#Disallowed_Stdlib
 */

class SK_CAPABILITY("mutex") SkMutex {
public:
    constexpr SkMutex() = default;

    ~SkMutex() {
        this->assertNotHeld();
    }

    void acquire() SK_ACQUIRE() {
        fSemaphore.wait();
        SkDEBUGCODE(fOwner = SkGetThreadID();)
    }

    void release() SK_RELEASE_CAPABILITY() {
        this->assertHeld();
        SkDEBUGCODE(fOwner = kIllegalThreadID;)
        fSemaphore.signal();
    }

    void assertHeld() SK_ASSERT_CAPABILITY(this) {
        SkASSERT(fOwner == SkGetThreadID());
    }

    void assertNotHeld() {
        SkASSERT(fOwner == kIllegalThreadID);
    }

private:
    SkSemaphore fSemaphore{1};
    SkDEBUGCODE(SkThreadID fOwner{kIllegalThreadID};)
};

class SK_SCOPED_CAPABILITY SkAutoMutexExclusive {
public:
    SkAutoMutexExclusive(SkMutex& mutex) SK_ACQUIRE(mutex) : fMutex(mutex) { fMutex.acquire(); }
    ~SkAutoMutexExclusive() SK_RELEASE_CAPABILITY() { fMutex.release(); }

    SkAutoMutexExclusive(const SkAutoMutexExclusive&) = delete;
    SkAutoMutexExclusive(SkAutoMutexExclusive&&) = delete;

    SkAutoMutexExclusive& operator=(const SkAutoMutexExclusive&) = delete;
    SkAutoMutexExclusive& operator=(SkAutoMutexExclusive&&) = delete;

private:
    SkMutex& fMutex;
};

#endif  // SkMutex_DEFINED
