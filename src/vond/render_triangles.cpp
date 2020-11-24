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

#define DEG_TO_RAD(deg) ((deg) * (M_PI / 180.0))

// The near and far clipping planes.
static const double Z_NEAR = 0.1;
static const double Z_FAR = 1000;

struct interpolation_params_s
{
    double startX;
    double u;
    double v;
    double invW;
    double depth;

    // Increments all parameters by the given deltas.
    // Note: Assumes that all params are of type double, and that there are as
    // many left-side params/deltas as right-side params/deltas.
    void increment(const interpolation_params_s &deltas, const unsigned times = 1)
    {
        const unsigned numParams = (sizeof(interpolation_params_s) / sizeof(double));

        double *val = (double*)this;
        double *delta = (double*)&deltas;

        for (unsigned i = 0; i < numParams; i++)
        {
            *val += (*delta * times);

            val++;
            delta++;
        }
    }
};

std::vector<triangle_s> transform_triangles(const std::vector<triangle_s> &triangles,
                                            const unsigned screenWidth,
                                            const unsigned screenHeight,
                                            const camera_s &camera)
{
    // Create a matrix by which we can transform the triangles into screen-space.
    matrix44_s toWorldSpace;
    {
        const vector3_s<double> objectPos = {500, 120, 500};
        toWorldSpace = matrix44_translation_s(objectPos.x, objectPos.y, objectPos.z);
    }

    matrix44_s toClipSpace;
    {
        const matrix44_s cameraMatrix = matrix44_rotation_s(-camera.orientation.x, -camera.orientation.y, camera.orientation.z) *
                                        matrix44_translation_s(-camera.pos.x, -camera.pos.y, -camera.pos.z);

        const matrix44_s perspectiveMatrix = matrix44_perspective_s(DEG_TO_RAD(camera.fov),
                                                                    (double(screenWidth) / screenHeight),
                                                                    Z_NEAR, Z_FAR);

        toClipSpace = perspectiveMatrix * cameraMatrix;
    }

    const matrix44_s toScreenSpace = matrix44_screen_space_s((screenWidth / 2.0),
                                                             (screenHeight / 2.0));


    // Transform the triangles.
    std::vector<triangle_s> transformedTris;
    {
        unsigned idx = 0;

        for (auto tri: triangles)
        {
            tri.v[0].transform(toWorldSpace);
            tri.v[1].transform(toWorldSpace);
            tri.v[2].transform(toWorldSpace);

            tri.v[0].depth = tri.v[0].pos.distance_to(camera.pos);
            tri.v[1].depth = tri.v[1].pos.distance_to(camera.pos);
            tri.v[2].depth = tri.v[2].pos.distance_to(camera.pos);

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

            transformedTris.push_back(tri);
        }
    }

    return transformedTris;
}

static void rs_fill_tri_row(const unsigned row,
                            const interpolation_params_s &rowLeftParams,
                            const interpolation_params_s &rowRightParams,
                            const triangle_material_s &triangleMaterial,
                            image_s<uint8_t> &dstPixelmap,
                            image_s<double> &dstDepthmap)
{
    if (rowRightParams.startX < rowLeftParams.startX)
    {
        return;
    }

    interpolation_params_s currentParams, endingParams, paramDeltas;

    // Establish interpolation parameters.
    {
        currentParams = rowLeftParams;
        endingParams = rowRightParams;

        const double rowWidth = ((rowRightParams.startX - rowLeftParams.startX) + 1);
        const auto make_param_deltas = [=](const auto current, const auto end)
        {
            return ((end - current) / rowWidth);
        };

        paramDeltas.u = make_param_deltas(currentParams.u, endingParams.u);
        paramDeltas.v = make_param_deltas(currentParams.v, endingParams.v);
        paramDeltas.depth = make_param_deltas(currentParams.depth, endingParams.depth);
        paramDeltas.invW = make_param_deltas(currentParams.invW, endingParams.invW);
    }

    // Bounds-check: clip against screen edges.
    {
        if (currentParams.startX < 0)
        {
            const unsigned diff = abs(currentParams.startX);

            currentParams.increment(paramDeltas, diff);
            currentParams.startX = 0;
        }

        if (endingParams.startX >= int(dstPixelmap.width()))
        {
            endingParams.startX = (dstPixelmap.width() - 1);
        }
    }

    // Draw the row.
    for (int x = currentParams.startX; x <= endingParams.startX; x++)
    {
        const double depth = (currentParams.depth / currentParams.invW);

        if (dstDepthmap.pixel_at(x, row).r > depth)
        {
            const double u = ((currentParams.u / currentParams.invW) * triangleMaterial.texture->width());
            const double v = ((currentParams.v / currentParams.invW) * triangleMaterial.texture->height());
            const color_rgba_s<uint8_t> color = triangleMaterial.texture
                                                ? triangleMaterial.texture->pixel_at(u, v)
                                                : triangleMaterial.baseColor;

            dstPixelmap.pixel_at(x, row) = color;
            dstDepthmap.pixel_at(x, row) = {depth, depth, depth};
        }

        currentParams.increment(paramDeltas);
    }

    return;
}

