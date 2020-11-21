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
#include "../../src/vector.h"
#include "../../src/types.h"

template <typename T>
struct image_s
{
    image_s(const uint width, const uint height, const uint bpp) :
        res({width, height, bpp})
    {
        this->allocate_memory();

        return;
    }

    // Creates a new image from a QImage, copying its pixels over.
    //
    image_s(const QImage &qImage) :
        image_s(uint(qImage.width()), uint(qImage.height()), uint(qImage.depth()))
    {
        k_assert(!qImage.isNull(), "Was asked to create an image out of a null QImage.");

        // Copy the pixels over.
        for (uint y = 0; y < res.h; y++)
        {
            for (uint x = 0; x < res.w; x++)
            {
                const QColor sourcePixel = qImage.pixel(x, y);
                color_rgba_s<T> &targetPixel = this->pixel_at(x, y);

                switch (qImage.depth())
                {
                    case 8:
                    {
                        targetPixel = {T(sourcePixel.value()),
                                       T(sourcePixel.value()),
                                       T(sourcePixel.value()),
                                       T(255)};
                        break;
                    }
                    case 32:
                    {
                        targetPixel = {T(sourcePixel.red()),
                                       T(sourcePixel.green()),
                                       T(sourcePixel.blue()),
                                       T(255)};
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

        return;
    }

    ~image_s(void)
    {
        delete [] pixels;

        return;
    }

    uint width(void) const
    {
        k_assert(pixels, "Tried to access the resolution of a null image.");

        return res.w;
    }

    uint height(void) const
    {
        k_assert(pixels, "Tried to access the resolution of a null image.");

        return res.h;
    }

    uint bpp(void) const
    {
        k_assert(pixels, "Tried to access the resolution of a null image.");

        return res.bpp;
    }

    const resolution_s& resolution(void) const
    {
        k_assert(pixels, "Tried to access the resolution of a null image.");

        return res;
    }

    color_rgba_s<T>& pixel_at(const uint x, const uint y) const
    {
        k_assert_optional(pixels, "Tried to access the pixels of a null image.");
        k_assert_optional(((x < res.w) && (y < res.h)), "Tried to access an image pixel out of bounds.");

        return pixels[(x + y * res.w)];
    }

    color_rgba_s<T> interpolated_pixel_at(const real x, const real y) const
    {
        k_assert_optional(pixels, "Tried to access the pixels of a null image.");
        k_assert_optional(((x < res.w) && (y < res.h)), "Tried to access an image pixel out of bounds.");

        const uint xFloored = floor(x);
        const uint yFloored = floor(y);
        const double xBias = (x - xFloored);
        const double yBias = (y - yFloored);

        const T r1 = LERP(pixels[(xFloored       + yFloored       * res.w)].r,
                          pixels[(xFloored       + (yFloored + 1) * res.w)].r, yBias);
        const T r2 = LERP(pixels[((xFloored + 1) + yFloored       * res.w)].r,
                          pixels[((xFloored + 1) + (yFloored + 1) * res.w)].r, yBias);

        const T g1 = LERP(pixels[(xFloored       + yFloored       * res.w)].g,
                          pixels[(xFloored       + (yFloored + 1) * res.w)].g, yBias);
        const T g2 = LERP(pixels[((xFloored + 1) + yFloored       * res.w)].g,
                          pixels[((xFloored + 1) + (yFloored + 1) * res.w)].g, yBias);

        const T b1 = LERP(pixels[(xFloored       + yFloored       * res.w)].b,
                          pixels[(xFloored       + (yFloored + 1) * res.w)].b, yBias);
        const T b2 = LERP(pixels[((xFloored + 1) + yFloored       * res.w)].b,
                          pixels[((xFloored + 1) + (yFloored + 1) * res.w)].b, yBias);

        return {T(LERP(r1, r2, xBias)),
                T(LERP(g1, g2, xBias)),
                T(LERP(b1, b2, xBias)),
                T(255)};
    }

    const u8* pixel_array(void) const
    {
        return (u8*)pixels;
    }

    void fill(const color_rgba_s<T> &fillColor)
    {
        for (uint y = 0; y < this->height(); y++)
        {
            for (uint x = 0; x < this->width(); x++)
            {
                this->pixel_at(x, y) = fillColor;
            }
        }

        return;
    }

    // Duplicates the given image into this one.
    //
    void operator=(const image_s &sourceImage)
    {
        delete [] this->pixels;

        this->res = sourceImage.res;
        this->allocate_memory();

        return;
    }

private:
    color_rgba_s<T> *pixels = nullptr;
    resolution_s res = {0, 0, 0};

    // Allocates enough room to hold an image of the size of the current resolution.
    // Note that the resolution must be set before calling this function.
    //
    void allocate_memory()
    {
        k_assert(((this->res.w != 0) && (this->res.w != 0) && (this->res.bpp != 0)),
                 "Expected a valid resolution to be set before allocating memory for the image's' pixels.");

        this->pixels = new color_rgba_s<T>[this->res.w * this->res.h];
        memset(pixels, 0, (sizeof(color_rgba_s<T>) * this->res.w * this->res.h));

        return;
    }
};

#endif
