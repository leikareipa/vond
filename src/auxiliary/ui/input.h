/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef UI_INPUT_H
#define UI_INPUT_H

void kinput_reset_input_state(void);

bool kinput_is_moving_forward(void);

bool kinput_is_moving_backward(void);

bool kinput_is_moving_left(void);

bool kinput_is_moving_right(void);

void kinput_move_camera_forward(const bool isMoving);

void kinput_move_camera_backward(const bool isMoving);

void kinput_move_camera_left(const bool isMoving);

void kinput_move_camera_right(const bool isMoving);

#endif
