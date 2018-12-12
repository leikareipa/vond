/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef RENDER_H
#define RENDER_H

#include "../src/render/landscape.h"
#include "../src/render/polygon.h"
#include "../src/memory.h"
#include "../src/image.h"

struct framebuffer_s
{
    framebuffer_s(const uint width, const uint height, const uint bpp) :
        canvas(new image_s(width, height, bpp, "Framebuffer canvas"))
    {
        depthmap.alloc((width * height), "Framebuffer depthmap");

        return;
    }

    ~framebuffer_s(void)
    {
        delete canvas;
        depthmap.release_memory();

        return;
    }

    uint width(void) const
    {
        return canvas->width();
    }

    uint height(void) const
    {
        return canvas->height();
    }

    uint bpp(void) const
    {
        return canvas->bpp();
    }

    image_s *const canvas;
    heap_bytes_s<uint> depthmap;
};

void krend_clear_framebuffer(framebuffer_s *const framebuffer);

#endif
