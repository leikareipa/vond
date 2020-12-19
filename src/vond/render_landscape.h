/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef RENDER_LANDSCAPE_H
#define RENDER_LANDSCAPE_H

#include <functional>
#include "vond/image.h"
#include "vond/color.h"
#include "vond/image_mosaic.h"

struct camera_s;

void kr_draw_landscape(std::function<color_s<double, 1>(const double x, const double y)> terrainHeightmapSampler,
                       std::function<color_s<uint8_t, 4>(const double x, const double y)> terrainTextureSampler,
                       image_s<uint8_t, 4> &dstPixelmap,
                       image_s<double, 1> &dstDepthmap,
                       const camera_s &camera);

#endif
