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

void kinput_move_camera_forward(void);

void kinput_move_camera_backward(void);

#endif
