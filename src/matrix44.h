/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * 4 x 4 matrix manipulation.
 *
 *      Adapted from code originally written by Benny Bobaganoosh for his 3d software renderer
 *      (https://github.com/BennyQBD/3DSoftwareRenderer). Full attribution:
 *      {
 *          Copyright (c) 2014, Benny Bobaganoosh
 *          All rights reserved.
 *
 *          Redistribution and use in source and binary forms, with or without
 *          modification, are permitted provided that the following conditions are met:
 *
 *          1. Redistributions of source code must retain the above copyright notice, this
 *              list of conditions and the following disclaimer.
 *          2. Redistributions in binary form must reproduce the above copyright notice,
 *              this list of conditions and the following disclaimer in the documentation
 *              and/or other materials provided with the distribution.
 *
 *          THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *          ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *          WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *          DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 *          ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *          (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *          LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *          ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *          (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *          SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *      }
 *
 */

#ifndef MATRIX44_H
#define MATRIX44_H

#include <cmath>
#include "../src/common.h"

struct matrix44_s
{
    static const uint sideLen = 4;
    real elements[sideLen * sideLen];

    void operator*=(const matrix44_s &other)
    {
        for(uint i = 0; i < sideLen; i++)
        {
            for(uint j = 0; j < sideLen; j++)
            {
                this->operator()(i,j) = ((this->operator()(i,0) * other(0,j)) +
                                         (this->operator()(i,1) * other(1,j)) +
                                         (this->operator()(i,2) * other(2,j)) +
                                         (this->operator()(i,3) * other(3,j)));
            }
        }

        return;
    }

    matrix44_s operator*(const matrix44_s &other) const
    {
        matrix44_s m;

        for(uint i = 0; i < sideLen; i++)
        {
            for(uint j = 0; j < sideLen; j++)
            {
                m(i,j) = ((this->operator()(i,0) * other(0,j)) +
                          (this->operator()(i,1) * other(1,j)) +
                          (this->operator()(i,2) * other(2,j)) +
                          (this->operator()(i,3) * other(3,j)));
            }
        }

        return m;
    }

    real& operator()(const unsigned int i, const unsigned int j)
    {
        return elements[i+j*sideLen];
    }

    real operator()(const unsigned int i, const unsigned int j) const
    {
        return elements[i+j*sideLen];
    }

};

struct matrix44_identity_s : public matrix44_s
{
    matrix44_identity_s()
    {
        elements[0] = 1;	elements[4] = 0;	elements[8]  = 0;	elements[12] = 0;
        elements[1] = 0;	elements[5] = 1;	elements[9]  = 0;	elements[13] = 0;
        elements[2] = 0;	elements[6] = 0;	elements[10] = 1;	elements[14] = 0;
        elements[3] = 0;	elements[7] = 0;	elements[11] = 0;	elements[15] = 1;
    }
};

struct matrix44_rotation_s : public matrix44_s
{
    matrix44_rotation_s(real x, real y, real z)
    {
        matrix44_s rx, ry, rz;

        rx.elements[0] = 1;             rx.elements[4] = 0;              rx.elements[8]  = 0;                 rx.elements[12] = 0;
        rx.elements[1] = 0;             rx.elements[5] = (real)cos(x);	 rx.elements[9]  = -(real)sin(x);     rx.elements[13] = 0;
        rx.elements[2] = 0;             rx.elements[6] = (real)sin(x);   rx.elements[10] = (real)cos(x);      rx.elements[14] = 0;
        rx.elements[3] = 0;             rx.elements[7] = 0;              rx.elements[11] = 0;                 rx.elements[15] = 1;

        ry.elements[0] = (real)cos(y);  ry.elements[4] = 0;              ry.elements[8]  = -(real)sin(y);     ry.elements[12] = 0;
        ry.elements[1] = 0;             ry.elements[5] = 1;              ry.elements[9]  = 0;                 ry.elements[13] = 0;
        ry.elements[2] = (real)sin(y);  ry.elements[6] = 0;              ry.elements[10] = (real)cos(y);      ry.elements[14] = 0;
        ry.elements[3] = 0;             ry.elements[7] = 0;              ry.elements[11] = 0;                 ry.elements[15] = 1;

        rz.elements[0] = (real)cos(z);	rz.elements[4] = -(real)sin(z);	 rz.elements[8]  = 0;                 rz.elements[12] = 0;
        rz.elements[1] = (real)sin(z);	rz.elements[5] = (real)cos(z);	 rz.elements[9]  = 0;                 rz.elements[13] = 0;
        rz.elements[2] = 0;             rz.elements[6] = 0;              rz.elements[10] = 1;                 rz.elements[14] = 0;
        rz.elements[3] = 0;             rz.elements[7] = 0;              rz.elements[11] = 0;                 rz.elements[15] = 1;

        rx = (rx * ry * rz);

        for (uint i = 0; i < 16; i++)
        {
            this->elements[i] = rx.elements[i];
        }

        return;
    }
};

struct matrix44_translation_s : public matrix44_s
{
    matrix44_translation_s(real x, real y, real z)
    {
        elements[0] = 1;	elements[4] = 0;	elements[8]  = 0;	elements[12] = x;
        elements[1] = 0;	elements[5] = 1;	elements[9]  = 0;	elements[13] = y;
        elements[2] = 0;	elements[6] = 0;	elements[10] = 1;	elements[14] = z;
        elements[3] = 0;	elements[7] = 0;	elements[11] = 0;	elements[15] = 1;
    }
};

struct matrix44_perspective_s : public matrix44_s
{
    matrix44_perspective_s(real fov, real aspectRatio, real zNear, real zFar)
    {
        real tanHalfFOV = tan(fov / 2);
        real zRange = zNear - zFar;

        this->operator()(0, 0) = 1.0f / (tanHalfFOV * aspectRatio);	this->operator()(0, 1) = 0;		this->operator()(0, 2) = 0;                     this->operator()(0, 3) = 0;
        this->operator()(1, 0) = 0;						this->operator()(1, 1) = 1.0f / tanHalfFOV;	this->operator()(1, 2) = 0;                     this->operator()(1, 3) = 0;
        this->operator()(2, 0) = 0;						this->operator()(2, 1) = 0;					this->operator()(2, 2) = (-zNear -zFar)/zRange;	this->operator()(2, 3) = 2 * zFar * zNear / zRange;
        this->operator()(3, 0) = 0;						this->operator()(3, 1) = 0;					this->operator()(3, 2) = 1;                     this->operator()(3, 3) = 0;
    }
};

struct matrix44_scaling_s : public matrix44_s
{
    matrix44_scaling_s(real x, real y, real z)
    {
        elements[0] = x;	elements[4] = 0;	elements[8]  = 0;	elements[12] = 0;
        elements[1] = 0;	elements[5] = y;	elements[9]  = 0;	elements[13] = 0;
        elements[2] = 0;	elements[6] = 0;	elements[10] = z;	elements[14] = 0;
        elements[3] = 0;	elements[7] = 0;	elements[11] = 0;	elements[15] = 1;
    }
};

struct matrix44_screen_space_s : public matrix44_s
{
    matrix44_screen_space_s(real halfWidth, real halfHeight)
    {
        elements[0] = halfWidth;	elements[4] = 0;            elements[8]  = 0;	elements[12] = halfWidth - 0.5;
        elements[1] = 0;            elements[5] = -halfHeight;	elements[9]  = 0;	elements[13] = halfHeight - 0.5;
        elements[2] = 0;            elements[6] = 0;            elements[10] = 1;	elements[14] = 0;
        elements[3] = 0;            elements[7] = 0;            elements[11] = 0;	elements[15] = 1;
    }
};

#endif

