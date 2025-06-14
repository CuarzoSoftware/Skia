// Copyright 2022 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SkContainers_DEFINED
#define SkContainers_DEFINED

#include "CZ/skia/private/base/SkAPI.h"
#include "CZ/skia/private/base/SkAlign.h"
#include "CZ/skia/private/base/SkSpan_impl.h"

#include <cstddef>
#include <cstdint>

class SK_SPI SkContainerAllocator {
public:
    SkContainerAllocator(size_t sizeOfT, int maxCapacity)
            : fSizeOfT{sizeOfT}
            , fMaxCapacity{maxCapacity} {}

    // allocate will abort on failure. Given a capacity of 0, it will return the empty span.
    // The bytes allocated are freed using sk_free().
    SkSpan<std::byte> allocate(int capacity, double growthFactor = 1.0);

    // Rounds a requested capacity up towards `kCapacityMultiple` in a constexpr-friendly fashion.
    template <typename T>
    static constexpr size_t RoundUp(size_t capacity) {
        return SkAlignTo(capacity * sizeof(T), kCapacityMultiple) / sizeof(T);
    }

private:
    friend struct SkContainerAllocatorTestingPeer;

    // All capacity counts will be rounded up to kCapacityMultiple. This matches ASAN's shadow
    // granularity, as well as our typical struct alignment on a 64-bit machine.
    static constexpr int64_t kCapacityMultiple = 8;

    // Rounds up capacity to next multiple of kCapacityMultiple and pin to fMaxCapacity.
    size_t roundUpCapacity(int64_t capacity) const;

    // Grows the capacity by growthFactor being sure to stay with in kMinBytes and fMaxCapacity.
    size_t growthFactorCapacity(int capacity, double growthFactor) const;

    const size_t fSizeOfT;
    const int64_t fMaxCapacity;
};

// sk_allocate_canfail returns the empty span on failure. Parameter size must be > 0.
SkSpan<std::byte> sk_allocate_canfail(size_t size);

// Returns the empty span if size is 0. sk_allocate_throw aborts on failure.
SkSpan<std::byte> sk_allocate_throw(size_t size);

SK_SPI void sk_report_container_overflow_and_die();
#endif  // SkContainers_DEFINED
