#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <string>
#include "vond/assert.h"
#include "vond/image.h"
#include "vond/vertex.h"

struct triangle_material_s
{
    std::string name;
    color_rgba_s<uint8_t> baseColor;
    image_s<uint8_t> *texture = nullptr;
};

struct triangle_s
{
    std::array<vertex4_s, 3> v;
    triangle_material_s material;
};

#endif
