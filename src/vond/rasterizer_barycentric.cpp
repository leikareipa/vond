/*
 * 2020 Tarpeeksi Hyvae Soft et al.
 *
 * Software: Vond
 *
 */

#include <cstdlib>
#include "vond/rasterizer_barycentric.h"
#include "vond/triangle.h"
#include "vond/image.h"
#include "vond/rect.h"

// Pre-computed triangle, needed during rendering of the triangle.
struct precomputed_params_s
{
    rect_s<int> boundingRect = {};
    vector3_s<double> xStep = {};
    vector3_s<double> yStep = {};
    vector3_s<double> topLeft;
    bool isSubPixelSize = false;
    bool isVisible = true;
};

// Original code by Dmitry V. Sokolov (Tiny Renderer, https://github.com/ssloy/tinyrenderer).
// Superficial modifications in 2020 by Tarpeeksi Hyvae Soft.
static vector3_s<double> get_barycentric_coords(const triangle_s &t, const int x, const int y)
{
    const vector3_s<double> e1 = {(t.v[2].pos.x - t.v[0].pos.x),
                                  (t.v[1].pos.x - t.v[0].pos.x),
                                  (t.v[0].pos.x - x)};

    const vector3_s<double> e2 = {(t.v[2].pos.y - t.v[0].pos.y),
                                  (t.v[1].pos.y - t.v[0].pos.y),
                                  (t.v[0].pos.y - y)};

    const vector3_s<double> u = e1.cross(e2);

    // If the triangle is degenerate.
    if (std::abs(u.z) < 1)
    {
        return {-1, -1, -1};
    }

    const double invZ = (1 / u.z);

    return {(1 - ((u.x + u.y) * invZ)),
            (u.y * invZ),
            (u.x * invZ)};
}

// Precomputes and returns for the given triangle and screen rectangle certain
// parameters that will be needed during rendering of the triangle.
static precomputed_params_s get_precomputed_parameters(const triangle_s &tri,
                                                       const rect_s<int> &screenRect)
{
    precomputed_params_s params = {};

    params.boundingRect = rect_s<int>::from_triangle(tri).clipped_against(screenRect);

    params.isVisible = ((params.boundingRect.width() > 0) &&
                        (params.boundingRect.height() > 0));

    if (!params.isVisible)
    {
        return params;
    }

    params.isSubPixelSize = ((params.boundingRect.width() == 1) &&
                             (params.boundingRect.height() == 1));

    // Interpolate barycentric coordinates over the triangle's bounding box.
    {
        const vector3_s<double> bcTopLeft = get_barycentric_coords(tri, params.boundingRect.left(), params.boundingRect.top());
        const vector3_s<double> bcTopRight = get_barycentric_coords(tri, params.boundingRect.right(), params.boundingRect.top());
        const vector3_s<double> bcBotLeft = get_barycentric_coords(tri, params.boundingRect.left(), params.boundingRect.bottom());

        const double sX = (1.0 / params.boundingRect.width());
        const double sY = (1.0 / params.boundingRect.height());

        params.xStep = vector3_s<double>{std::lerp(bcTopLeft.x, bcTopRight.x, sX),
                                         std::lerp(bcTopLeft.y, bcTopRight.y, sX),
                                         std::lerp(bcTopLeft.z, bcTopRight.z, sX)} - bcTopLeft;

        params.yStep = vector3_s<double>{std::lerp(bcTopLeft.x, bcBotLeft.x, sY),
                                         std::lerp(bcTopLeft.y, bcBotLeft.y, sY),
                                         std::lerp(bcTopLeft.z, bcBotLeft.z, sY)} - bcTopLeft;

        params.topLeft = bcTopLeft;
    }

    return params;
}

void kr_barycentric_rasterize_triangle(const triangle_s &tri,
                                       image_s<uint8_t> &dstPixelmap,
                                       image_s<double> &dstDepthmap)
{
    const precomputed_params_s precomputedParams = get_precomputed_parameters(tri, rect_s<int>{{0, 0}, {int(dstPixelmap.width()), int(dstPixelmap.height())}});

    if (!precomputedParams.isVisible)
    {
        return;
    }

    // Some triangles get sub-pixel small, so just draw them as single pixels.
    if (precomputedParams.isSubPixelSize)
    {
        double z = (tri.v[0].pos.z + tri.v[1].pos.z + tri.v[2].pos.z) / 3;

        int x = precomputedParams.boundingRect.left();
        int y = precomputedParams.boundingRect.top();

        if (z < dstDepthmap.pixel_at(x, y).r)
        {
            dstPixelmap.pixel_at(x, y) = {255, 255, 0};
            dstDepthmap.pixel_at(x, y) = {0};
        }

        return;
    }

    // Draw all pixels that fall within the triangle's surface.
    {
        #define BARY_INTERPOLATE(TRI_PARAM) ((tri.v[0].TRI_PARAM * xPos.x) +\
                                             (tri.v[1].TRI_PARAM * xPos.y) +\
                                             (tri.v[2].TRI_PARAM * xPos.z))

        vector3_s<double> xPos = {};
        vector3_s<double> yPos = (precomputedParams.topLeft - precomputedParams.yStep);

        for (int y = precomputedParams.boundingRect.top(); y <= precomputedParams.boundingRect.bottom(); y++)
        {
            yPos += precomputedParams.yStep;

            // For early stopping.
            bool spanDone = false;

            xPos = (yPos - precomputedParams.xStep);

            for (int x = precomputedParams.boundingRect.left(); x <= precomputedParams.boundingRect.right(); x++)
            {
                xPos += precomputedParams.xStep;

                // If this pixel isn't inside the triangle.
                if ((xPos.x < 0) || (xPos.y < 0) || (xPos.z < 0))
                {
                    if (spanDone)
                    {
                        break;
                    }

                    continue;
                }

                spanDone = true;

                const double depth = BARY_INTERPOLATE(pos.z);

                if (depth < dstDepthmap.pixel_at(x, y).r)
                {
                    const double u = (BARY_INTERPOLATE(uv[0]) * tri.material.texture->width());
                    const double v = (BARY_INTERPOLATE(uv[1]) * tri.material.texture->height());

                    dstPixelmap.pixel_at(x, y) = tri.material.texture->interpolated_pixel_at(u, v);
                    dstDepthmap.pixel_at(x, y) = {depth, depth, depth};
                }
            }
        }

        #undef BARY_INTERPOLATE
    }

    return;
}
