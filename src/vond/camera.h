/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef VOND_CAMERA_H
#define VOND_CAMERA_H

#include "vond/vector.h"

namespace vond
{
    struct camera
    {
        vond::vector3<double> pos;
        vond::vector3<double> orientation;
        double zoom;
        double fov;
    };
}

#endif
