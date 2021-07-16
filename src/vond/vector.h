/*
 * Tarpeeksi Hyvae Soft 2018-2021
 *
 * Software: Vond
 *
 */

#ifndef VOND_VECTOR_H
#define VOND_VECTOR_H

#include "vond/matrix.h"

namespace vond
{
    template <typename T, std::size_t NumComponents>
    struct vector
    {
        T components[NumComponents];

        std::size_t size(void) const
        {
            return NumComponents;
        }

        vond::vector<T, NumComponents> normalized(void) const
        {
            vond::vector<T, NumComponents> normalizedVec = *this;

            const double selfLength = this->length();

            if ((selfLength != 0) &&
                (selfLength != 1))
            {
                const double invNorm = (1 / sqrt(selfLength));

                for (std::size_t i = 0; i < NumComponents; i++)
                {
                    normalizedVec.components[i] *= invNorm;
                }
            }

            return normalizedVec;
        }

        double dot(const vond::vector<T, NumComponents> &other) const
        {
            double dotVal = 0;
            for (std::size_t i = 0; i < NumComponents; i++)
            {
                dotVal += (this->components[i] * other.components[i]);
            }

            return dotVal;
        }

        vond::vector<T, 3> cross(const vond::vector<T, 3> &other) const
        {
            static_assert(NumComponents == 3);

            return {((this->components[1] * other.components[2]) - (this->components[2] * other.components[1])),
                    ((this->components[2] * other.components[0]) - (this->components[0] * other.components[2])),
                    ((this->components[0] * other.components[1]) - (this->components[1] * other.components[0]))};
        }

        double length(void) const
        {
            double lengthVal = 0;
            for (std::size_t i = 0; i < NumComponents; i++)
            {
                lengthVal += (this->components[i] * this->components[i]);
            }

            return lengthVal;
        }

        double distance_to(const vond::vector<T, NumComponents> &other) const
        {
            double distanceVal = 0;
            for (std::size_t i = 0; i < NumComponents; i++)
            {
                distanceVal += ((other.components[i] - this->components[i]) * (other.components[i] - this->components[i]));
            }

            return sqrt(distanceVal);
        }

        vond::vector<T, NumComponents> lerp(const vond::vector<T, NumComponents> &other, const double lerp) const
        {
            vond::vector<T, NumComponents> lerpedVec;
            for (std::size_t i = 0; i < NumComponents; i++)
            {
                lerpedVec.components[i] = std::lerp(this->components[i], other.components[i], lerp);
            }

            return lerpedVec;
        }

        T& operator[](const std::size_t componentIdx)
        {
            vond_optional_assert((componentIdx < NumComponents), "About to overflow the vector component array.");

            return this->components[componentIdx];
        }

        T operator[](const std::size_t componentIdx) const
        {
            vond_optional_assert((componentIdx < NumComponents), "About to overflow the vector component array.");

            return this->components[componentIdx];
        }

        vond::vector<T, 3> operator*(const vond::matrix44 &m44) const
        {
            static_assert(NumComponents == 3);

            vond::vector<T, 3> returnVec = *this;

            return (returnVec *= m44);
        }

        vond::vector<T, 3>& operator*=(const vond::matrix44 &m44)
        {
            static_assert(NumComponents == 3);

            T a = ((m44.elements[0] * this->components[0]) + (m44.elements[4] * this->components[1]) + (m44.elements[ 8] * this->components[2]));
            T b = ((m44.elements[1] * this->components[0]) + (m44.elements[5] * this->components[1]) + (m44.elements[ 9] * this->components[2]));
            T c = ((m44.elements[2] * this->components[0]) + (m44.elements[6] * this->components[1]) + (m44.elements[10] * this->components[2]));

            this->components[0] = a;
            this->components[1] = b;
            this->components[2] = c;

            return *this;
        }

        vond::vector<T, NumComponents> operator-(const vond::vector<T, NumComponents> &other) const
        {
            vond::vector<T, NumComponents> returnVec = *this;
            for (std::size_t i = 0; i < NumComponents; i++)
            {
                returnVec.components[i] -= other.components[i];
            }

            return returnVec;
        }

        vond::vector<T, NumComponents> operator*(const double x) const
        {
            vond::vector<T, NumComponents> returnVec = *this;

            return (returnVec *= x);
        }

        vond::vector<T, NumComponents>& operator*=(const double x)
        {
            for (std::size_t i = 0; i < NumComponents; i++)
            {
                this->components[i] *= x;
            }

            return *this;
        }

        vond::vector<T, NumComponents> operator*(const vond::vector<T, NumComponents> other) const
        {
            vond::vector<T, NumComponents> returnVec = *this;

            return (returnVec *= other);
        }

        vond::vector<T, NumComponents>& operator*=(const vond::vector<T, NumComponents> &other)
        {
            for (std::size_t i = 0; i < NumComponents; i++)
            {
                this.components[i] *= other.components[i];
            }

            return *this;
        }

        vond::vector<T, NumComponents> operator+(const vond::vector<T, NumComponents> other) const
        {
            vond::vector<T, NumComponents> returnVec = *this;

            return (returnVec += other);
        }

        vond::vector<T, NumComponents>& operator+=(const vond::vector<T, NumComponents> &other)
        {
            for (std::size_t i = 0; i < NumComponents; i++)
            {
                this->components[i] += other.components[i];
            }

            return *this;
        }
    };

    template <typename T>
    using vector2 = vector<T, 2>;

    template <typename T>
    using vector3 = vector<T, 3>;
}

#endif
