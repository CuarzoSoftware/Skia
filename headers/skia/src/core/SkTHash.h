/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTHash_DEFINED
#define SkTHash_DEFINED

#include "CZ/skia/core/SkTypes.h"
#include "CZ/skia/src/base/SkMathPriv.h"
#include "CZ/skia/src/core/SkChecksum.h"

#include <initializer_list>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace skia_private {

// Before trying to use THashTable, look below to see if THashMap or THashSet works for you.
// They're easier to use, usually perform the same, and have fewer sharp edges.

// T and K are treated as ordinary copyable C++ types.
// Traits must have:
//   - static K GetKey(T)
//   - static uint32_t Hash(K)
// Traits may also define (both required if either is defined):
//   - static bool ShouldGrow(int count, int capacity)
//   - static bool ShouldShrink(int count, int capacity)
// , which specify the max/min load factor of the table.
// If the key is large and stored inside T, you may want to make K a const&.
// Similarly, if T is large you might want it to be a pointer.
template <typename T, typename K, typename Traits = T>
class THashTable {
public:
    THashTable()  = default;
    ~THashTable() = default;

    THashTable(const THashTable&  that) { *this = that; }
    THashTable(      THashTable&& that) { *this = std::move(that); }

    THashTable& operator=(const THashTable& that) {
        if (this != &that) {
            fCount     = that.fCount;
            fCapacity  = that.fCapacity;
            fSlots.reset(new Slot[that.fCapacity]);
            for (int i = 0; i < fCapacity; i++) {
                fSlots[i] = that.fSlots[i];
            }
        }
        return *this;
    }

    THashTable& operator=(THashTable&& that) {
        if (this != &that) {
            fCount    = that.fCount;
            fCapacity = that.fCapacity;
            fSlots    = std::move(that.fSlots);

            that.fCount = that.fCapacity = 0;
        }
        return *this;
    }

    // Clear the table.
    void reset() { *this = THashTable(); }

    // How many entries are in the table?
    int count() const { return fCount; }

    // How many slots does the table contain? (Note that unlike an array, hash tables can grow
    // before reaching 100% capacity.)
    int capacity() const { return fCapacity; }

    // Approximately how many bytes of memory do we use beyond sizeof(*this)?
    size_t approxBytesUsed() const { return fCapacity * sizeof(Slot); }

    // Exchange two hash tables.
    void swap(THashTable& that) {
        std::swap(fCount, that.fCount);
        std::swap(fCapacity, that.fCapacity);
        std::swap(fSlots, that.fSlots);
    }

    void swap(THashTable&& that) {
        *this = std::move(that);
    }

    // !!!!!!!!!!!!!!!!!                 CAUTION                   !!!!!!!!!!!!!!!!!
    // set(), find() and foreach() all allow mutable access to table entries.
    // If you change an entry so that it no longer has the same key, all hell
    // will break loose.  Do not do that!
    //
    // Please prefer to use THashMap or THashSet, which do not have this danger.

    // The pointers returned by set() and find() are valid only until the next call to set().
    // The pointers you receive in foreach() are only valid for its duration.

    // Copy val into the hash table, returning a pointer to the copy now in the table.
    // If there already is an entry in the table with the same key, we overwrite it.
    T* set(T val) {
        bool shouldGrow = false;
        if constexpr (HasShouldGrow<Traits>::value) {
            shouldGrow = Traits::ShouldGrow(fCount, fCapacity);
        } else {
            shouldGrow = (4 * fCount >= 3 * fCapacity);
        }
        if (shouldGrow) {
            this->resize(fCapacity > 0 ? fCapacity * 2 : 4);
        }
        return this->uncheckedSet(std::move(val));
    }

    // If there is an entry in the table with this key, return a pointer to it.  If not, null.
    T* find(const K& key) const {
        uint32_t hash = Hash(key);
        int index = hash & (fCapacity-1);
        for (int n = 0; n < fCapacity; n++) {
            Slot& s = fSlots[index];
            if (s.empty()) {
                return nullptr;
            }
            if (hash == s.fHash && key == Traits::GetKey(*s)) {
                return &*s;
            }
            index = this->next(index);
        }
        SkASSERT(fCapacity == fCount);
        return nullptr;
    }

    // If there is an entry in the table with this key, return it.  If not, null.
    // This only works for pointer type T, and cannot be used to find an nullptr entry.
    T findOrNull(const K& key) const {
        if (T* p = this->find(key)) {
            return *p;
        }
        return nullptr;
    }

