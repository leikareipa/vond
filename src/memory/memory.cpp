/*
 * Tarpeeksi Hyvae Soft /
 * Vond
 *
 * A basic memory manager. Allocates a bunch of memory on initialization, then doles
 * it out to callers in small sequential chunks afterwards. Useful to prevent fragmentation
 * and to see how memory is being used by the program, though a limited implementation.
 *
 */

#include <stdexcept>
#include <thread>
#include <cstdlib>
#include <cstring>
#include "../../src/memory/memory_interface.h"
#include "../../src/memory.h"
#include "../../src/common.h"
#include "../../src/main.h"

/*
 * TODOS:
 *
 * - add dynamic resizing to the memory cache for when it's running short.
 *
 */

// The memory manager isn't prepared for multithreading, so discourage its use in
// that context. We'll grab the id of whatever thread initializes this, and later
// bail if any other thread requests operations of the manager.
static const std::thread::id NATIVE_THREAD_ID = std::this_thread::get_id();

// A single allocation from the memory cache.
struct mem_allocation_s
{
    char reason[64];    // For what purpose the memory was needed.
    const void *memory;
    uint numBytes;
    bool isInUse;       // Set to false if the owner asks the memory manager to release the memory.
    bool isReused;      // Whether this allocation used to belong to someone else who since had it released.
};

static int TOTAL_BYTES_ALLOCATED = 0;
static int TOTAL_BYTES_RELEASED = 0;

static uint MEMORY_CACHE_SIZE = 250/*MB*/ * 1024 * 1024;   // In bytes.
static u8 *MEMORY_CACHE = nullptr;
static u8 *NEXT_FREE = nullptr;    // A pointer to the next free byte in the memory cache (allocations from the cache are made sequentially).

// Each allocation requested is logged into the allocation table.
static const uint NUM_ALLOC_TABLE_ELEMENTS = 2048;
static mem_allocation_s *ALLOC_TABLE = nullptr;

// Set to 1 to disallow any further allocations from the cache.
static uint CACHE_ALLOC_LOCKED = 0;

static void initialize_cache(void)
{
    DEBUG(("Initializing the memory manager..."));

    // Ideally, we'll only call this function once.
    k_assert(MEMORY_CACHE == nullptr, "Calling the memory manager initializer with a non-null memory cache.");

    static_assert(std::is_same<decltype(MEMORY_CACHE), u8*>::value && (sizeof(u8) == 1), "Expected the memory cache to be u8*.");
    MEMORY_CACHE = (u8*)calloc(MEMORY_CACHE_SIZE, 1); // Expect to use calloc(), for a cleared-out block.
    NEXT_FREE = MEMORY_CACHE;
    k_assert(MEMORY_CACHE != nullptr, "The memory manager failed to initialize.");

    // Use the beginning of the memory cache for the allocation table.
    k_assert((ALLOC_TABLE == nullptr), "Expected a null allocation table.");
    const uint allocTableByteSize = (NUM_ALLOC_TABLE_ELEMENTS * sizeof(mem_allocation_s));
    k_assert((allocTableByteSize < MEMORY_CACHE_SIZE), "Not enough room in the memory cache for the allocation table.");
    ALLOC_TABLE = (mem_allocation_s*)MEMORY_CACHE;
    TOTAL_BYTES_ALLOCATED += allocTableByteSize;
    NEXT_FREE += allocTableByteSize;

    return;
}

// Will return a valid pointer to a zero-initialized block of memory of the given
// size; or trips an assert if can't.
//
void* kmem_allocate(const int numBytes, const char *const reason)
{
    k_assert(std::this_thread::get_id() == NATIVE_THREAD_ID, "Potential attempt to allocate memory from multiple threads. This isn't supported.");

    k_assert(!CACHE_ALLOC_LOCKED, "Memory allocations are locked, can't add new ones.");
    k_assert(numBytes > 0, "Can't allocate sub-byte memory blocks.");

    // Initialize the memory cache the first time the allocator is called.
    if (MEMORY_CACHE == nullptr)
    {
        initialize_cache();
    }

    k_assert(NEXT_FREE != nullptr, "Didn't have a valid free slot in the memory cache for a new allocation.");
    u8 *mem = NEXT_FREE;

    // Find a free spot in the allocation table.
    uint tableIdx = 0;
    bool useOldEntry = false;   // If we decided to use a previously-cleared entry.
    while (1)
    {
        // Take the first unallocated entry. These will always be at the end of
        // the allocation chain, since past allocations are never cleared until
        // the program exits.
        if (ALLOC_TABLE[tableIdx].memory == nullptr)
        {
            break;
        }

        // But also, if there's a released entry of just the right size, we can
        // take that one instead.
        if (!ALLOC_TABLE[tableIdx].isInUse &&
            ALLOC_TABLE[tableIdx].numBytes == uint(numBytes))
        {
            useOldEntry = true;
            break;
        }

        tableIdx++;
        k_assert(tableIdx < NUM_ALLOC_TABLE_ELEMENTS,
                 "Memory manager overflowed while looking for room for an allocation.");
    }

    if (useOldEntry)
    {
        mem = (u8*)ALLOC_TABLE[tableIdx].memory;
        ALLOC_TABLE[tableIdx].isReused = true;
    }
    else
    {
        TOTAL_BYTES_ALLOCATED += numBytes;
        NEXT_FREE += numBytes;
    }

    k_assert(strlen(reason) < NUM_ELEMENTS(ALLOC_TABLE[tableIdx].reason),
             "The reason given for an allocation was too long to fit into the string array.");
    strcpy(ALLOC_TABLE[tableIdx].reason, reason);
    ALLOC_TABLE[tableIdx].memory = mem;
    ALLOC_TABLE[tableIdx].numBytes = numBytes;
    ALLOC_TABLE[tableIdx].isInUse = true;

    // Validate the memory before sending it off.
    k_assert((mem + numBytes) < (MEMORY_CACHE + MEMORY_CACHE_SIZE),
             "Memory allocation would overflow the memory cache size. The cache needs to be made larger.");
    k_assert(mem != nullptr, "Detected a null return from the memory manager allocator. This should not be the case.");
    memset(mem, 0, numBytes);

    return (void*)mem;
}

