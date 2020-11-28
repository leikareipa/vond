/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * Takes in a heightmap image, and renders it into the given frame buffer as a
 * voxel-based landscape.
 *
 */

#include <cstdlib>
#include "vond/matrix.h"
#include "vond/vector.h"
#include "vond/assert.h"
#include "vond/camera.h"
#include "vond/render.h"
#include "vond/image.h"

// The maximum number of steps from the camera that rays are traced. If the ray
// exceeds the bounds of the heightmap, it'll be wrapped around it.
static const unsigned MAX_RAY_LENGTH = 1000000;

static enum class detail_level_e
{
    l0,
    l25,
    l50,
    l75,
    l100
} DETAIL_LEVEL = detail_level_e::l50;

static const double PERSPECTIVE_CORRECTION_DETAIL =
        (DETAIL_LEVEL == detail_level_e::l100)? 0.05
        : (DETAIL_LEVEL == detail_level_e::l75)? 0.1
        : (DETAIL_LEVEL == detail_level_e::l50)? 0.2
        : (DETAIL_LEVEL == detail_level_e::l25)? 0.3
        : 0.4;

static const double RAY_SKIP_MULTIPLIER =
        (DETAIL_LEVEL == detail_level_e::l100)? 0.0001
        : (DETAIL_LEVEL == detail_level_e::l75)? 0.0002
        : (DETAIL_LEVEL == detail_level_e::l50)? 0.0003
        : (DETAIL_LEVEL == detail_level_e::l25)? 0.0004
        : 0.0005;

// Define this to have landscape in the distance rendered in less detail.
//#define REDUCED_DISTANCE_DETAIL

struct ray_s
{
    vector3_s<double> pos;
    vector3_s<double> dir;
};

