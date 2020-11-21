/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef RENDER_LANDSCAPE_H
#define RENDER_LANDSCAPE_H

struct camera_s;

#include "src/image.h"

void kr_draw_landscape(const image_s<double> &srcHeightmap,
                       const image_s<u8> &srcTexture,
                       image_s<u8> &dstPixelmap,
                       image_s<double> &dstDepthmap,
                       const camera_s &camera);

#endif
