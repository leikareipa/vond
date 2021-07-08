/*
 * 2020 Tarpeeksi Hyvae Soft
 *
 * Software: Vond
 *
 */

#ifndef RASTERIZER_BARYCENTRIC_H
#define RASTERIZER_BARYCENTRIC_H

#include <stdint.h>
#include "vond/triangle.h"
#include "vond/image.h"

namespace vond::rasterize_triangle
{
    // Rasterizes the given triangle into the given pixel map using barycentric
    // coordinate-based rendering.
    void barycentric(const vond::triangle &tri,
                     vond::image<uint8_t, 4> &dstPixelmap,
                     vond::image<double, 1> &dstDepthmap);
}

#endif