void kr_draw_landscape(const image_s<double, 1> &srcHeightmap,
                       const image_s<uint8_t, 4> &srcTexture,
                       image_s<uint8_t, 4> &dstPixelmap,
                       image_s<double, 1> &dstDepthmap,
                       const camera_s &camera)
{
    vond_assert((srcHeightmap.width() == srcTexture.width()) &&
             (srcHeightmap.height() == srcTexture.height()),
             "The heightmap must have the same resolution as the texture map.");

    vond_assert((dstPixelmap.width() == dstDepthmap.width()) &&
             (dstPixelmap.height() == dstDepthmap.height()),
             "The pixel map must have the same resolution as the depth map.");

    const double aspectRatio = (dstPixelmap.width() / double(dstPixelmap.height()));
    const double tanFov = tan((camera.fov / 2.0) * (M_PI / 180.0));
    const matrix44_s viewMatrix = (matrix44_rotation_s(0, camera.orientation.y, 0) *
                                   matrix44_rotation_s(camera.orientation.x, 0, 0));

    // Loop through each horizontal slice on the screen.
    #pragma omp parallel for
    for (unsigned x = 0; x < dstPixelmap.width(); x++)
    {
        unsigned stepsTaken = 0;    // How many steps we've traced along the current vertical pixel.
        unsigned rayDepth = 0;      // How many steps the ray has traced into the current horizontal slice.

        const double screenPlaneX = ((2.0 * ((x + 0.5) / dstPixelmap.width()) - 1.0) * tanFov * aspectRatio);
        const double perspectiveCorrection = std::max(PERSPECTIVE_CORRECTION_DETAIL,
                                                      acos(vector3_s<double>{screenPlaneX, 0, camera.zoom}.dot(vector3_s<double>{0, 0, camera.zoom})));

        // For each horizontal slice, shoot a ray toward each of the pixels in its
        // vertical column, starting from the bottom of the screen and working up.
        {
            unsigned y = 0;
            for (; y < dstPixelmap.height(); y++)
            {
                const double screenPlaneY = ((2.0 * ((y + 0.5) / dstPixelmap.height()) - 1.0) * tanFov);

                ray_s ray;

                // Direct the ray toward the current screen pixel, taking into
                // consideration the orientation of the camera.
                {
                    const vector3_s<double> view = (vector3_s<double>{screenPlaneX, screenPlaneY, camera.zoom} * viewMatrix);

                    ray.pos = camera.pos;
                    ray.dir = (view.normalized() * perspectiveCorrection);
                }

                // Move the ray up to where the previous ray terminated.
                {
                    // If the previous ray terminated on its first step, we
                    // assume it's clipping into the terrain. If so, to prevent
                    // artefacting, we let this ray start from the camera's origin.
                    if (stepsTaken == 0)
                    {
                        rayDepth = 0;
                    }

                    ray.pos += (ray.dir * rayDepth);

                    stepsTaken = 0;
                }

                // Follow the ray through the heightmap.
                {
                    // Don't trace rays that are directed upward and above the maximum
                    // height of the terrain.
                    if ((ray.pos.y > 255) && ray.dir.y > 0)
                    {
                        break;
                    }

                    // Find the first voxel that this ray intersects. This will be the
                    // first voxel whose height is greater than the ray's height at that
                    // grid element. Once the ray intersects such a voxel, it'll be drawn
                    // to screen, and tracing for this screen slice ends.
                    for (; rayDepth < (MAX_RAY_LENGTH / perspectiveCorrection); stepsTaken++)
                    {
                        // Don't trace rays that are directed upward and above the maximum
                        // height of the terrain.
                        if ((ray.pos.y > 255) && ray.dir.y > 0)
                        {
                            goto draw_sky;
                        }

                        #ifdef REDUCED_DISTANCE_DETAIL
                            // Make the distance more pixelated.
                            if (rayDepth > 2000 && (x % 2) == 0)
                            {
                                break;
                            }
                        #endif

                        // Get the height of the voxel that's directly below this ray.
                        double voxelHeight = (rayDepth < 500)
                                           ? srcHeightmap.interpolated_pixel_at(ray.pos.x, ray.pos.z)[0]
                                           : srcHeightmap.pixel_at(ray.pos.x, ray.pos.z)[0];

                        // Draw the voxel if the ray intersects it (i.e. if the voxel
                        // is taller than the ray's current height).
                        if (voxelHeight >= ray.pos.y)
                        {
                            const double depth = ray.pos.distance_to(camera.pos);
                            const double distanceFog = std::max(1.0, std::min(2.0, (depth / 2200.0)));

                            color_s<uint8_t, 4> color = (rayDepth < 3000)
                                                        ? srcTexture.interpolated_pixel_at(ray.pos.x, ray.pos.z)
                                                        : srcTexture.pixel_at(ray.pos.x, ray.pos.z);

                            //color.b = std::min(255.0, (color.b * distanceFog));

                            dstPixelmap.pixel_at(x, (dstPixelmap.height() - y - 1)) = color;
                            dstDepthmap.pixel_at(x, (dstDepthmap.height() - y - 1)) = {depth};

                            #ifdef REDUCED_DISTANCE_DETAIL
                                // For reduced resolution, draw this pixel double-wide.
                                if ((rayDepth > 2000) && (x % 2 != 0) && (x < (dstPixelmap.width() - 1)))

                                {
                                    dstPixelmap.pixel_at((x + 1), (dstPixelmap.height() - y - 1)) = color;
                                    dstDepthmap.pixel_at((x + 1), (dstPixelmap.height() - y - 1)) = {depth, depth, depth};
                                }
                            #endif

                            break;
                        }

                        // Move the ray to the next voxel.
                        const unsigned extraSteps = (rayDepth * RAY_SKIP_MULTIPLIER);
                        ray.pos += (ray.dir * (extraSteps + 1));
                        rayDepth += (extraSteps + 1);
                    }
                }
            }

            // Draw the sky for the rest of this screen slice's height.
            draw_sky:
            {
                for (; y < dstPixelmap.height(); y++)
                {
                    ray_s ray;

                    const double screenPlaneY = (2.0 * ((y + 0.5) / dstPixelmap.height()) - 1.0) * tanFov;

                    vector3_s<double> v = vector3_s<double>{screenPlaneX, screenPlaneY, camera.zoom};
                    v *= viewMatrix;

                    // Aim the ray at this pixel.
                    ray.dir = v.normalized();

                    double rayHeight = abs(ray.dir.dot(vector3_s<double>{0, 1, 0}));

                    // The base horizon color when looking directly into the horizon.
                    color_s<uint8_t, 4> horizonColor = {125, 145, 175, 255};

                    int colorAttenuation = std::min(115, (int)floor(190 * rayHeight * 1.1));

                    const color_s<uint8_t, 4> color = {uint8_t(horizonColor[0] - colorAttenuation),
                                                       uint8_t(horizonColor[1] - colorAttenuation),
                                                       uint8_t(horizonColor[2] - colorAttenuation),
                                                       255};

                    const double depth = std::numeric_limits<double>::max();

                    dstPixelmap.pixel_at(x, (dstPixelmap.height() - y - 1)) = color;
                    dstDepthmap.pixel_at(x, (dstDepthmap.height() - y - 1)) = {depth};

                    #ifdef REDUCED_DISTANCE_DETAIL
                        // For reduced resolution, draw this pixel double-wide.
                        if ((rayDepth > 0) && (x % 2 != 0) && (x < (dstPixelmap.width() - 1)))
                        {
                            dstPixelmap.pixel_at((x + 1), (dstPixelmap.height() - y - 1)) = color;
                            dstDepthmap.pixel_at((x + 1), (dstPixelmap.height() - y - 1)) = {depth, depth, depth};
                        }
                    #endif
                }
            }
        }
    }

    return;
}
