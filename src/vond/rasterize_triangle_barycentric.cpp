/*
 * 2020 Tarpeeksi Hyvae Soft et al.
 *
 * Software: Vond
 *
 */

#include <cstdlib>
#include "vond/rasterize_triangle_barycentric.h"
#include "vond/triangle.h"
#include "vond/image.h"
#include "vond/rect.h"

// A place to dump pre-computed triangle parameters that're needed when rasterizing
// the triangle.
struct precomputed_params_s
{
    vond::rect<int> boundingRect = {};
    vond::vector3<double> xStep = {};
    vond::vector3<double> yStep = {};
    vond::vector3<double> topLeft = {};
    bool isSubPixelSize = false;
    bool isVisible = true;
};

// Original code by Dmitry V. Sokolov (Tiny Renderer, https://github.com/ssloy/tinyrenderer).
// Superficial modifications in 2020 by Tarpeeksi Hyvae Soft.
static vond::vector3<double> get_barycentric_coords(const vond::triangle &t, const int x, const int y)
{
    const vond::vector3<double> e1 = {(t.v[2].position[0] - t.v[0].position[0]),
                                      (t.v[1].position[0] - t.v[0].position[0]),
                                      (t.v[0].position[0] - x)};

    const vond::vector3<double> e2 = {(t.v[2].position[1] - t.v[0].position[1]),
                                      (t.v[1].position[1] - t.v[0].position[1]),
                                      (t.v[0].position[1] - y)};

    const vond::vector3<double> u = e1.cross(e2);

    // If the triangle is degenerate.
    if (std::abs(u[2]) < 1)
    {
        return {-1, -1, -1};
    }

    const double invZ = (1 / u[2]);

    return {(1 - ((u[0] + u[1]) * invZ)),
            (u[1] * invZ),
            (u[0] * invZ)};
}

// Precomputes and returns for the given triangle and screen rectangle certain
// parameters that will be needed during rendering of the triangle.
static precomputed_params_s get_precomputed_parameters(const vond::triangle &tri,
                                                       const vond::rect<int> &screenRect)
{
    precomputed_params_s params = {};

    params.boundingRect = vond::rect<int>::from_triangle(tri).clipped_against(screenRect);

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
        const vond::vector3<double> bcTopLeft = get_barycentric_coords(tri, params.boundingRect.left(), params.boundingRect.top());
        const vond::vector3<double> bcTopRight = get_barycentric_coords(tri, params.boundingRect.right(), params.boundingRect.top());
        const vond::vector3<double> bcBotLeft = get_barycentric_coords(tri, params.boundingRect.left(), params.boundingRect.bottom());

        const double sX = (1.0 / params.boundingRect.width());
        const double sY = (1.0 / params.boundingRect.height());

        params.xStep = vond::vector3<double>{std::lerp(bcTopLeft[0], bcTopRight[0], sX),
                                             std::lerp(bcTopLeft[1], bcTopRight[1], sX),
                                             std::lerp(bcTopLeft[2], bcTopRight[2], sX)} - bcTopLeft;

        params.yStep = vond::vector3<double>{std::lerp(bcTopLeft[0], bcBotLeft[0], sY),
                                             std::lerp(bcTopLeft[1], bcBotLeft[1], sY),
                                             std::lerp(bcTopLeft[2], bcBotLeft[2], sY)} - bcTopLeft;

        params.topLeft = bcTopLeft;
    }

    return params;
}

void vond::rasterize_triangle::barycentric(const vond::triangle &tri,
                                           vond::image<uint8_t, 4> &dstPixelmap,
                                           vond::image<double, 1> &dstDepthmap)
{
    const precomputed_params_s precomputedParams = get_precomputed_parameters(tri, vond::rect<int>{{0, 0}, {int(dstPixelmap.width()), int(dstPixelmap.height())}});

    if (!precomputedParams.isVisible)
    {
        return;
    }

    // Some triangles get sub-pixel small, so just draw them as single pixels.
    if (precomputedParams.isSubPixelSize)
    {
        double depth = ((tri.v[0].position[2] + tri.v[1].position[2] + tri.v[2].position[2]) / 3.0);

        int x = precomputedParams.boundingRect.left();
        int y = precomputedParams.boundingRect.top();

        if (depth < dstDepthmap.pixel_at(x, y)[0])
        {
            const double u = (((tri.v[0].uv[0] + tri.v[1].uv[0] + tri.v[2].uv[0]) / 3.0) * tri.material.texture->width());
            const double v = (((tri.v[0].uv[1] + tri.v[1].uv[1] + tri.v[2].uv[1]) / 3.0) * tri.material.texture->height());

            dstPixelmap.pixel_at(x, y) = tri.material.texture
                                         ? tri.material.texture->bilinear_sample(u, v)
                                         : tri.material.baseColor;
            dstDepthmap.pixel_at(x, y) = {depth};
        }

        return;
    }

    // Draw all pixels that fall within the triangle's surface.
    {
        #define BARY_INTERPOLATE(TRI_PARAM) ((tri.v[0].TRI_PARAM * xPos[0]) +\
                                             (tri.v[1].TRI_PARAM * xPos[1]) +\
                                             (tri.v[2].TRI_PARAM * xPos[2]))

        vond::vector3<double> xPos = {};
        vond::vector3<double> yPos = (precomputedParams.topLeft - precomputedParams.yStep);

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
                if ((xPos[0] < 0) || (xPos[1] < 0) || (xPos[2] < 0))
                {
                    if (spanDone)
                    {
                        break;
                    }

                    continue;
                }

                spanDone = true;

                const double depth = BARY_INTERPOLATE(position[2]);

                if (depth < dstDepthmap.pixel_at(x, y)[0])
                {
                    const double u = (BARY_INTERPOLATE(uv[0]) * tri.material.texture->width());
                    const double v = (BARY_INTERPOLATE(uv[1]) * tri.material.texture->height());

                    dstPixelmap.pixel_at(x, y) = tri.material.texture->bilinear_sample(u, v);
                    dstDepthmap.pixel_at(x, y) = {depth};
                }
            }
        }

        #undef BARY_INTERPOLATE
    }

    return;
}
