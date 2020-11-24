/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef COMMON_H
#define COMMON_H

#ifdef NDEBUG
    #error "NDEBUG disables assertions. Assertions are required by design."
#endif
#include <cassert>
#include <cstdio>

#include "../src/types.h"

#define k_assert(condition, error_string) assert(condition && error_string);

// For assertions in performance-critical sections; not guaranteed to evaluate
// to an assertion in release-oriented buids.
#ifdef ENFORCE_OPTIONAL_ASSERTS
    #define k_optional_assert k_assert
#else
    #define k_optional_assert(...)
#endif

#endif
