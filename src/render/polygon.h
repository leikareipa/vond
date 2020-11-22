/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef RENDER_POLYGONS_H
#define RENDER_POLYGONS_H

#include <vector>
#include <string>
#include "../../src/vector.h"

template <typename T> struct image_s;

class framebuffer_s;
struct camera_s;

struct polygon_material_s
{
    std::string name;
    image_s<u8> *texture = nullptr;           // If null, baseColor will be used instead.
    color_rgba_s<u8> baseColor;
};

struct vertex4_s
{
    vector3_s<double> pos = {0, 0, 0};
    double w = 1;
    double depth = 0;
    double uv[2] = {0, 0};

    void transform(const matrix44_s &mat)
    {
        const decltype(this->pos.x) x0 = ((mat.elements[0] * this->pos.x) + (mat.elements[4] * this->pos.y) + (mat.elements[ 8] * this->pos.z) + (mat.elements[12] * this->w));
        const decltype(this->pos.y) y0 = ((mat.elements[1] * this->pos.x) + (mat.elements[5] * this->pos.y) + (mat.elements[ 9] * this->pos.z) + (mat.elements[13] * this->w));
        const decltype(this->pos.z) z0 = ((mat.elements[2] * this->pos.x) + (mat.elements[6] * this->pos.y) + (mat.elements[10] * this->pos.z) + (mat.elements[14] * this->w));
        const decltype(this->w)     w0 = ((mat.elements[3] * this->pos.x) + (mat.elements[7] * this->pos.y) + (mat.elements[11] * this->pos.z) + (mat.elements[15] * this->w));

        this->pos.x = x0;
        this->pos.y = y0;
        this->pos.z = z0;
        this->w = w0;

        return;
    }
};

struct triangle_s
{
    vertex4_s v[3];
    polygon_material_s material;
};

void kr_draw_triangles(const std::vector<triangle_s> &triangles,
                       image_s<u8> &dstPixelmap,
                       image_s<double> &dstDepthmap,
                       const camera_s &camera);

#endif