// Pixel-fills the corresponding triangle on screen formed by the three given vertices.
// Expects the vertices to be in screen space, and that the base vertices are level on
// the y axis (you'd first split your triangle along y, then submit the two pieces to
// this function individually).
//
static void fill_split_triangle(const vertex4_s *peak,
                                const vertex4_s *base1,
                                const vertex4_s *base2,
                                const triangle_material_s &triangleMaterial,
                                image_s<uint8_t> &dstPixelmap,
                                image_s<double> &dstDepthmap)
{
    vond_assert((base1->pos.y == base2->pos.y),
             "The software triangle filler was given a malformed triangle. Expected a flat base.");

    // Values to be interpolated vertically as we render the triangle.
    interpolation_params_s paramsLeft, paramsRight, deltasLeft, deltasRight;

    // Whether the triangle peak is above or below the base, in screen space.
    bool isDownTri = false;

    // Figure out which corner of the base is on the left/right in screen space.
    const vertex4_s *leftVert = base2;
    const vertex4_s *rightVert = base1;
    if (base1->pos.x < base2->pos.x)
    {
        leftVert = base1;
        rightVert = base2;
    }

    // The top and bottom y coordinates of the triangle.
    int startRow = peak->pos.y;
    int endRow = base1->pos.y;

    // Detect whether the triangle is the top or bottom half of the split.
    if (startRow > endRow)
    {
        isDownTri = true;

        std::swap(startRow, endRow);

        // Don't draw the base row twice; i.e. skip it for the down-triangle.
        startRow++;
    }
    // If the triangle is very thin vertically, don't bother drawing it.
    else if ((startRow == endRow) ||
             (endRow - startRow) <= 0)
    {
        return;
    }  

    // Establish interpolation parameters.
    {
        const double triHeight = ((endRow - startRow) + 1);
        const auto make_params = [=](const auto start, const auto end, const bool invertParams)
        {
            return invertParams
                   ? std::tuple<double, double>{end, ((start - end) / triHeight)}
                   : std::tuple<double, double>{start, ((end - start) / triHeight)};
        };

        std::tie(paramsLeft.invW, deltasLeft.invW) = make_params((1 / leftVert->w), (1 / peak->w), !isDownTri);
        std::tie(paramsRight.invW, deltasRight.invW) = make_params((1 / rightVert->w), (1 / peak->w), !isDownTri);
        std::tie(paramsLeft.startX, deltasLeft.startX) = make_params(leftVert->pos.x, peak->pos.x, !isDownTri);
        std::tie(paramsRight.startX, deltasRight.startX) = make_params(rightVert->pos.x, peak->pos.x, !isDownTri);
        std::tie(paramsLeft.u, deltasLeft.u) = make_params((leftVert->uv[0] / leftVert->w), (peak->uv[0] / peak->w), !isDownTri);
        std::tie(paramsLeft.v, deltasLeft.v) = make_params((leftVert->uv[1] / leftVert->w), (peak->uv[1] / peak->w), !isDownTri);
        std::tie(paramsRight.u, deltasRight.u) = make_params((rightVert->uv[0] / rightVert->w), (peak->uv[0] / peak->w), !isDownTri);
        std::tie(paramsRight.v, deltasRight.v) = make_params((rightVert->uv[1] / rightVert->w), (peak->uv[1] / peak->w), !isDownTri);
        std::tie(paramsLeft.depth, deltasLeft.depth) = make_params((leftVert->depth / leftVert->w), (peak->depth / peak->w), !isDownTri);
        std::tie(paramsRight.depth, deltasRight.depth) = make_params((rightVert->depth / rightVert->w), (peak->depth / peak->w), !isDownTri);
    }

    // Bounds-check to make sure we're not going to draw outside of the screen area
    // horizontally.
    if (startRow < 0)
    {
        paramsLeft.increment(deltasLeft, abs(startRow));
        paramsRight.increment(deltasRight, abs(startRow));

        startRow = 0;
    }
    if (endRow >= int(dstPixelmap.height()))
    {
        endRow = (dstPixelmap.height() - 1);
    }

    // Iterate over each y row in the triangle, on each row filling in a horizontal line between
    // the left and right edge of the triangle.
    for (int row = startRow; row <= endRow; row++)
    {
        paramsLeft.increment(deltasLeft);
        paramsRight.increment(deltasRight);

        rs_fill_tri_row(row,
                        paramsLeft,
                        paramsRight,
                        triangleMaterial,
                        dstPixelmap,
                        dstDepthmap);
    }

    return;
}

