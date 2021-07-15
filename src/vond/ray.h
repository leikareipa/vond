/*
 * Tarpeeksi Hyvae Soft 2018-2021
 *
 * Software: Vond
 *
 */

#ifndef VOND_RAY_H
#define VOND_RAY_H

#include "vond/vector.h"

namespace vond
{
    struct ray
    {
        vond::vector3<double> origin;
        vond::vector3<double> direction;
    };
}

#endif
