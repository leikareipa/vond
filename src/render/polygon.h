/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef RENDER_POLYGONS_H
#define RENDER_POLYGONS_H

#include <vector>
#include <string>
#include "../../src/data_access/asset_store.h"
#include "../../src/vector.h"
#include "../../src/memory.h"

class framebuffer_s;
class image_s;
struct camera_s;

struct polygon_material_s
{
    std::string name;
    heap_bytes_s<image_s> texture;           // If null, baseColor will be used instead.
    color_rgba_s baseColor;
};

struct vertex4_s
{
    real x = 0, y = 0, z = 0, w = 1;
    real depth = 0;
    real uv[2] = {0, 0};

    void transform(const Matrix44 &mat)
    {
        const decltype(this->x) x0 = ((mat.elements[0] * this->x) + (mat.elements[4] * this->y) + (mat.elements[8] * this->z) + (mat.elements[12] * this->w));
        const decltype(this->y) y0 = ((mat.elements[1] * this->x) + (mat.elements[5] * this->y) + (mat.elements[9] * this->z) + (mat.elements[13] * this->w));
        const decltype(this->z) z0 = ((mat.elements[2] * this->x) + (mat.elements[6] * this->y) + (mat.elements[10] * this->z) + (mat.elements[14] * this->w));
        const decltype(this->w) w0 = ((mat.elements[3] * this->x) + (mat.elements[7] * this->y) + (mat.elements[11] * this->z) + (mat.elements[15] * this->w));

        this->x = x0;
        this->y = y0;
        this->z = z0;
        this->w = w0;

        return;
    }
};

struct triangle_s
{
    vertex4_s v[3];
    asset_s<polygon_material_s> material;
};

void kr_draw_triangles(const std::vector<triangle_s> &triangles, const camera_s &camera, framebuffer_s *const framebuffer);

#endif


















