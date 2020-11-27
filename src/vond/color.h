/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef COLOR_H
#define COLOR_H

#include "vond/assert.h"

template <typename T, size_t N>
struct color_s
{
    T channel[N] = {0};

    T& operator[](const size_t idx)
    {
        vond_optional_assert((idx < N), "Overflowing color channels.");
        return this->channel[idx];
    }
};

#endif
