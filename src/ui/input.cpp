/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * Processes user input into events in the program.
 *
 */

#include "../../src/ui/input.h"

static bool MOVING_FORWARD = false;
static bool MOVING_BACKWARD = false;

/// Temp hack for movement. Good enough for testing, but will be replaced later.
void kinput_reset_input_state(void)
{
    MOVING_FORWARD = false;
    MOVING_BACKWARD = false;

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
void kinput_move_camera_forward(void)
{
    MOVING_FORWARD = true;

    return;
}

/// Temp hack for movement. Good enough for testing, but will be replaced later.
void kinput_move_camera_backward(void)
{
    MOVING_BACKWARD = true;

    return;
}
