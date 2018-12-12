/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "types.h"

struct camera_s;
class image_s;

void kd_acquire_display(const uint width, const uint height, const char *const title);

void kd_release_display(void);

void kd_update_input(camera_s *const camera);

void kd_update_display(const image_s &frameBufferCanvas);

void kd_target_fps(const uint fps);

uint kd_current_fps(void);

#endif
