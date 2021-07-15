/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef VOND_VERTEX_H
#define VOND_VERTEX_H

#include "vond/vector.h"
#include "vond/matrix.h"

namespace vond
{
    struct vertex
    {
        vond::vector3<double> position = {0, 0, 0};
        vond::vector2<double> uv = {0, 0};
        double w = 1;

        void transform(const vond::matrix44 &mat)
        {
            double x0 = ((mat.elements[0] * this->position[0]) + (mat.elements[4] * this->position[1]) + (mat.elements[ 8] * this->position[2]) + (mat.elements[12] * this->w));
            double y0 = ((mat.elements[1] * this->position[0]) + (mat.elements[5] * this->position[1]) + (mat.elements[ 9] * this->position[2]) + (mat.elements[13] * this->w));
            double z0 = ((mat.elements[2] * this->position[0]) + (mat.elements[6] * this->position[1]) + (mat.elements[10] * this->position[2]) + (mat.elements[14] * this->w));
            double w0 = ((mat.elements[3] * this->position[0]) + (mat.elements[7] * this->position[1]) + (mat.elements[11] * this->position[2]) + (mat.elements[15] * this->w));

            this->position[0] = x0;
            this->position[1] = y0;
            this->position[2] = z0;
            this->w = w0;

            return;
        }
    };
}

#endif
