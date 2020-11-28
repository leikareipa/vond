/*
 * 2020 Tarpeeksi Hyvae Soft
 *
 * Software: Vond
 *
 */

#ifndef RASTERIZER_SCANLINE_H
#define RASTERIZER_SCANLINE_H

#include <stdint.h>

struct triangle_s;
template <typename T, size_t NumColorChannels> struct image_s;

// Parameters to be interpolated - during rendering - vertically along triangle
// edges and horizontally along pixel spans.
struct interpolation_params_s
{
    double startX;
    double u;
    double v;
    double invW;
    double depth;

    // Increments all parameters by the given deltas.
    // Note: Assumes that all params are of type double, and that there are as
    // many left-side params/deltas as right-side params/deltas.
    void increment(const interpolation_params_s &deltas, const unsigned times = 1)
    {
        const unsigned numParams = (sizeof(interpolation_params_s) / sizeof(double));

        double *val = (double*)this;
        double *delta = (double*)&deltas;

        for (unsigned i = 0; i < numParams; i++)
        {
            *val += (*delta * times);

            val++;
            delta++;
        }
    }
};

// Rasterizes the given triangle into the given pixel map using scanline-based
// rendering.
void kr_scanline_rasterize_triangle(const triangle_s &tri,
                                    image_s<uint8_t, 4> &dstPixelmap,
                                    image_s<double, 1> &dstDepthmap);

#endif
