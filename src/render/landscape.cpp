/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * Takes in a heightmap image, and renders it into the given frame buffer as a
 * voxel-based landscape.
 *
 */

#include <cstdlib>
#include "../../src/matrix44.h"
#include "../../src/vector.h"
#include "../../src/memory.h"
#include "../../src/common.h"
#include "../../src/camera.h"
#include "../../src/render.h"
#include "../../src/image.h"

// The maximum number of steps from the camera that rays are traced. If the ray
// exceeds the bounds of the heightmap, it'll be wrapped around it.
static const uint MAX_RAY_LENGTH = 1000000;

static enum class detail_level_e
{
    l0,
    l25,
    l50,
    l75,
    l100
} DETAIL_LEVEL = detail_level_e::l50;

static const real PERSPECTIVE_CORRECTION_DETAIL =
        (DETAIL_LEVEL == detail_level_e::l100)? 0.05
        : (DETAIL_LEVEL == detail_level_e::l75)? 0.1
        : (DETAIL_LEVEL == detail_level_e::l50)? 0.2
        : (DETAIL_LEVEL == detail_level_e::l25)? 0.3
        : 0.4;

static const real RAY_SKIP_MULTIPLIER =
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

void kr_draw_landscape(const image_s &heightmap,
                       const image_s &texture,
                       const camera_s &camera,
                       framebuffer_s *const frameBuffer)
{
    k_assert(frameBuffer != nullptr,
             "Was asked to draw the landscape into a null pixel buffer.");
    k_assert(heightmap.resolution().same_width_height_as(texture.resolution()),
             "The heightmap must have the same resolution as the texture map.");

    const real aspectRatio = (frameBuffer->width() / real(frameBuffer->height()));
    const real tanFov = tan((camera.fov / 2.0) * (M_PI / 180.0));
    const matrix44_s viewMatrix = (matrix44_rotation_s(0, camera.orientation.y, 0) *
                                   matrix44_rotation_s(camera.orientation.x, 0, 0));

    // Loop through each horizontal slice on the screen.
    #pragma omp parallel for
    for (uint x = 0; x < frameBuffer->width(); x++)
    {
        uint stepsTaken = 0;    // How many steps we've traced along the current vertical pixel.
        uint rayDepth = 0;      // How many steps the ray has traced into the current horizontal slice.

        const real screenPlaneX = ((2.0 * ((x + 0.5) / frameBuffer->width()) - 1.0) * tanFov * aspectRatio);
        const real perspectiveCorrection = std::max(PERSPECTIVE_CORRECTION_DETAIL,
                                                    acos(vector3_s<double>{screenPlaneX, 0, camera.zoom}.dot(vector3_s<double>{0, 0, camera.zoom})));

        // For each horizontal slice, shoot a ray toward each of the pixels in its
        // vertical column, starting from the bottom of the screen and working up.
        {
            uint y = 0;
            for (; y < frameBuffer->height(); y++)
            {
                const real screenPlaneY = ((2.0 * ((y + 0.5) / frameBuffer->height()) - 1.0) * tanFov);

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

                    // Wrap the ray around the edges of the heightmap, if needed.
                    ray.pos.x = ((ray.pos.x / heightmap.width() - floor(ray.pos.x / heightmap.width())) * heightmap.width());
                    ray.pos.z = ((ray.pos.z / heightmap.height() - floor(ray.pos.z / heightmap.height())) * heightmap.height());

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

                        // Wrap rays around the map at the edges, so we get an infinity-
                        // like effect.
                        if      (ray.pos.z < 1)                         ray.pos.z = (heightmap.height() - 2);
                        else if (ray.pos.z >= (heightmap.height() - 1)) ray.pos.z = 1;
                        if      (ray.pos.x < 1)                         ray.pos.x = (heightmap.width() - 2);
                        else if (ray.pos.x >= (heightmap.width() - 1))  ray.pos.x = 1;

                        // Get the height of the voxel that's directly below this ray.
                        real voxelHeight = (rayDepth < 500)
                                           ? heightmap.interpolated_float_pixel_at(ray.pos.x, ray.pos.z).x
                                           : heightmap.pixel_at(ray.pos.x, ray.pos.z).r;

                        // Draw the voxel if the ray intersects it (i.e. if the voxel
                        // is taller than the ray's current height).
                        if (voxelHeight >= ray.pos.y)
                        {
                            color_rgba_s color = (rayDepth < 1500)
                                                 ? texture.interpolated_pixel_at(ray.pos.x, ray.pos.z)
                                                 : texture.pixel_at(ray.pos.x, ray.pos.z);

                            frameBuffer->canvas->pixel_at(x, frameBuffer->height() - y - 1) = color;
                            frameBuffer->depthmap[x + (frameBuffer->height() - y - 1) * frameBuffer->width()] = rayDepth;

                            #ifdef REDUCED_DISTANCE_DETAIL
                                // For reduced resolution, draw this pixel double-wide.
                                if ((rayDepth > 2000) && (x % 2 != 0) && (x < (frameBuffer->width() - 1)))
                                {
                                    frameBuffer->canvas->pixel_at((x + 1), frameBuffer->height() - y - 1) = color;
                                    frameBuffer->depthmap[(x + 1) + (frameBuffer->height() - y - 1) * frameBuffer->width()] = rayDepth;
                                }
                            #endif

                            break;
                        }

                        // Move the ray to the next voxel.
                        const uint extraSteps = (rayDepth * RAY_SKIP_MULTIPLIER);
                        ray.pos += (ray.dir * (extraSteps + 1));
                        rayDepth += (extraSteps + 1);
                    }
                }
            }

            // Draw the sky for the rest of this screen slice's height.
            draw_sky:
            {
                for (; y < frameBuffer->height(); y++)
                {
                    ray_s ray;

                    const real screenPlaneY = (2.0 * ((y + 0.5) / frameBuffer->height()) - 1.0) * tanFov;

                    vector3_s<double> v = vector3_s<double>{screenPlaneX, screenPlaneY, camera.zoom};
                    v *= viewMatrix;

                    // Aim the ray at this pixel.
                    ray.dir = v.normalized();

                    real rayHeight = abs(ray.dir.dot(vector3_s<double>{0, 1, 0}));

                    // The base horizon color when looking directly into the horizon.
                    color_rgba_s horizonColor = {125, 145, 175, 255};

                    // The amount by which to scale the base horizon color when
                    // taking into consideration the direction of the camera.
                    u8 scaledColor = floor(100 * rayHeight*1.1);
                    if (scaledColor > 115) scaledColor = 115;
                    color_rgba_s scaledHorizonColor = {scaledColor, scaledColor, scaledColor, 255};

                    color_rgba_s c = {u8(horizonColor.r - scaledHorizonColor.r),
                                      u8(horizonColor.g - scaledHorizonColor.g),
                                      u8(horizonColor.b - scaledHorizonColor.b),
                                      255};

                    frameBuffer->canvas->pixel_at(x, frameBuffer->height() - y - 1) = c;
                    frameBuffer->depthmap[x + (frameBuffer->height() - y - 1) * frameBuffer->width()] = ~0u;

                    #ifdef REDUCED_DISTANCE_DETAIL
                        // For reduced resolution, draw this pixel double-wide.
                        if ((rayDepth > 0) && (x % 2 != 0) && (x < (frameBuffer->width() - 1)))
                        {
                            frameBuffer->canvas->pixel_at((x + 1), frameBuffer->height() - y - 1) = c;
                            frameBuffer->depthmap[(x + 1) + (frameBuffer->height() - y - 1) * frameBuffer->width()] = ~0u;
                        }
                    #endif
                }
            }
        }
    }

    return;
}
