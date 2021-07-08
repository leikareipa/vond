/*
 * 2020 Tarpeeksi Hyvae Soft
 *
 * Software: Vond
 *
 */

#ifndef VOND_IMAGE_MOSAIC_H
#define VOND_IMAGE_MOSAIC_H

#include <array>
#include <cstring>
#include <unordered_map>
#include "vond/image.h"

namespace vond
{
    // A collection of vond::image<> instances, representing tiles of a larger image (the
    // mosaic). A mosaic consists is n-by-m tiles, all tiles having the same resolution.
    template <typename T, unsigned NumImageColorChannels>
    class image_mosaic
    {
    public:
        image_mosaic(const unsigned numTilesX, const unsigned numTilesY, const unsigned tileWidth, const unsigned tileHeight) :
            numTilesX(numTilesX),
            numTilesY(numTilesY),
            tileWidth(tileWidth),
            tileHeight(tileHeight),
            tiles(new const vond::image<T, NumImageColorChannels>*[numTilesX * numTilesY])
        {
            std::fill(this->tiles, (this->tiles + (numTilesX * numTilesY)), nullptr);

            return;
        }

        ~image_mosaic(void)
        {
            delete [] this->tiles;
        }


        void insert_at(const unsigned tilePosX, const unsigned tilePosY, const vond::image<T, NumImageColorChannels> *const image)
        {
            vond_assert((image->width() == this->tileWidth) &&
                        (image->height() == this->tileHeight),
                        "The image has incorrect dimensions for this mosaic.");

            vond_assert((tilePosX < numTilesX) &&
                        (tilePosY < numTilesY),
                        "Mosaic insert overflowing.");

            this->tiles[tilePosX + tilePosY * numTilesX] = image;

            return;
        }

        vond::color<T, NumImageColorChannels>& pixel_at(int x, int y) const
        {
            const unsigned tileX = (x / this->tileWidth);
            const unsigned tileY = (y / this->tileHeight);

            vond_optional_assert((tileX < this->numTilesX) &
                                (tileY < this->numTilesY),
                                "Mosaic overflowing on pixel query.");

            return this->tiles[tileX + tileY * this->numTilesX]->pixel_at((x - (tileX * this->tileWidth)), (y - (tileY * this->tileHeight)));
        }

        vond::color<T, NumImageColorChannels> interpolated_pixel_at(double x, double y) const
        {
            const unsigned tileX = (x / this->tileWidth);
            const unsigned tileY = (y / this->tileHeight);

            vond_optional_assert((tileX < this->numTilesX) &
                                (tileY < this->numTilesY),
                                "Mosaic overflowing on pixel query.");

            const vond::image<T, NumImageColorChannels> &image = *this->tiles[tileX + tileY * this->numTilesX];

            // FIXME: Only sampling one image will produce edge artefacts.
            return image.interpolated_pixel_at((x - (tileX * this->tileWidth)), (y - (tileY * this->tileHeight)));
        }

        // Returns true if the mosaic contains pixel data at the given XY coordinates;
        // false otherwise.
        bool coordinates_have_data(const double x, const double y) const
        {
            const unsigned tileX = (x / this->tileWidth);
            const unsigned tileY = (y / this->tileHeight);
            const unsigned idx = (tileX + tileY * this->numTilesX);

            if ((tileX > this->numTilesX) ||
                (tileY > this->numTilesY) ||
                !tiles[idx])
            {
                return false;
            }

            return true;
        }

        // Returns true if the given XY coordinates are outside of the mosaic's borders;
        // false otherwise.
        bool coordinates_out_of_range(const double x, const double y) const
        {
            if ((x < 0) ||
                (y < 0) ||
                (x >= (numTilesX * tileWidth)) ||
                (y >= (numTilesY * tileHeight)))
            {
                return true;
            }

            return false;
        }

        std::tuple<double, double> clamped_coordinates(double x, double y) const
        {
            const unsigned maxX = (numTilesX * this->tileWidth);
            const unsigned maxY = (numTilesY * this->tileHeight);

            if (x < 0) x = 0;
            else if (x >= maxX) x = (maxX - 1);
            if (y < 0) y = 0;
            else if (y >= maxY) y = (maxY - 1);

            return {x, y};
        }

    private:
        const unsigned numTilesX;
        const unsigned numTilesY;
        const unsigned tileWidth;
        const unsigned tileHeight;
        const vond::image<T, NumImageColorChannels> **tiles;
    };
}

#endif