void kr_rasterize_triangle(const triangle_s &tri,
                           image_s<uint8_t> &dstPixelmap,
                           image_s<double> &dstDepthmap)
{
    // Sort the triangle's vertices by height. ('High' here means low y, such that
    // y = 0 is the top of the screen.)
    const vertex4_s *high = &tri.v[0];
    const vertex4_s *mid = &tri.v[1];
    const vertex4_s *low = &tri.v[2];
    if (low->pos.y < mid->pos.y)
    {
        std::swap(low, mid);
    }
    if (mid->pos.y < high->pos.y)
    {
        std::swap(mid, high);
    }
    if (low->pos.y < mid->pos.y)
    {
        std::swap(low, mid);
    }

    // Split the triangle into two parts, one pointing up and the other down.
    // (Split algo from Bastian Molkenthin's www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html.)
    vertex4_s splitBase;
    const double splitRatio = ((mid->pos.y - high->pos.y) / (double)(low->pos.y - high->pos.y));
    splitBase.pos.x = (high->pos.x + ((low->pos.x - high->pos.x) * splitRatio));
    splitBase.pos.y = mid->pos.y;
    splitBase.w = std::lerp(high->w, low->w, splitRatio);
    splitBase.depth = std::lerp(high->depth, low->depth, splitRatio);
    splitBase.uv[0] = std::lerp(high->uv[0], low->uv[0], splitRatio);
    splitBase.uv[1] = std::lerp(high->uv[1], low->uv[1], splitRatio);

    // Fill the split triangle.
    fill_split_triangle(high, mid, &splitBase, tri.material, dstPixelmap, dstDepthmap);  // Up triangle.
    fill_split_triangle(low, mid, &splitBase, tri.material, dstPixelmap, dstDepthmap);   // Down triangle.

    return;
}

void kr_draw_triangles(const std::vector<triangle_s> &triangles,
                       image_s<uint8_t> &dstPixelmap,
                       image_s<double> &dstDepthmap,
                       const camera_s &camera)
{
    const auto transformedTriangles = transform_triangles(triangles, dstPixelmap.width(), dstPixelmap.height(), camera);

    for (const auto &tri: transformedTriangles)
    {
        //Rasterize(tri, dstPixelmap, dstDepthmap);
        kr_rasterize_triangle(tri, dstPixelmap, dstDepthmap);
    }

    return;
}
