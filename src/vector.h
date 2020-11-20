/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef VECTOR_H
#define VECTOR_H

#include "../src/types.h"
#include "../src/matrix44.h"

template <typename T>
struct vector2_s
{
    T x, y;
};

struct vector3_s
{
    real x, y, z;

    vector3_s(const real _x = 0.0, const real _y = 0.0, const real _z = 0.0)
    {
        x = _x;
        y = _y;
        z = _z;

        return;
    }

    vector3_s normalized(void)
    {
        vector3_s ret = *this;

        const real squaredNorm = ((this->x * this->x) + (this->y * this->y) + (this->z * this->z));

        if (squaredNorm != 0 && squaredNorm != 1)
        {
            const real invNorm = (1.0 / sqrt(squaredNorm));
            ret.x *= invNorm;
            ret.y *= invNorm;
            ret.z *= invNorm;
        }

        return ret;
    }

    real dot(const vector3_s &other) const
    {
        return real((this->x * other.x) + (this->y * other.y) + (this->z * other.z));
    }

    real length(void) const
    {
        return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z));
    }

    void operator=(const vector3_s &other)
    {
        this->x = other.x;
        this->y = other.y;
        this->z = other.z;

        return;
    }

    vector3_s operator*(const real &x) const
    {
        return vector3_s((this->x * x), (this->y * x), (this->z * x));
    }

    vector3_s operator*(const Matrix44 &matrix) const
    {
        const decltype(this->x) x0 = ((matrix.elements[0] * this->x) + (matrix.elements[4] * this->y) + (matrix.elements[8] * this->z));
        const decltype(this->y) y0 = ((matrix.elements[1] * this->x) + (matrix.elements[5] * this->y) + (matrix.elements[9] * this->z));
        const decltype(this->z) z0 = ((matrix.elements[2] * this->x) + (matrix.elements[6] * this->y) + (matrix.elements[10] * this->z));

        return vector3_s(x0, y0, z0);
    }

    vector3_s operator-(const vector3_s &other) const
    {
        return {(this->x - other.x),
                (this->y - other.y),
                (this->z - other.z)};
    }

    void operator*=(const Matrix44 &matrix)
    {
        *this = (*this * matrix);

        return;
    }

    void operator*=(const vector3_s &v)
    {
        this->x *= v.x;
        this->y *= v.y;
        this->z *= v.z;

        return;
    }

    void operator+=(const vector3_s &other)
    {
        this->x += other.x;
        this->y += other.y;
        this->z += other.z;

        return;
    }
};

#endif
