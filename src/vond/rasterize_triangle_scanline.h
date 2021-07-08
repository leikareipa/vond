/*
 * 2020 Tarpeeksi Hyvae Soft
 *
 * Software: Vond
 *
 */

#ifndef RASTERIZER_SCANLINE_H
#define RASTERIZER_SCANLINE_H

#include <stdint.h>
#include "vond/image.h"
#include "vond/triangle.h"

namespace vond::rasterize_triangle
{
    // Rasterizes the given triangle into the given pixel map using scanline-based
    // rendering.
    void scanline(const vond::triangle &tri,
                  vond::image<uint8_t, 4> &dstPixelmap,
                  vond::image<double, 1> &dstDepthmap);
}

#endif
