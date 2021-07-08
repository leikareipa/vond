/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "vond/camera.h"
#include "vond/image.h"

void kd_acquire_display(const unsigned width, const unsigned height, const char *const title);

void kd_release_display(void);

void kd_update_input(vond::camera *const camera);

void kd_update_display(const vond::image<uint8_t, 4> &pixelmap);

void kd_target_fps(const unsigned fps);

unsigned kd_current_fps(void);

#endif
