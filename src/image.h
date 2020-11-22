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
        width_(width),
        height_(height),
        bpp_(bpp),
        pixels(new color_rgba_s<T>[width * height])
    {
        k_assert(((this->width() > 0) &&
                  (this->height() > 0) &&
                  (this->bpp() > 0)),
                 "Invalid image resolution detected.");

        this->fill({0, 0, 0, 255});

        return;
    }

    // Creates a new image from a QImage, copying its pixels over.
    //
    image_s(const QImage &qImage) :
        image_s(uint(qImage.width()), uint(qImage.height()), uint(qImage.depth()))
    {
        k_assert(!qImage.isNull(), "Was asked to create an image out of a null QImage.");

        // Copy the pixels over.
        for (uint y = 0; y < this->height(); y++)
        {
            for (uint x = 0; x < this->width(); x++)
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
        return this->width_;
    }

    uint height(void) const
    {
        return this->height_;
    }

    uint bpp(void) const
    {
        return this->bpp_;
    }

    color_rgba_s<T>& pixel_at(int x, int y) const
    {
        std::tie(x, y) = this->bounds_checked_coordinates(x, y);

        k_assert_optional(pixels, "Tried to access the pixels of a null image.");
        k_assert_optional(((x < this->width()) && (y < this->height())), "Tried to access an image pixel out of bounds.");

        return pixels[(x + y * this->width())];
    }

    color_rgba_s<T> interpolated_pixel_at(double x, double y) const
    {
        std::tie(x, y) = this->bounds_checked_coordinates(x, y);

        k_assert_optional(pixels, "Tried to access the pixels of a null image.");
        k_assert_optional(((x < this->width()) && (y < this->height())), "Tried to access an image pixel out of bounds.");

        const uint xFloored = floor(x);
        const uint yFloored = floor(y);
        const double xBias = (x - xFloored);
        const double yBias = (y - yFloored);

        const T r1 = LERP(pixels[(xFloored       + yFloored       * this->width())].r,
                          pixels[(xFloored       + (yFloored + 1) * this->width())].r, yBias);
        const T r2 = LERP(pixels[((xFloored + 1) + yFloored       * this->width())].r,
                          pixels[((xFloored + 1) + (yFloored + 1) * this->width())].r, yBias);

        const T g1 = LERP(pixels[(xFloored       + yFloored       * this->width())].g,
                          pixels[(xFloored       + (yFloored + 1) * this->width())].g, yBias);
        const T g2 = LERP(pixels[((xFloored + 1) + yFloored       * this->width())].g,
                          pixels[((xFloored + 1) + (yFloored + 1) * this->width())].g, yBias);

        const T b1 = LERP(pixels[(xFloored       + yFloored       * this->width())].b,
                          pixels[(xFloored       + (yFloored + 1) * this->width())].b, yBias);
        const T b2 = LERP(pixels[((xFloored + 1) + yFloored       * this->width())].b,
                          pixels[((xFloored + 1) + (yFloored + 1) * this->width())].b, yBias);

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

    // Returns the given image coordinates bounds-checked against the image's
    // dimensions. E.g if the image has a resolution of 800 x 600, a y coordinate
    // value of 605 might be returned as 599 (clamped).
    std::tuple<double, double> bounds_checked_coordinates(double x, double y) const
    {
        switch (this->boundsCheckingMode)
        {
            case bounds_checking_mode_e::wrapped: return this->wrapped_coordinates(x, y);
            case bounds_checking_mode_e::clamped: return this->clamped_coordinates(x, y);
        }

        return this->clamped_coordinates(x, y);
    }

    std::tuple<double, double> clamped_coordinates(double x, double y) const
    {
        if (x < 0) x = 0;
        else if (x >= this->width()) x = (this->width() - 1);
        if (y < 0) y = 0;
        else if (y >= this->width()) y = (this->width() - 1);

        return {x, y};
    }

    std::tuple<double, double> wrapped_coordinates(double x, double y) const
    {
        return {((x / (this->width() - 1) - floor(x / (this->width() - 1))) * this->width()),
                ((y / (this->height() - 1) - floor(y / (this->height() - 1))) * this->height())};
    }

    // Returns a copy of this image but with its values converted into the given
    // type.
    template <typename T2>
    image_s<T2> as(const double scale = 1,
                   const double low = 0,
                   const double high = 255) const
    {
        image_s<T2> newImage(this->width(), this->height(), this->bpp());

        for (uint y = 0; y < this->height(); y++)
        {
            for (uint x = 0; x < this->width(); x++)
            {
                const auto thisPixel = this->pixel_at(x, y);

                newImage.pixel_at(x, y) = {T2(std::max(low, std::min(high, double(thisPixel.r * scale)))),
                                           T2(std::max(low, std::min(high, double(thisPixel.g * scale)))),
                                           T2(std::max(low, std::min(high, double(thisPixel.b * scale)))),
                                           T2(255)};
            }
        }

        return newImage;
    }

private:
    const unsigned width_;
    const unsigned height_;
    const unsigned bpp_;
    color_rgba_s<T> *const pixels;

    // Determines how image coordinates that fall outside of the image's dimensions
    // are handled.
    enum class bounds_checking_mode_e
    {
        clamped,
        wrapped,
    } boundsCheckingMode = bounds_checking_mode_e::clamped;
};

#endif
