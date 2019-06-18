/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef IMAGE_H
#define IMAGE_H

#include <QImage>
#include <QColor>
#include "../../src/memory/memory_interface.h"

struct image_s
{
    image_s(const uint width, const uint height, const uint bpp, const char *const reason = nullptr) :
        res({width, height, bpp})
    {
        this->allocate_memory(reason);

        return;
    }

    // Creates a new image from a QImage, copying its pixels over.
    //
    image_s(const QImage &qImage, const char *const reason = nullptr) :
        res({uint(qImage.width()), uint(qImage.height()), uint(qImage.depth())})
    {
        k_assert(!qImage.isNull(), "Was asked to create an image out of a null QImage.");

        this->allocate_memory(reason);

        // Copy the pixels over.
        for (uint y = 0; y < res.h; y++)
        {
            for (uint x = 0; x < res.w; x++)
            {
                const QColor sourcePixel = qImage.pixel(x, y);
                color_rgba_s &targetPixel = pixel_at(x, y);

                switch (qImage.depth())
                {
                    case 8:
                    {
                        targetPixel = {u8(sourcePixel.value()),
                                       u8(sourcePixel.value()),
                                       u8(sourcePixel.value()),
                                       255};
                        break;
                    }
                    case 32:
                    {
                        targetPixel = {u8(sourcePixel.red()),
                                       u8(sourcePixel.green()),
                                       u8(sourcePixel.blue()),
                                       255};
                        break;
                    }
                    default:
                    {
                        k_assert(0, "Encountered an unknown color depth when converting from a QImage.");
                        break;
                    }
                }
            }
        }

        DEBUG(("Converted a QImage (%d x %d @ %d bpp) into an internal image (%d MB).",
               qImage.width(), qImage.height(), qImage.depth(), this->pixels.num_bytes() / 1024 / 1024));

        return;
    }

    ~image_s(void)
    {
      //  pixels.release_memory();

        return;
    }

    uint width(void) const
    {
        k_assert(!pixels.is_null(), "Tried to access the resolution of a null image.");

        return res.w;
    }

    uint height(void) const
    {
        k_assert(!pixels.is_null(), "Tried to access the resolution of a null image.");

        return res.h;
    }

    uint bpp(void) const
    {
        k_assert(!pixels.is_null(), "Tried to access the resolution of a null image.");

        return res.bpp;
    }

    const resolution_s& resolution(void) const
    {
        k_assert(!pixels.is_null(), "Tried to access the resolution of a null image.");

        return res;
    }

    color_rgba_s& pixel_at(const uint x, const uint y) const
    {
        k_assert_optional(!pixels.is_null(), "Tried to access the pixels of a null image.");
        k_assert_optional(((x < res.w) && (y < res.h)), "Tried to access an image pixel out of bounds.");

        return pixels[(x + y * res.w)];
    }

    const u8* pixel_array(void) const
    {
        return (u8*)pixels.ptr();
    }

    // Duplicates the given image into this one.
    //
    void operator=(const image_s &sourceImage)
    {
        this->res = sourceImage.res;
        this->allocate_memory("(Automatic image assignment)");

        for (uint y = 0; y < this->height(); y++)
        {
            for (uint x = 0; x < this->width(); x++)
            {
                this->pixel_at(x, y) = sourceImage.pixel_at(x, y);
            }
        }

        return;
    }

private:
    heap_bytes_s<color_rgba_s> pixels;
    resolution_s res = {0, 0, 0};

    // Allocates enough room to hold an image of the size of the current resolution.
    // Note that the resolution must be set before calling this function.
    //
    void allocate_memory(const char *const reason)
    {
        k_assert(((this->res.w != 0) && (this->res.w != 0) && (this->res.bpp != 0)),
                 "Expected a valid resolution to be set before allocating memory for the image's' pixels.");

        pixels.alloc((this->res.w * this->res.h), (reason == nullptr? "(No reason given)" : reason));

        return;
    }
};

#endif
