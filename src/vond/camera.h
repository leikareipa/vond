/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef CAMERA_H
#define CAMERA_H

#include "vond/vector.h"
#include "types.h"

struct camera_s
{
    vector3_s<double> pos;
    vector3_s<double> orientation;
    real zoom;
    real fov;
};

#endif
