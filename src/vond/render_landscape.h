/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef VOND_RENDER_LANDSCAPE_H
#define VOND_RENDER_LANDSCAPE_H

#include <functional>
#include "vond/image.h"
#include "vond/color.h"
#include "vond/image_mosaic.h"
#include "vond/camera.h"

namespace vond
{
    void render_landscape(std::function<vond::color<double, 1>(const double x, const double y)> heightmapSampler,
                          std::function<vond::color<uint8_t, 4>(const double x, const double y)> textureSampler,
                          std::function<vond::color<uint8_t, 4>(const vond::vector3<double> &direction)> skySampler,
                          vond::image<uint8_t, 4> &dstPixelmap,
                          vond::image<double, 1> &dstDepthmap,
                          const vond::camera &camera);
}

#endif