static uint alloc_table_index_of_pointer(const void *const mem)
{
    uint tableIDx = 0;
    while (ALLOC_TABLE[tableIDx].memory != mem)
    {
        tableIDx++;
        if (tableIDx >= NUM_ALLOC_TABLE_ELEMENTS)
        {
            k_assert(0, "Couldn't find the requested allocation in the allocation table.");
            goto bail;
        }
    }

    return tableIDx;

    bail:
    throw std::runtime_error("Unexpected code path in the memory manager.");    // We should never reach this, but will if asserts are disabled.
    return 0;
}

// Returns the number of bytes allocated for the memory block starting at the given
// address.
uint kmem_sizeof_allocation(const void *const mem)
{
    if (mem == nullptr)
    {
        k_assert(0, "The memory manager was asked for the size of a null allocation.");
        return 0;
    }

    const uint idx = alloc_table_index_of_pointer(mem);

    return ALLOC_TABLE[idx].numBytes;
}

// Mark the given pointer as nullptr. Internally, we still retain its pointer for
// for possible reuse, and will deallocate its memory at the end of the program.
//
void kmem_release(void **mem)
{
    k_assert(std::this_thread::get_id() == NATIVE_THREAD_ID, "Potential attempt to release memory from multiple threads. This isn't supported.");

    if (*mem == nullptr)
    {
        return;
    }

    // Make sure we've been asked to release a valid allocation.
    assert(*mem >= MEMORY_CACHE);
    assert(*mem <= (MEMORY_CACHE + MEMORY_CACHE_SIZE));

    const uint idx = alloc_table_index_of_pointer(*mem);

    // Warn of double deletes.
    if (!ALLOC_TABLE[idx].isInUse)
    {
        k_assert(0, "Double-deleting memory.");
    }

    ALLOC_TABLE[idx].isInUse = false;

    if (!ALLOC_TABLE[idx].isReused)
    {
        TOTAL_BYTES_RELEASED += ALLOC_TABLE[idx].numBytes;
    }

    *mem = nullptr;

    return;
}

void kmem_lock_cache_alloc(void)
{
    k_assert(CACHE_ALLOC_LOCKED == 0, "Was asked to lock the memory cache, but it had already been locked.");

    CACHE_ALLOC_LOCKED = 1;

    return;
}

void kmem_deallocate_memory_cache(void)
{
    DEBUG(("Releasing the memory cache."));

    DEBUG(("Taking stock of allocations in the memory cache (estimated values)."));
    DEBUG(("Allocated:\t%d KB (%d%% utilization).", (TOTAL_BYTES_ALLOCATED / 1024), int((real(TOTAL_BYTES_ALLOCATED) / MEMORY_CACHE_SIZE) * 100)));
    DEBUG(("Released:\t%d KB.", (TOTAL_BYTES_RELEASED / 1024)));
    DEBUG(("Balance:\t%d bytes.", (TOTAL_BYTES_ALLOCATED - TOTAL_BYTES_RELEASED)));

    /*for (uint i = 0; i < NUM_ELEMENTS(ALLOC_TABLE); i++)
    {
        if ((ALLOC_TABLE[i].alloc != nullptr) &&
            !ALLOC_TABLE[i].isUnused)
        {
            DEBUG(("Unreleased memory: %p, %u bytes. Stated purpose: '%s'.", ALLOC_TABLE[i].alloc, ALLOC_TABLE[i].numBytes, ALLOC_TABLE[i].reason));
        }
    }*/

    DEBUG(("(Unreleased allocations will be freed automatically.)"));

    free(MEMORY_CACHE);

    return;
}
