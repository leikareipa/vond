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

template <typename T>
struct color_rgba_s
{
    T r, g, b, a;
};

struct resolution_s
{
    uint w, h, bpp;

    bool same_width_height_as(const resolution_s &other) const
    {
        return bool((this->w == other.w) && (this->h == other.h));
    }
};

#define NUM_ELEMENTS(array) int((sizeof(array) / sizeof((array)[0])))

#define LERP(x, y, step) ((x) + ((step) * ((y) - (x))))

#define k_assert(condition, error_string)   assert(condition && error_string);

// For assertions in performance-critical sections; not guaranteed to evaluate
// to an assertion in release-oriented buids.
#ifdef ENFORCE_OPTIONAL_ASSERTS
    #define k_assert_optional k_assert
#else
    #define k_assert_optional(...)
#endif

#define INFO(args)  (printf("[info ] {%s:%i} ", __FILE__, __LINE__), printf args, printf("\n"), fflush(stdout))
#define DEBUG(args) (printf("[debug] {%s:%i} ", __FILE__, __LINE__), printf args, printf("\n"), fflush(stdout))
#define ERROR(args) (fprintf(stderr, "[ERROR] {%s:%i} ", __FILE__, __LINE__), printf args, printf("\n"), fflush(stdout))

#endif
