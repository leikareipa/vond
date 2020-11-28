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

template <typename T, size_t NumColorChannels> struct image_s;
class framebuffer_s;
struct camera_s;

void kr_draw_triangles(const std::vector<triangle_s> &triangles,
                       image_s<uint8_t, 4> &dstPixelmap,
                       image_s<double, 1> &dstDepthmap,
                       const camera_s &camera);

#endif
