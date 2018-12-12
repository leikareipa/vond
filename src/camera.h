/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef CAMERA_H
#define CAMERA_H

#include "../../src/vector.h"
#include "../../src/types.h"

struct camera_s
{
    vector3_s pos;
    vector3_s orientation;
    real zoom;
    real fov;
};

#endif
