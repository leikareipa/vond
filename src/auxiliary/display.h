/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef DISPLAY_H
#define DISPLAY_H

struct camera_s;
template <typename T> struct image_s;

void kd_acquire_display(const unsigned width, const unsigned height, const char *const title);

void kd_release_display(void);

void kd_update_input(camera_s *const camera);

void kd_update_display(const image_s<uint8_t> &pixelmap);

void kd_target_fps(const unsigned fps);

unsigned kd_current_fps(void);

#endif
