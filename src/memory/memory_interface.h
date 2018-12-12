/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * A basic interface to a basic memory manager.
 *
 * Note that the memory allocated by this interface object doesn't get released
 * when the object goes out of scope. It'll remain until program exit, unless
 * specifically freed before then.
 *
 */

#ifndef HEAP_MEM_H
#define HEAP_MEM_H

#include "../../src/display.h"
#include "../../src/common.h"
#include "../../src/memory.h"
#include "../../src/types.h"

/*
 * TODOS:
 *
 * - syntax gets a bit too cumbersome in more involved use.
 *
 */

// A basic interface for a custom memory manager.
template <typename T>
struct heap_bytes_s
{
    heap_bytes_s(void)
    {
        data = nullptr;
    }

    heap_bytes_s(const uint size, const char *const reason = nullptr)
    {
        alloc(size, reason);

        return;
    }

    T& operator[](const uint offs) const
    {
        k_assert_optional((data != nullptr), "Tried to access null heap memory.");
        k_assert_optional((offs < this->num_bytes()), "Accessing heap memory out of bounds with [].");

        return this->data[offs];
    }

    // Raw pointer to the beginning of the data. For when you want to operate on the
    // raw data as an array.
    //
    T* ptr(void) const
    {
        k_assert_optional(data != nullptr, "Tried to access null heap memory.");

        return this->data;
    }

    // For e.g. memset() and the like; run the length value through this function
    // to verify that it's not out of bounds.
    //
    uint up_to(const uint size)
    {
        k_assert((size <= this->num_bytes()), "Possible memory access out of bounds.");

        return size;
    }

    // Manually assign the data pointer to memory, without allocating it.
    //
    void point_to(T *const newPtr, const int numElements)
    {
        k_assert((newPtr != nullptr), "Assigning a null pointer.");
        k_assert((data == nullptr), "Can't assign a pointer to a non-null memory object.");
        k_assert((numElements > 0), "Can't assign with data sizes less than 1.");

        this->data = newPtr;
        this->numElements = numElements;
        this->alias = true;

        return;
    }

    void alloc(const int numElements, const char *const reason = nullptr)
    {
        this->data = (T*)kmem_allocate((sizeof(T) * numElements), (reason == nullptr? "(No reason given.)" : reason));
        this->numElements = numElements;

        return;
    }

    void release_memory(void)
    {
        if (data == nullptr)
        {
            DEBUG(("Was called to release a null memory object. Ignoring this."));
            return;
        }

        if (!alias)
        {
            kmem_release((void**)&this->data);
            this->numElements = 0;
        }
        else
        {
            DEBUG(("Marking aliased memory to null instead of releasing it."));
            this->data = nullptr;
            this->numElements = 0;
        }

        return;
    }

    uint num_bytes(void) const
    {
        return (this->numElements * sizeof(T));
    }

    bool is_null(void) const
    {
        return !bool(this->data);
    }

private:
    T *data = nullptr;
    uint numElements = 0;
    bool alias = false; // Set to true if the data pointer was assigned from outside rather than allocated specifically for this memory object.
};

#endif
