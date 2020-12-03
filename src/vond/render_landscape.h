/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef RENDER_LANDSCAPE_H
#define RENDER_LANDSCAPE_H

#include "vond/image.h"
#include "vond/image_mosaic.h"

struct camera_s;

void kr_draw_landscape(const image_mosaic_c<double, 1> &srcHeightmap,
                       const image_mosaic_c<uint8_t, 4> &srcTexture,
                       image_s<uint8_t, 4> &dstPixelmap,
                       image_s<double, 1> &dstDepthmap,
                       const camera_s &camera);

#endif
