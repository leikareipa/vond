/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef CAMERA_H
#define CAMERA_H

#include "vond/vector.h"

struct camera_s
{
    vector3_s<double> pos;
    vector3_s<double> orientation;
    double zoom;
    double fov;
};

#endif