    // If a value with this key exists in the hash table, removes it and returns true.
    // Otherwise, returns false.
    bool removeIfExists(const K& key) {
        uint32_t hash = Hash(key);
        int index = hash & (fCapacity-1);
        for (int n = 0; n < fCapacity; n++) {
            Slot& s = fSlots[index];
            if (s.empty()) {
                return false;
            }
            if (hash == s.fHash && key == Traits::GetKey(*s)) {
                this->removeSlot(index);
                if (fCapacity > 4) {
                    bool shouldShrink = false;
                    if constexpr (HasShouldShrink<Traits>::value) {
                        shouldShrink = Traits::ShouldShrink(fCount, fCapacity);
                    } else {
                        shouldShrink = (4 * fCount <= fCapacity);
                    }
                    if (shouldShrink) {
                        this->resize(fCapacity / 2);
                    }
                }
                return true;
            }
            index = this->next(index);
        }
        SkASSERT(fCapacity == fCount);
        return false;
    }

    // Removes the value with this key from the hash table. Asserts if it is missing.
    void remove(const K& key) {
        SkAssertResult(this->removeIfExists(key));
    }

    // Hash tables will automatically resize themselves when set() and remove() are called, but
    // resize() can be called to manually grow capacity before a bulk insertion.
    void resize(int capacity) {
        // We must have enough capacity to hold every key.
        SkASSERT(capacity >= fCount);
        // `capacity` must be a power of two, because we use `hash & (capacity-1)` to look up keys
        // in the table (since this is faster than a modulo).
        SkASSERT((capacity & (capacity - 1)) == 0);

        int oldCapacity = fCapacity;
        SkDEBUGCODE(int oldCount = fCount);

        fCount = 0;
        fCapacity = capacity;
        std::unique_ptr<Slot[]> oldSlots = std::move(fSlots);
        fSlots.reset(new Slot[capacity]);

        for (int i = 0; i < oldCapacity; i++) {
            Slot& s = oldSlots[i];
            if (s.has_value()) {
                this->uncheckedSet(*std::move(s));
            }
        }
        SkASSERT(fCount == oldCount);
    }

    // Reserve extra capacity. This only grows capacity; requests to shrink are ignored.
    // We assume that the passed-in value represents the number of items that the caller wants to
    // store in the table. The passed-in value is adjusted to honor the following rules:
    // - Hash tables must have a power-of-two capacity.
    // - Hash tables grow when they exceed 3/4 capacity, not when they are full.
    void reserve(int n) {
        int newCapacity = SkNextPow2(n);

        bool shouldGrow = false;
        if constexpr (HasShouldGrow<Traits>::value) {
            shouldGrow = Traits::ShouldGrow(n, newCapacity);
        } else {
            shouldGrow = (n * 4 > newCapacity * 3);
        }
        if (shouldGrow) {
            newCapacity *= 2;
        }

        if (newCapacity > fCapacity) {
            this->resize(newCapacity);
        }
    }

    // Call fn on every entry in the table.  You may mutate the entries, but be very careful.
    template <typename Fn>  // f(T*)
    void foreach(Fn&& fn) {
        for (int i = 0; i < fCapacity; i++) {
            if (fSlots[i].has_value()) {
                fn(&*fSlots[i]);
            }
        }
    }

    // Call fn on every entry in the table.  You may not mutate anything.
    template <typename Fn>  // f(T) or f(const T&)
    void foreach(Fn&& fn) const {
        for (int i = 0; i < fCapacity; i++) {
            if (fSlots[i].has_value()) {
                fn(*fSlots[i]);
            }
        }
    }

    // A basic iterator-like class which disallows mutation; sufficient for range-based for loops.
    // Intended for use by THashMap and THashSet via begin() and end().
    // Adding or removing elements may invalidate all iterators.
    template <typename SlotVal>
    class Iter {
    public:
        using TTable = THashTable<T, K, Traits>;

        Iter(const TTable* table, int slot) : fTable(table), fSlot(slot) {}

        static Iter MakeBegin(const TTable* table) {
            return Iter{table, table->firstPopulatedSlot()};
        }

        static Iter MakeEnd(const TTable* table) {
            return Iter{table, table->capacity()};
        }

        const SlotVal& operator*() const {
            return *fTable->slot(fSlot);
        }

        const SlotVal* operator->() const {
            return fTable->slot(fSlot);
        }

        bool operator==(const Iter& that) const {
            // Iterators from different tables shouldn't be compared against each other.
            SkASSERT(fTable == that.fTable);
            return fSlot == that.fSlot;
        }

