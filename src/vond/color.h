/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef VOND_COLOR_H
#define VOND_COLOR_H

#include "vond/assert.h"

namespace vond
{
    template <typename T, size_t N>
    struct color
    {
        T channel[N] = {0};

        T& operator[](const size_t idx)
        {
            vond_optional_assert((idx < N), "Overflowing color channels.");
            return this->channel[idx];
        }

        vond::color<T,N> operator*(const double scaler)
        {
            vond::color<T,N> newColor;

            for (unsigned i = 0; i < N; i++)
            {
                newColor.channel[i] = (this->channel[i] * scaler);
            }

            return newColor;
        }
    };
}

#endif
