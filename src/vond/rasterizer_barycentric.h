/*
 * 2020 Tarpeeksi Hyvae Soft
 *
 * Software: Vond
 *
 */

#ifndef RASTERIZER_BARYCENTRIC_H
#define RASTERIZER_BARYCENTRIC_H

#include <stdint.h>

struct triangle_s;
template <typename T, size_t NumColorChannels> struct image_s;

// Rasterizes the given triangle into the given pixel map using barycentric
// coordinate-based rendering.
void kr_barycentric_rasterize_triangle(const triangle_s &tri,
                                       image_s<uint8_t, 4> &dstPixelmap,
                                       image_s<double, 1> &dstDepthmap);

#endif
