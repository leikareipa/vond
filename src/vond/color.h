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
    template <typename T, std::size_t N>
    struct color
    {
        T channel[N] = {0};

        T& channel_at(const std::size_t idx)
        {
            vond_optional_assert((idx < N), "Overflowing color channels.");
            return this->channel[idx];
        }

        T channel_at(const std::size_t idx) const
        {
            vond_optional_assert((idx < N), "Overflowing color channels.");
            return this->channel[idx];
        }

        T operator[](const std::size_t idx) const
        {
            return this->channel[idx];
        }

        vond::color<T, N>& operator*=(const double scalar)
        {
            for (unsigned i = 0; i < N; i++)
            {
                this->channel[i] *= scalar;
            }

            return *this;
        }

        vond::color<T, N> operator*(const double scalar) const
        {
            auto color = *this;
            return (color *= scalar);
        }
    };
}

#endif
