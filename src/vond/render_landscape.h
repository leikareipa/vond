/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef RENDER_LANDSCAPE_H
#define RENDER_LANDSCAPE_H

#include "vond/image.h"

struct camera_s;

void kr_draw_landscape(const image_s<double, 1> &srcHeightmap,
                       const image_s<uint8_t, 4> &srcTexture,
                       image_s<uint8_t, 4> &dstPixelmap,
                       image_s<double, 1> &dstDepthmap,
                       const camera_s &camera);

#endif
