/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Vond
 *
 */

#ifndef RECT_H
#define RECT_H

#include <limits>
#include "vond/vector.h"
#include "vond/triangle.h"
#include "vond/image.h"

template <typename T>
struct rect_s
{
    vector2_s<T> topLeft = {};
    vector2_s<T> bottomRight = {};

    static rect_s<T> from_triangle(const triangle_s &tri)
    {
        rect_s<T> triRect;

        double minX = std::numeric_limits<double>::max();
        double maxX = std::numeric_limits<double>::lowest();
        double minY = minX;
        double maxY = maxX;

        for (unsigned i = 0; i < 3; i++)
        {
            minX = std::min(minX, tri.v[i].pos.x);
            maxX = std::max(maxX, tri.v[i].pos.x);
            minY = std::min(minY, tri.v[i].pos.y);
            maxY = std::max(maxY, tri.v[i].pos.y);
        }

        triRect.topLeft = {T(minX), T(minY)};
        triRect.bottomRight = {T(maxX), T(maxY)};

        return triRect;
    }

    rect_s<T> clipped_against(const rect_s<T> &other)
    {
        rect_s<T> newRect;

        newRect.topLeft.x = std::max(this->left(), other.left());
        newRect.topLeft.y = std::max(this->top(), other.top());
        newRect.bottomRight.x = std::min(this->right(), other.right());
        newRect.bottomRight.y = std::min(this->bottom(), other.bottom());

        return newRect;
    }

    T width() const { return (bottomRight.x - topLeft.x); }
    T height() const { return (bottomRight.y - topLeft.y); }
    T left() const { return topLeft.x; }
    T right() const { return bottomRight.x; }
    T top() const { return topLeft.y; }
    T bottom() const { return bottomRight.y; }
};

#endif
