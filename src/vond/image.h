/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef IMAGE_H
#define IMAGE_H

#include <QImage>
#include <QColor>
#include "vond/vector.h"
#include "vond/color.h"

// Determines how image coordinates that fall outside of the image's dimensions
// are handled.
enum class image_bounds_checking_mode_e
{
    clamped,
    wrapped,
};

template <typename T, size_t NumColorChannels>
struct image_s
{
    image_s(const unsigned width, const unsigned height, const unsigned bpp) :
        width_(width),
        height_(height),
        bpp_(bpp),
        pixels(new color_s<T, NumColorChannels>[width * height])
    {
        vond_assert(((this->width() > 0) &&
                  (this->height() > 0) &&
                  (this->bpp() > 0)),
                 "Invalid image resolution detected.");

        this->fill({0});

        return;
    }

    image_s(const QImage &qImage) :
        image_s(this->from_qimage(qImage))
    {
        return;
    }

    static image_s<T, NumColorChannels> from_qimage(const QImage &qImage)
    {
        vond_assert(!qImage.isNull(), "Was asked to create an image out of a null QImage.");

        image_s<T, NumColorChannels> image(qImage.width(), qImage.height(), qImage.depth());

        // Copy the pixels over.
        for (unsigned y = 0; y < image.height(); y++)
        {
            for (unsigned x = 0; x < image.width(); x++)
            {
                const QColor sourcePixel = qImage.pixel(x, y);
                color_s<T, NumColorChannels> &targetPixel = image.pixel_at(x, y);

                switch (image.bpp())
                {
                    case 8:
                    {
                        for (unsigned i = 0; i < NumColorChannels; i++)
                        {
                            targetPixel[i] = T(sourcePixel.value());
                        }

                        break;
                    }
                    case 32:
                    {
                        color_s<int, 4> c = {sourcePixel.red(),
                                             sourcePixel.green(),
                                             sourcePixel.blue(),
                                             255};

                        for (unsigned i = 0; i < NumColorChannels; i++)
                        {
                            targetPixel[i] = T(c[i % 4]);
                        }

                        break;
                    }
                    default:
                    {
                        vond_assert(0, "Encountered an unknown color depth when converting from a QImage.");
                        break;
                    }
                }
            }
        }

        return image;
    }

    ~image_s(void)
    {
        delete [] pixels;

        return;
    }

    unsigned width(void) const
    {
        return this->width_;
    }

    unsigned height(void) const
    {
        return this->height_;
    }

    unsigned bpp(void) const
    {
        return this->bpp_;
    }

    color_s<T, NumColorChannels>& pixel_at(int x, int y) const
    {
        std::tie(x, y) = this->bounds_checked_coordinates(x, y);

        vond_optional_assert(pixels, "Tried to access the pixels of a null image.");
        vond_optional_assert(((x < this->width()) && (y < this->height())), "Tried to access an image pixel out of bounds.");

        return pixels[(x + y * this->width())];
    }

    color_s<T, NumColorChannels> interpolated_pixel_at(double x, double y) const
    {
        std::tie(x, y) = this->bounds_checked_coordinates(x, y);

        vond_optional_assert(pixels, "Tried to access the pixels of a null image.");
        vond_optional_assert(((x < this->width()) && (y < this->height())), "Tried to access an image pixel out of bounds.");

        unsigned xFloored = floor(x);
        unsigned yFloored = floor(y);
        const double xBias = (x - xFloored);
        const double yBias = (y - yFloored);

        if (xFloored >= (this->width() - 1)) xFloored = (this->width() - 2);
        if (yFloored >= (this->height() - 1)) yFloored = (this->height() - 2);

        color_s<T, NumColorChannels> interpolatedPixel;

        for (unsigned i = 0; i < NumColorChannels; i++)
        {
            const T c1 = std::lerp(pixels[(xFloored       + yFloored       * this->width())][i],
                                   pixels[(xFloored       + (yFloored + 1) * this->width())][i], yBias);
            const T c2 = std::lerp(pixels[((xFloored + 1) + yFloored       * this->width())][i],
                                   pixels[((xFloored + 1) + (yFloored + 1) * this->width())][i], yBias);

            interpolatedPixel[i] = T(std::lerp(c1, c2, xBias));
        }

        return interpolatedPixel;
    }

    const uint8_t* pixel_array(void) const
    {
        return (uint8_t*)pixels;
    }

    void fill_channel(const T fillValue, const unsigned channelIdx)
    {
        vond_assert((channelIdx < NumColorChannels), "Overflowing the color channel.");

        for (unsigned y = 0; y < this->height(); y++)
        {
            for (unsigned x = 0; x < this->width(); x++)
            {
                this->pixel_at(x, y)[channelIdx] = fillValue;
            }
        }

        return;
    }

    void fill(const color_s<T, NumColorChannels> &fillColor)
    {
        for (unsigned y = 0; y < this->height(); y++)
        {
            for (unsigned x = 0; x < this->width(); x++)
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
            case image_bounds_checking_mode_e::wrapped: return this->wrapped_coordinates(x, y);
            case image_bounds_checking_mode_e::clamped: return this->clamped_coordinates(x, y);
        }

        return this->clamped_coordinates(x, y);
    }

    std::tuple<double, double> clamped_coordinates(double x, double y) const
    {
        if (x < 0) x = 0;
        else if (x >= this->width()) x = (this->width() - 1);
        if (y < 0) y = 0;
        else if (y >= this->height()) y = (this->height() - 1);

        return {x, y};
    }

    std::tuple<double, double> wrapped_coordinates(double x, double y) const
    {
        double xw = ((x / (this->width() - 1) - floor(x / (this->width() - 1))) * this->width());
        double yw = ((y / (this->height() - 1) - floor(y / (this->height() - 1))) * this->height());

        return clamped_coordinates(xw, yw);
    }

    // Returns a copy of this image but with its values converted into the given
    // type.
    template <typename T2, size_t NumColorChannels2>
    image_s<T2, NumColorChannels2> as(const double scale = 1,
                                      const double low = 0,
                                      const double high = 255) const
    {
        image_s<T2, NumColorChannels2> newImage(this->width(), this->height(), this->bpp());

        for (unsigned y = 0; y < this->height(); y++)
        {
            for (unsigned x = 0; x < this->width(); x++)
            {
                for (unsigned i = 0; i < NumColorChannels2; i++)
                {
                    const double convertedValue = double(this->pixel_at(x, y)[i % NumColorChannels] * scale);

                    newImage.pixel_at(x, y)[i] = T2(std::max(low, std::min(high, convertedValue)));
                }
            }
        }

        return newImage;
    }

    // Determines how image coordinates that fall outside of the image's dimensions
    // are handled.
    image_bounds_checking_mode_e boundsCheckingMode = image_bounds_checking_mode_e::clamped;

private:
    const unsigned width_;
    const unsigned height_;
    const unsigned bpp_;
    color_s<T, NumColorChannels> *const pixels;
};

#endif
