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

namespace vond
{
    template <typename T>
    struct rect
    {
        vond::vector2<T> topLeft = {};
        vond::vector2<T> bottomRight = {};

        static rect<T> from_triangle(const vond::triangle &tri)
        {
            rect<T> triRect;

            double minX = std::numeric_limits<double>::max();
            double maxX = std::numeric_limits<double>::lowest();
            double minY = minX;
            double maxY = maxX;

            for (unsigned i = 0; i < 3; i++)
            {
                minX = std::min(minX, tri.v[i].position[0]);
                maxX = std::max(maxX, tri.v[i].position[0]);
                minY = std::min(minY, tri.v[i].position[1]);
                maxY = std::max(maxY, tri.v[i].position[1]);
            }

            triRect.topLeft = {T(minX), T(minY)};
            triRect.bottomRight = {T(maxX), T(maxY)};

            return triRect;
        }

        rect<T> clipped_against(const rect<T> &other)
        {
            rect<T> newRect;

            newRect.topLeft[0] = std::max(this->left(), other.left());
            newRect.topLeft[1] = std::max(this->top(), other.top());
            newRect.bottomRight[0] = std::min(this->right(), other.right());
            newRect.bottomRight[1] = std::min(this->bottom(), other.bottom());

            return newRect;
        }

        T width() const { return (bottomRight[0] - topLeft[0]); }
        T height() const { return (bottomRight[1] - topLeft[1]); }
        T left() const { return topLeft[0]; }
        T right() const { return bottomRight[0]; }
        T top() const { return topLeft[1]; }
        T bottom() const { return bottomRight[1]; }
    };
}

#endif
