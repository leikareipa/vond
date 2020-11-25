/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef VERTEX_H
#define VERTEX_H

#include "vond/vector.h"
#include "vond/matrix.h"

struct vertex_s
{
    vector3_s<double> pos = {0, 0, 0};
    double w = 1;
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

#endif
