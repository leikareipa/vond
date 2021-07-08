/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef VOND_RENDER_POLYGONS_H
#define VOND_RENDER_POLYGONS_H

#include <vector>
#include <string>
#include "vond/vector.h"
#include "vond/vertex.h"
#include "vond/triangle.h"
#include "vond/camera.h"

namespace vond
{
    void render_triangles(const std::vector<vond::triangle> &triangles,
                          vond::image<uint8_t, 4> &dstPixelmap,
                          vond::image<double, 1> &dstDepthmap,
                          const vond::camera &camera);
}

#endif