        bool operator!=(const Iter& that) const {
            return !(*this == that);
        }

        Iter& operator++() {
            fSlot = fTable->nextPopulatedSlot(fSlot);
            return *this;
        }

        Iter operator++(int) {
            Iter old = *this;
            this->operator++();
            return old;
        }

    protected:
        const TTable* fTable;
        int fSlot;
    };

private:
    template <typename U, typename = void> struct HasShouldGrow : std::false_type {};
    template <typename U, typename = void> struct HasShouldShrink : std::false_type {};

    template <typename U>
    struct HasShouldGrow<
            U,
            std::void_t<decltype(U::ShouldGrow(std::declval<int>(), std::declval<int>()))>>
            : std::true_type {
        static_assert(HasShouldShrink<U>::value,
                      "The traits class must also provide ShouldShrink() method.");
    };

    template <typename U>
    struct HasShouldShrink<
            U,
            std::void_t<decltype(U::ShouldShrink(std::declval<int>(), std::declval<int>()))>>
            : std::true_type {
        static_assert(HasShouldGrow<U>::value,
                      "The traits class must also provide ShouldGrow() method.");
    };

    // Finds the first non-empty slot for an iterator.
    int firstPopulatedSlot() const {
        for (int i = 0; i < fCapacity; i++) {
            if (fSlots[i].has_value()) {
                return i;
            }
        }
        return fCapacity;
    }

    // Increments an iterator's slot.
    int nextPopulatedSlot(int currentSlot) const {
        for (int i = currentSlot + 1; i < fCapacity; i++) {
            if (fSlots[i].has_value()) {
                return i;
            }
        }
        return fCapacity;
    }

    // Reads from an iterator's slot.
    const T* slot(int i) const {
        SkASSERT(fSlots[i].has_value());
        return &*fSlots[i];
    }

    T* uncheckedSet(T&& val) {
        const K& key = Traits::GetKey(val);
        SkASSERT(key == key);
        uint32_t hash = Hash(key);
        int index = hash & (fCapacity-1);
        for (int n = 0; n < fCapacity; n++) {
            Slot& s = fSlots[index];
            if (s.empty()) {
                // New entry.
                s.emplace(std::move(val), hash);
                fCount++;
                return &*s;
            }
            if (hash == s.fHash && key == Traits::GetKey(*s)) {
                // Overwrite previous entry.
                // Note: this triggers extra copies when adding the same value repeatedly.
                s.emplace(std::move(val), hash);
                return &*s;
            }

            index = this->next(index);
        }
        SkASSERT(false);
        return nullptr;
    }

    void removeSlot(int index) {
        fCount--;

        // Rearrange elements to restore the invariants for linear probing.
        for (;;) {
            Slot& emptySlot = fSlots[index];
            int emptyIndex = index;
            int originalIndex;
            // Look for an element that can be moved into the empty slot.
            // If the empty slot is in between where an element landed, and its native slot, then
            // move it to the empty slot. Don't move it if its native slot is in between where
            // the element landed and the empty slot.
            // [native] <= [empty] < [candidate] == GOOD, can move candidate to empty slot
            // [empty] < [native] < [candidate] == BAD, need to leave candidate where it is
            do {
                index = this->next(index);
                Slot& s = fSlots[index];
                if (s.empty()) {
                    // We're done shuffling elements around.  Clear the last empty slot.
                    emptySlot.reset();
                    return;
                }
                originalIndex = s.fHash & (fCapacity - 1);
            } while ((index <= originalIndex && originalIndex < emptyIndex)
                     || (originalIndex < emptyIndex && emptyIndex < index)
                     || (emptyIndex < index && index <= originalIndex));
            // Move the element to the empty slot.
            Slot& moveFrom = fSlots[index];
            emptySlot = std::move(moveFrom);
        }
    }

    int next(int index) const {
        index--;
        if (index < 0) { index += fCapacity; }
        return index;
    }

    static uint32_t Hash(const K& key) {
        uint32_t hash = Traits::Hash(key) & 0xffffffff;
        return hash ? hash : 1;  // We reserve hash 0 to mark empty.
    }

    class Slot {
    public:
        Slot() = default;
        ~Slot() { this->reset(); }

