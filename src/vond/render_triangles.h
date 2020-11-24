/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef RENDER_POLYGONS_H
#define RENDER_POLYGONS_H

#include <vector>
#include <string>
#include "vond/vector.h"
#include "vond/vertex.h"
#include "vond/triangle.h"

template <typename T> struct image_s;
class framebuffer_s;
struct camera_s;

void kr_draw_triangles(const std::vector<triangle_s> &triangles,
                       image_s<uint8_t> &dstPixelmap,
                       image_s<double> &dstDepthmap,
                       const camera_s &camera);

#endif
