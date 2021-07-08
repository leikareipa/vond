#ifndef VOND_TRIANGLE_H
#define VOND_TRIANGLE_H

#include <string>
#include "vond/image.h"
#include "vond/vertex.h"

namespace vond
{
    struct triangle_material
    {
        std::string name = "Unnamed material";
        vond::color<uint8_t, 4> baseColor = {0};
        vond::image<uint8_t, 4> *texture = nullptr;
    };

    struct triangle
    {
        vond::vertex v[3];
        vond::triangle_material material;
    };
}

#endif