        Slot(const Slot& that) { *this = that; }
        Slot& operator=(const Slot& that) {
            if (this == &that) {
                return *this;
            }
            if (fHash) {
                if (that.fHash) {
                    fVal.fStorage = that.fVal.fStorage;
                    fHash = that.fHash;
                } else {
                    this->reset();
                }
            } else {
                if (that.fHash) {
                    new (&fVal.fStorage) T(that.fVal.fStorage);
                    fHash = that.fHash;
                } else {
                    // do nothing, no value on either side
                }
            }
            return *this;
        }

        Slot(Slot&& that) { *this = std::move(that); }
        Slot& operator=(Slot&& that) {
            if (this == &that) {
                return *this;
            }
            if (fHash) {
                if (that.fHash) {
                    fVal.fStorage = std::move(that.fVal.fStorage);
                    fHash = that.fHash;
                } else {
                    this->reset();
                }
            } else {
                if (that.fHash) {
                    new (&fVal.fStorage) T(std::move(that.fVal.fStorage));
                    fHash = that.fHash;
                } else {
                    // do nothing, no value on either side
                }
            }
            return *this;
        }

        T& operator*() & { return fVal.fStorage; }
        const T& operator*() const& { return fVal.fStorage; }
        T&& operator*() && { return std::move(fVal.fStorage); }
        const T&& operator*() const&& { return std::move(fVal.fStorage); }

        Slot& emplace(T&& v, uint32_t h) {
            this->reset();
            new (&fVal.fStorage) T(std::move(v));
            fHash = h;
            return *this;
        }

        bool has_value() const { return fHash != 0; }
        explicit operator bool() const { return this->has_value(); }
        bool empty() const { return !this->has_value(); }

        void reset() {
            if (fHash) {
                fVal.fStorage.~T();
                fHash = 0;
            }
        }

        uint32_t fHash = 0;

    private:
        union Storage {
            T fStorage;
            Storage() {}
            ~Storage() {}
        } fVal;
    };

    int fCount    = 0,
        fCapacity = 0;
    std::unique_ptr<Slot[]> fSlots;
};

// Maps K->V.  A more user-friendly wrapper around THashTable, suitable for most use cases.
// K and V are treated as ordinary copyable C++ types, with no assumed relationship between the two.
template <typename K, typename V, typename HashK = SkGoodHash>
class THashMap {
public:
    // Allow default construction and assignment.
    THashMap() = default;

    THashMap(THashMap<K, V, HashK>&& that) = default;
    THashMap(const THashMap<K, V, HashK>& that) = default;

    THashMap<K, V, HashK>& operator=(THashMap<K, V, HashK>&& that) = default;
    THashMap<K, V, HashK>& operator=(const THashMap<K, V, HashK>& that) = default;

    // Construct with an initializer list of key-value pairs.
    struct Pair : public std::pair<K, V> {
        using std::pair<K, V>::pair;
        static const K& GetKey(const Pair& p) { return p.first; }
        static auto Hash(const K& key) { return HashK()(key); }
    };

    THashMap(std::initializer_list<Pair> pairs) {
        int capacity = pairs.size() >= 4 ? SkNextPow2(pairs.size() * 4 / 3)
                                         : 4;
        fTable.resize(capacity);
        for (const Pair& p : pairs) {
            fTable.set(p);
        }
    }

    // Clear the map.
    void reset() { fTable.reset(); }

    // How many key/value pairs are in the table?
    int count() const { return fTable.count(); }

    // Is empty?
    bool empty() const { return fTable.count() == 0; }

    // Approximately how many bytes of memory do we use beyond sizeof(*this)?
    size_t approxBytesUsed() const { return fTable.approxBytesUsed(); }

    // Reserve extra capacity.
    void reserve(int n) { fTable.reserve(n); }

    // Exchange two hash maps.
    void swap(THashMap& that) { fTable.swap(that.fTable); }
    void swap(THashMap&& that) { fTable.swap(std::move(that.fTable)); }

    // N.B. The pointers returned by set() and find() are valid only until the next call to set().

    // Set key to val in the table, replacing any previous value with the same key.
    // We copy both key and val, and return a pointer to the value copy now in the table.
    V* set(K key, V val) {
        Pair* out = fTable.set({std::move(key), std::move(val)});
        return &out->second;
    }

    // If there is key/value entry in the table with this key, return a pointer to the value.
    // If not, return null.
    V* find(const K& key) const {
        if (Pair* p = fTable.find(key)) {
            return &p->second;
        }
        return nullptr;
    }

    V& operator[](const K& key) {
        if (V* val = this->find(key)) {
            return *val;
        }
        return *this->set(key, V{});
    }

