#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <string>
#include "vond/image.h"
#include "vond/vertex.h"

struct triangle_material_s
{
    std::string name = "Unnamed material";
    color_s<uint8_t, 4> baseColor = {0};
    image_s<uint8_t, 4> *texture = nullptr;
};

struct triangle_s
{
    vertex_s v[3];
    triangle_material_s material;
};

#endif
