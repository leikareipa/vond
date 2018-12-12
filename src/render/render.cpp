/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#include "../../src/render.h"

void krend_clear_framebuffer(framebuffer_s *const framebuffer)
{
    for (uint y = 0; y < framebuffer->height(); y++)
    {
        for (uint x = 0; x < framebuffer->width(); x++)
        {
            framebuffer->canvas->pixel_at(x, y) = {0, 0, 0, 255};

            static_assert(std::is_same<decltype(framebuffer->depthmap), heap_bytes_s<uint>>::value, "Expected unsigned int heap bytes.");
            framebuffer->depthmap[x + y * framebuffer->width()] = ~0u;
        }
    }

    return;
}
