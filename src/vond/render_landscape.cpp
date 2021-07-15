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
#include "vond/image.h"
#include "vond/render_landscape.h"

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

static const unsigned PIXEL_WIDTH_MULTIPLIER = 1;

struct ray_s
{
    vond::vector3<double> pos;
    vond::vector3<double> dir;
};

void vond::render_landscape(std::function<vond::color<double, 1>(const double x, const double y)> heightmapSampler,
                            std::function<vond::color<uint8_t, 4>(const double x, const double y)> textureSampler,
                            std::function<vond::color<uint8_t, 4>(const vond::vector3<double> &direction)> skySampler,
                            vond::image<uint8_t, 4> &dstPixelmap,
                            vond::image<double, 1> &dstDepthmap,
                            const vond::camera &camera)
{
    vond_assert((dstPixelmap.width() == dstDepthmap.width()) &&
                (dstPixelmap.height() == dstDepthmap.height()),
                "The pixel map must have the same resolution as the depth map.");

    const double aspectRatio = (dstPixelmap.width() / double(dstPixelmap.height()));
    const double tanFov = tan((camera.fov / 2.0) * (M_PI / 180.0));
    const vond::matrix44 viewMatrix = (vond::rotation_matrix(0, camera.orientation[1], 0) *
                                       vond::rotation_matrix(camera.orientation[0], 0, 0));

    // Loop through each horizontal slice on the screen.
    #pragma omp parallel for
    for (unsigned x = 0; x < dstPixelmap.width(); x += PIXEL_WIDTH_MULTIPLIER)
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

                    ray.pos = camera.position;
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
                    if ((ray.pos[1] > 255) && (ray.dir[1] >= 0))
                    {
                        break;
                    }

                    // Find the first voxel that this ray intersects. This will be the
                    // first voxel whose height is greater than the ray's height at that
                    // grid element. Once the ray intersects such a voxel, it'll be drawn
                    // to screen, and tracing for this screen slice ends.
                    for (; rayDepth < (MAX_RAY_LENGTH / RAY_STEP_SIZE); stepsTaken++)
                    {
                        // Get the height of the voxel that's directly below this ray.
                        double voxelHeight = heightmapSampler(ray.pos[0], ray.pos[2]).channel_at(0);

                        // Draw the voxel if the ray intersects it (i.e. if the voxel
                        // is taller than the ray's current height).
                        if (voxelHeight >= ray.pos[1])
                        {
                            const vond::color<uint8_t, 4> color = textureSampler(ray.pos[0], ray.pos[2]);

                            // If this pixel in the ground texture is fully transparent.
                            if (!color.channel_at(3))
                            {
                                goto draw_sky;
                            }

                            const double depth = ray.pos.distance_to(camera.position);

                            for (unsigned i = 0; i < PIXEL_WIDTH_MULTIPLIER; i++)
                            {
                                dstPixelmap.pixel_at((x + i), (dstPixelmap.height() - y - 1)) = color;
                                dstDepthmap.pixel_at((x + i), (dstDepthmap.height() - y - 1)) = {depth};
                            }

                            break;
                        }

                        const unsigned extraSteps = (rayDepth * RAY_SKIP_MULTIPLIER);
                        ray.pos += (ray.dir * (extraSteps + 1));
                        rayDepth += (extraSteps + 1);

                        // Don't trace rays that are directed upward and above the maximum
                        // height of the terrain.
                        if ((ray.pos[1] > 255) && (ray.dir[1] >= 0))
                        {
                            goto draw_sky;
                        }
                    }
                }
            }

            // Draw the sky for the rest of this screen slice's height.
            draw_sky:
            {
                // Kludge fix for there sometimes being 1 pixel thin holes between the terrain and the sky.
                if ((y > 0) && (y < (dstPixelmap.height() - 1)))
                {
                    y--;
                }

                for (; y < dstPixelmap.height(); y++)
                {
                    const double screenPlaneY = (2.0 * ((y + 0.5) / dstPixelmap.height()) - 1.0) * tanFov;
                    const auto rayDirection = (vond::vector3<double>{screenPlaneX, screenPlaneY, camera.zoom} * viewMatrix);
                    const vond::color<uint8_t, 4> color = skySampler(rayDirection.normalized());

                    for (unsigned i = 0; i < PIXEL_WIDTH_MULTIPLIER; i++)
                    {
                        dstPixelmap.pixel_at((x + i), (dstPixelmap.height() - y - 1)) = color;
                        dstDepthmap.pixel_at((x + i), (dstDepthmap.height() - y - 1)) = {std::numeric_limits<double>::max()};
                    }
                }
            }
        }
    }

    return;
}
