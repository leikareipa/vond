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
#include "vond/render.h"
#include "vond/image.h"

// The maximum number of steps from the camera that rays are traced. If the ray
// exceeds the bounds of the heightmap, it'll be wrapped around it.
static const unsigned MAX_RAY_LENGTH = 100000;

static enum class detail_level_e
{
    l0,
    l25,
    l50,
    l75,
    l100
} DETAIL_LEVEL = detail_level_e::l50;

static const double RAY_STEP_SIZE =
        (DETAIL_LEVEL == detail_level_e::l100)? 0.05
        : (DETAIL_LEVEL == detail_level_e::l75)? 0.1
        : (DETAIL_LEVEL == detail_level_e::l50)? 0.2
        : (DETAIL_LEVEL == detail_level_e::l25)? 0.3
        : 0.4;

static const double RAY_SKIP_MULTIPLIER =
        (DETAIL_LEVEL == detail_level_e::l100)? 0.0002
        : (DETAIL_LEVEL == detail_level_e::l75)? 0.0004
        : (DETAIL_LEVEL == detail_level_e::l50)? 0.0006
        : (DETAIL_LEVEL == detail_level_e::l25)? 0.0008
        : 0.001;

struct ray_s
{
    vond::vector3<double> pos;
    vond::vector3<double> dir;
};

void vond::render_landscape(std::function<vond::color<double, 1>(const double x, const double y)> groundHeightmapSampler,
                            std::function<vond::color<uint8_t, 4>(const double x, const double y)> groundTextureSampler,
                            vond::image<uint8_t, 4> &dstPixelmap,
                            vond::image<double, 1> &dstDepthmap,
                            const vond::camera &camera)
{
    vond_assert((dstPixelmap.width() == dstDepthmap.width()) &&
                (dstPixelmap.height() == dstDepthmap.height()),
                "The pixel map must have the same resolution as the depth map.");

    const double aspectRatio = (dstPixelmap.width() / double(dstPixelmap.height()));
    const double tanFov = tan((camera.fov / 2.0) * (M_PI / 180.0));
    const vond::matrix44 viewMatrix = (vond::rotation_matrix(0, camera.orientation.y, 0) *
                                     vond::rotation_matrix(camera.orientation.x, 0, 0));

    // Loop through each horizontal slice on the screen.
    #pragma omp parallel for
    for (unsigned x = 0; x < dstPixelmap.width(); x++)
    {
        unsigned stepsTaken = 0;    // How many steps we've traced along the current vertical pixel.
        unsigned rayDepth = 0;      // How many steps the ray has traced into the current horizontal slice.

        const double screenPlaneX = ((2.0 * ((x + 0.5) / dstPixelmap.width()) - 1.0) * tanFov * aspectRatio);

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
                    const vond::vector3<double> view = (vond::vector3<double>{screenPlaneX, screenPlaneY, camera.zoom} * viewMatrix);

                    ray.pos = camera.pos;
                    ray.dir = (view.normalized() * RAY_STEP_SIZE);
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
                    for (; rayDepth < (MAX_RAY_LENGTH / RAY_STEP_SIZE); stepsTaken++)
                    {
                        // Don't trace rays that are directed upward and above the maximum
                        // height of the terrain.
                        if ((ray.pos.y > 255) && (ray.dir.y > 0))
                        {
                            goto draw_sky;
                        }

                        // Get the height of the voxel that's directly below this ray.
                        double voxelHeight = groundHeightmapSampler(ray.pos.x, ray.pos.z)[0];

                        // Draw the voxel if the ray intersects it (i.e. if the voxel
                        // is taller than the ray's current height).
                        if (voxelHeight >= ray.pos.y)
                        {
                            const vond::color<uint8_t, 4> color = groundTextureSampler(ray.pos.x, ray.pos.z);

                            // If this pixel in the ground texture is fully transparent.
                            if (!color.channel_at(3))
                            {
                                goto draw_sky;
                            }

                            const double depth = ray.pos.distance_to(camera.pos);

                            dstPixelmap.pixel_at(x, (dstPixelmap.height() - y - 1)) = color;
                            dstDepthmap.pixel_at(x, (dstDepthmap.height() - y - 1)) = {depth};

                            break;
                        }
                        
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

                    vond::vector3<double> v = vond::vector3<double>{screenPlaneX, screenPlaneY, camera.zoom};
                    v *= viewMatrix;

                    // Aim the ray at this pixel.
                    ray.dir = v.normalized();

                    double rayHeight = abs(ray.dir.dot(vond::vector3<double>{0, 1, 0}));

                    // The base horizon color when looking directly into the horizon.
                    vond::color<uint8_t, 4> horizonColor = {125, 145, 175, 255};

                    int colorAttenuation = std::min(115, (int)floor(190 * rayHeight * 1.1));

                    const vond::color<uint8_t, 4> color = {uint8_t(horizonColor[0] - colorAttenuation),
                                                           uint8_t(horizonColor[1] - colorAttenuation),
                                                           uint8_t(horizonColor[2] - colorAttenuation),
                                                           255};

                    dstPixelmap.pixel_at(x, (dstPixelmap.height() - y - 1)) = color;
                    dstDepthmap.pixel_at(x, (dstDepthmap.height() - y - 1)) = {std::numeric_limits<double>::max()};
                }
            }
        }
    }

    return;
}
