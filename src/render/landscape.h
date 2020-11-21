/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef RENDER_LANDSCAPE_H
#define RENDER_LANDSCAPE_H

struct camera_s;
class framebuffer_s;
class image_s;

void kr_draw_landscape(const image_s &heightmap, const image_s &texture, const camera_s &camera, framebuffer_s *const frameBuffer);

#endif
