/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef VECTOR_H
#define VECTOR_H

#include "vond/matrix.h"

template <typename T>
struct vector2_s
{
    T x, y;
};

template <typename T>
struct vector3_s
{
    T x, y, z;

    vector3_s<T> normalized(void) const
    {
        vector3_s<T> ret = *this;

        const double squaredNorm = ((this->x * this->x) + (this->y * this->y) + (this->z * this->z));

        if ((squaredNorm != 0) &&
            (squaredNorm != 1))
        {
            const double invNorm = (1 / sqrt(squaredNorm));

            ret.x *= invNorm;
            ret.y *= invNorm;
            ret.z *= invNorm;
        }

        return ret;
    }

    double dot(const vector3_s &other) const
    {
        return ((this->x * other.x) + (this->y * other.y) + (this->z * other.z));
    }

    double length(void) const
    {
        return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z));
    }

    double distance_to(const vector3_s &other) const
    {
        return sqrt(((other.x - this->x) * (other.x - this->x)) +
                    ((other.y - this->y) * (other.y - this->y)) +
                    ((other.z - this->z) * (other.z - this->z)));
    }

    vector3_s<T> operator*(const double &x) const
    {
        return {T(this->x * x),
                T(this->y * x),
                T(this->z * x)};
    }

    vector3_s<T> operator*(const matrix44_s &matrix) const
    {
        const decltype(this->x) x0 = ((matrix.elements[0] * this->x) + (matrix.elements[4] * this->y) + (matrix.elements[8] * this->z));
        const decltype(this->y) y0 = ((matrix.elements[1] * this->x) + (matrix.elements[5] * this->y) + (matrix.elements[9] * this->z));
        const decltype(this->z) z0 = ((matrix.elements[2] * this->x) + (matrix.elements[6] * this->y) + (matrix.elements[10] * this->z));

        return {x0, y0, z0};
    }

    vector3_s<T> operator-(const vector3_s &other) const
    {
        return {T(this->x - other.x),
                T(this->y - other.y),
                T(this->z - other.z)};
    }

    void operator*=(const matrix44_s &matrix)
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
