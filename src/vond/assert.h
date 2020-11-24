/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef ASSERT_H
#define ASSERT_H

#ifdef NDEBUG
    #error "NDEBUG disables assertions."
#endif
#include <cassert>

#define vond_assert(condition, error_string) assert(condition && error_string);

// For assertions in performance-critical sections; not guaranteed to evaluate
// to an assertion in release-oriented buids.
#ifdef ENFORCE_OPTIONAL_ASSERTS
    #define vond_optional_assert vond_assert
#else
    #define vond_optional_assert(...)
#endif

#endif
