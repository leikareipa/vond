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
    template <typename T, std::size_t NumChannels>
    struct color
    {
        T channel[NumChannels] = {0};

        T& channel_at(const std::size_t idx)
        {
            vond_optional_assert((idx < NumChannels), "Overflowing color channels.");
            return this->channel[idx];
        }

        T channel_at(const std::size_t idx) const
        {
            vond_optional_assert((idx < NumChannels), "Overflowing color channels.");
            return this->channel[idx];
        }

        T operator[](const std::size_t idx) const
        {
            return this->channel[idx];
        }

        vond::color<T, NumChannels>& operator*=(const double scalar)
        {
            for (unsigned i = 0; i < NumChannels; i++)
            {
                this->channel[i] *= scalar;
            }

            return *this;
        }

        vond::color<T, NumChannels> operator*(const double scalar) const
        {
            auto color = *this;
            return (color *= scalar);
        }
    };

    template <typename T>
    using color_grayscale = color<T, 1>;

    template <typename T>
    using color_rgb = color<T, 3>;

    template <typename T>
    using color_rgba = color<T, 4>;
}

#endif
