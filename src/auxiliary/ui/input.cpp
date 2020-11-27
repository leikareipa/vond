/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * Processes user input into events in the program.
 *
 */

#include "auxiliary/ui/input.h"

static bool MOVING_FORWARD = false;
static bool MOVING_BACKWARD = false;
static bool MOVING_LEFT = false;
static bool MOVING_RIGHT = false;

/// Temp hack for movement. Good enough for testing, but will be replaced later.
void kinput_reset_input_state(void)
{
    MOVING_FORWARD = false;
    MOVING_BACKWARD = false;
    MOVING_LEFT = false;
    MOVING_RIGHT = false;

    return;
}

/// Temp hack for movement. Good enough for testing, but will be replaced later.
bool kinput_is_moving_forward(void)
{
    return MOVING_FORWARD;
}

/// Temp hack for movement. Good enough for testing, but will be replaced later.
bool kinput_is_moving_backward(void)
{
    return MOVING_BACKWARD;
}

/// Temp hack for movement. Good enough for testing, but will be replaced later.
bool kinput_is_moving_right(void)
{
    return MOVING_RIGHT;
}

/// Temp hack for movement. Good enough for testing, but will be replaced later.
bool kinput_is_moving_left(void)
{
    return MOVING_LEFT;
}

/// Temp hack for movement. Good enough for testing, but will be replaced later.
void kinput_move_camera_forward(const bool isMoving)
{
    MOVING_FORWARD = isMoving;

    return;
}

/// Temp hack for movement. Good enough for testing, but will be replaced later.
void kinput_move_camera_backward(const bool isMoving)
{
    MOVING_BACKWARD = isMoving;

    return;
}

/// Temp hack for movement. Good enough for testing, but will be replaced later.
void kinput_move_camera_right(const bool isMoving)
{
    MOVING_RIGHT = isMoving;

    return;
}

/// Temp hack for movement. Good enough for testing, but will be replaced later.
void kinput_move_camera_left(const bool isMoving)
{
    MOVING_LEFT = isMoving;

    return;
}
