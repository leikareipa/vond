/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * Transforms and rasterizes the given triangles into the given frame buffer.
 *
 */

#include <cmath>
#include "vond/matrix.h"
#include "vond/camera.h"
#include "vond/render.h"
#include "vond/image.h"
#include "vond/rasterizer_barycentric.h"
#include "vond/rasterizer_scanline.h".h"

#define DEG_TO_RAD(deg) ((deg) * (M_PI / 180.0))

// The near and far clipping planes.
static const double Z_NEAR = 0.1;
static const double Z_FAR = 1000;

std::vector<triangle_s> transform_triangles(const std::vector<triangle_s> &triangles,
                                            const unsigned screenWidth,
                                            const unsigned screenHeight,
                                            const camera_s &camera)
{
    // Create a matrix by which we can transform the triangles into screen-space.
    matrix44_s toWorldSpace;
    {
        const vector3_s<double> objectPos = {512, 110, 512};
        toWorldSpace = matrix44_translation_s(objectPos.x, objectPos.y, objectPos.z);
    }

    matrix44_s toClipSpace;
    {
        const matrix44_s cameraMatrix = matrix44_rotation_s(-camera.orientation.x, -camera.orientation.y, camera.orientation.z) *
                                        matrix44_translation_s(-camera.pos.x, -camera.pos.y, -camera.pos.z);

        const matrix44_s perspectiveMatrix = matrix44_perspective_s(DEG_TO_RAD(camera.fov),
                                                                    (double(screenWidth) / screenHeight),
                                                                    Z_NEAR, Z_FAR);

        toClipSpace = (perspectiveMatrix * cameraMatrix);
    }

    const matrix44_s toScreenSpace = matrix44_screen_space_s((screenWidth / 2.0),
                                                             (screenHeight / 2.0));


    // Transform the triangles.
    std::vector<triangle_s> transformedTris;
    {
        unsigned idx = 0;
        double vertDepths[3];

        for (auto tri: triangles)
        {
            tri.v[0].transform(toWorldSpace);
            tri.v[1].transform(toWorldSpace);
            tri.v[2].transform(toWorldSpace);

            vertDepths[0] = tri.v[0].pos.distance_to(camera.pos);
            vertDepths[1] = tri.v[1].pos.distance_to(camera.pos);
            vertDepths[2] = tri.v[2].pos.distance_to(camera.pos);

            tri.v[0].transform(toClipSpace);
            tri.v[1].transform(toClipSpace);
            tri.v[2].transform(toClipSpace);

            /// Temp hack. Prevent triangles behind the camera from wigging out.
            if (tri.v[0].w <= 0 ||
                tri.v[1].w <= 0 ||
                tri.v[2].w <= 0)
            {
                continue;
            }

            tri.v[0].transform(toScreenSpace);
            tri.v[1].transform(toScreenSpace);
            tri.v[2].transform(toScreenSpace);

            // Perspective division.
            for (unsigned i = 0; i < 3; i++)
            {
                tri.v[i].pos.x /= tri.v[i].w;
                tri.v[i].pos.y /= tri.v[i].w;
            }

            // Cull triangles that are entirely outside the screen.
            {
                if ((tri.v[0].pos.x < 0 && tri.v[1].pos.x < 0 && tri.v[2].pos.x < 0) ||
                    (tri.v[0].pos.y < 0 && tri.v[1].pos.y < 0 && tri.v[2].pos.y < 0))
                {
                    continue;
                }

                if ((tri.v[0].pos.x >= (int)screenWidth && tri.v[1].pos.x >= (int)screenWidth && tri.v[2].pos.x >= (int)screenWidth) ||
                    (tri.v[0].pos.y >= (int)screenHeight && tri.v[1].pos.y >= (int)screenHeight && tri.v[2].pos.y >= (int)screenHeight))
                {
                    continue;
                }
            }

            tri.v[0].pos.z = vertDepths[0];
            tri.v[1].pos.z = vertDepths[1];
            tri.v[2].pos.z = vertDepths[2];

            transformedTris.push_back(tri);
        }
    }

    return transformedTris;
}

void kr_draw_triangles(const std::vector<triangle_s> &triangles,
                       image_s<uint8_t> &dstPixelmap,
                       image_s<double> &dstDepthmap,
                       const camera_s &camera)
{
    const auto transformedTriangles = transform_triangles(triangles, dstPixelmap.width(), dstPixelmap.height(), camera);

    for (const auto &tri: transformedTriangles)
    {
        kr_barycentric_rasterize_triangle(tri, dstPixelmap, dstDepthmap);
       // kr_scanline_rasterize_triangle(tri, dstPixelmap, dstDepthmap);
    }

    return;
}
