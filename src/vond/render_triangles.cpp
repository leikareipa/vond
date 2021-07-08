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
#include "vond/rasterize_triangle_barycentric.h"
#include "vond/rasterize_triangle_scanline.h"

#define DEG_TO_RAD(deg) ((deg) * (M_PI / 180.0))

// The near and far clipping planes.
static const double Z_NEAR = 0.1;
static const double Z_FAR = 1000;

static std::vector<vond::triangle> transform_triangles(const std::vector<vond::triangle> &triangles,
                                                       const unsigned screenWidth,
                                                       const unsigned screenHeight,
                                                       const vond::camera &camera)
{
    // Create a matrix by which we can transform the triangles into screen-space.
    vond::matrix44 toWorldSpace;
    {
        const vond::vector3<double> objectPos = {512, 110, 512};
        toWorldSpace = vond::translation_matrix(objectPos.x, objectPos.y, objectPos.z);
    }

    vond::matrix44 toClipSpace;
    {
        const vond::matrix44 cameraMatrix = vond::rotation_matrix(-camera.orientation.x, -camera.orientation.y, camera.orientation.z) *
                                            vond::translation_matrix(-camera.pos.x, -camera.pos.y, -camera.pos.z);

        const vond::matrix44 perspectiveMatrix = vond::perspective_matrix(DEG_TO_RAD(camera.fov),
                                                                          (double(screenWidth) / screenHeight),
                                                                          Z_NEAR, Z_FAR);

        toClipSpace = (perspectiveMatrix * cameraMatrix);
    }

    const vond::matrix44 toScreenSpace = vond::screen_space_matrix((screenWidth / 2.0),
                                                                   (screenHeight / 2.0));


    // Transform the triangles.
    std::vector<vond::triangle> transformedTris;
    {
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

void vond::render_triangles(const std::vector<vond::triangle> &triangles,
                            vond::image<uint8_t, 4> &dstPixelmap,
                            vond::image<double, 1> &dstDepthmap,
                            const vond::camera &camera)
{
    const auto transformedTriangles = transform_triangles(triangles, dstPixelmap.width(), dstPixelmap.height(), camera);

    for (const auto &tri: transformedTriangles)
    {
        vond::rasterize_triangle::barycentric(tri, dstPixelmap, dstDepthmap);
        //vond::rasterize_triangle::scanline(tri, dstPixelmap, dstDepthmap);
    }

    return;
}
