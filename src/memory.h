/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef MEMORY_H
#define MEMORY_H

#include "../src/types.h"

void kmem_lock_cache_alloc(void);

void* kmem_allocate(const int numBytes, const char *const reason);

void kmem_release(void **mem);

uint kmem_sizeof_allocation(const void *const mem);

void kmem_deallocate_memory_cache(void);

#include "../src/memory/memory_interface.h"

#endif