    // Removes the key/value entry in the table with this key. Asserts if the key is not present.
    void remove(const K& key) {
        fTable.remove(key);
    }

    // If the key exists in the table, removes it and returns true. Otherwise, returns false.
    bool removeIfExists(const K& key) {
        return fTable.removeIfExists(key);
    }

    // Call fn on every key/value pair in the table.  You may mutate the value but not the key.
    template <typename Fn,  // f(K, V*) or f(const K&, V*)
              std::enable_if_t<std::is_invocable_v<Fn, K, V*>>* = nullptr>
    void foreach(Fn&& fn) {
        fTable.foreach([&fn](Pair* p) { fn(p->first, &p->second); });
    }

    // Call fn on every key/value pair in the table.  You may not mutate anything.
    template <typename Fn,  // f(K, V), f(const K&, V), f(K, const V&) or f(const K&, const V&).
              std::enable_if_t<std::is_invocable_v<Fn, K, V>>* = nullptr>
    void foreach(Fn&& fn) const {
        fTable.foreach([&fn](const Pair& p) { fn(p.first, p.second); });
    }

    // Call fn on every key/value pair in the table.  You may not mutate anything.
    template <typename Fn,  // f(Pair), or f(const Pair&)
              std::enable_if_t<std::is_invocable_v<Fn, Pair>>* = nullptr>
    void foreach(Fn&& fn) const {
        fTable.foreach([&fn](const Pair& p) { fn(p); });
    }

    // Dereferencing an iterator gives back a key-value pair, suitable for structured binding.
    using Iter = typename THashTable<Pair, K>::template Iter<std::pair<K, V>>;

    Iter begin() const {
        return Iter::MakeBegin(&fTable);
    }

    Iter end() const {
        return Iter::MakeEnd(&fTable);
    }

private:
    THashTable<Pair, K> fTable;
};

// A set of T.  T is treated as an ordinary copyable C++ type.
template <typename T, typename HashT = SkGoodHash>
class THashSet {
public:
    // Allow default construction and assignment.
    THashSet() = default;

    THashSet(THashSet<T, HashT>&& that) = default;
    THashSet(const THashSet<T, HashT>& that) = default;

    THashSet<T, HashT>& operator=(THashSet<T, HashT>&& that) = default;
    THashSet<T, HashT>& operator=(const THashSet<T, HashT>& that) = default;

    // Construct with an initializer list of Ts.
    THashSet(std::initializer_list<T> vals) {
        int capacity = vals.size() >= 4 ? SkNextPow2(vals.size() * 4 / 3)
                                        : 4;
        fTable.resize(capacity);
        for (const T& val : vals) {
            fTable.set(val);
        }
    }

    // Clear the set.
    void reset() { fTable.reset(); }

    // How many items are in the set?
    int count() const { return fTable.count(); }

    // Is empty?
    bool empty() const { return fTable.count() == 0; }

    // Approximately how many bytes of memory do we use beyond sizeof(*this)?
    size_t approxBytesUsed() const { return fTable.approxBytesUsed(); }

    // Reserve extra capacity.
    void reserve(int n) { fTable.reserve(n); }

    // Exchange two hash sets.
    void swap(THashSet& that) { fTable.swap(that.fTable); }
    void swap(THashSet&& that) { fTable.swap(std::move(that.fTable)); }

    // Copy an item into the set.
    void add(T item) { fTable.set(std::move(item)); }

    // Is this item in the set?
    bool contains(const T& item) const { return SkToBool(this->find(item)); }

    // If an item equal to this is in the set, return a pointer to it, otherwise null.
    // This pointer remains valid until the next call to add().
    const T* find(const T& item) const { return fTable.find(item); }

    // Remove the item in the set equal to this.
    void remove(const T& item) {
        SkASSERT(this->contains(item));
        fTable.remove(item);
    }

    // Call fn on every item in the set.  You may not mutate anything.
    template <typename Fn>  // f(T), f(const T&)
    void foreach (Fn&& fn) const {
        fTable.foreach(fn);
    }

private:
    struct Traits {
        static const T& GetKey(const T& item) { return item; }
        static auto Hash(const T& item) { return HashT()(item); }
    };

public:
    using Iter = typename THashTable<T, T, Traits>::template Iter<T>;

    Iter begin() const {
        return Iter::MakeBegin(&fTable);
    }

    Iter end() const {
        return Iter::MakeEnd(&fTable);
    }

private:
    THashTable<T, T, Traits> fTable;
};

}  // namespace skia_private

#endif  // SkTHash_DEFINED
