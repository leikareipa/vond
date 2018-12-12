/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * Sketching out an asset management system, where assets (like textures and so on)
 * are wrapped around a container interface with memory management.
 *
 * This system is notably underdeveloped, at the moment, and likely to change in
 * the future.
 *
 */

#ifndef ASSET_STORE_H
#define ASSET_STORE_H

#include <string>
#include "../../src/memory.h"
#include "../../src/types.h"

template <typename T>
struct asset_s
{
    asset_s(void)
    {
        return;
    }

    asset_s(const heap_bytes_s<T> &container) :
        data(container)
    {
        return;
    }

    const T& operator()(const uint offs = 0) const
    {
        k_assert(!this->data.is_null(), "Accessing an uninitialized asset.");

        return data[offs];
    }

private:
    heap_bytes_s<T> data;
};

// Creates an asset wrapper for the given asset data.
//
// Usage:
//      1. Create an object out of what you want made into an asset.
//      2. Call this function with a pointer to the object.
//      3. This will make a heap copy of the object, and return its pointer in an asset wrapper.
//
template <typename T>
asset_s<T> kasset_make_asset(T *const assetPtr)
{
    // Duplicate the asset object onto the heap.
    heap_bytes_s<T> assetContainer(1);
    memcpy(assetContainer.ptr(), assetPtr, assetContainer.up_to(sizeof(T)));

    asset_s<T> asset(assetContainer);
    return asset;
}

#endif
