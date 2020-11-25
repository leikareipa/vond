/*
 * 2020 Tarpeeksi Hyvae Soft et al.
 *
 * Software: Vond
 *
 */

#include <cstdlib>
#include "vond/rasterizer_scanline.h"
#include "vond/triangle.h"
#include "vond/image.h"

static void fill_triangle_row(const unsigned row,
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
                                                ? triangleMaterial.texture->interpolated_pixel_at(u, v)
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
static void fill_split_triangle(const vertex_s *peak,
                                const vertex_s *base1,
                                const vertex_s *base2,
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
    const vertex_s *leftVert = base2;
    const vertex_s *rightVert = base1;
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
        std::tie(paramsLeft.depth, deltasLeft.depth) = make_params((leftVert->pos.z / leftVert->w), (peak->pos.z / peak->w), !isDownTri);
        std::tie(paramsRight.depth, deltasRight.depth) = make_params((rightVert->pos.z / rightVert->w), (peak->pos.z / peak->w), !isDownTri);
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

        fill_triangle_row(row,
                        paramsLeft,
                        paramsRight,
                        triangleMaterial,
                        dstPixelmap,
                        dstDepthmap);
    }

    return;
}

void kr_scanline_rasterize_triangle(const triangle_s &tri,
                                    image_s<uint8_t> &dstPixelmap,
                                    image_s<double> &dstDepthmap)
{
    // Sort the triangle's vertices by height. ('High' here means low y, such that
    // y = 0 is the top of the screen.)
    const vertex_s *high = &tri.v[0];
    const vertex_s *mid = &tri.v[1];
    const vertex_s *low = &tri.v[2];
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
    vertex_s splitBase;
    const double splitRatio = ((mid->pos.y - high->pos.y) / (double)(low->pos.y - high->pos.y));
    splitBase.pos.x = (high->pos.x + ((low->pos.x - high->pos.x) * splitRatio));
    splitBase.pos.y = mid->pos.y;
    splitBase.pos.z = std::lerp(high->pos.z, low->pos.z, splitRatio);
    splitBase.w = std::lerp(high->w, low->w, splitRatio);
    splitBase.uv[0] = std::lerp(high->uv[0], low->uv[0], splitRatio);
    splitBase.uv[1] = std::lerp(high->uv[1], low->uv[1], splitRatio);

    // Fill in the up and down triangles, respectively.
    fill_split_triangle(high, mid, &splitBase, tri.material, dstPixelmap, dstDepthmap);
    fill_split_triangle(low, mid, &splitBase, tri.material, dstPixelmap, dstDepthmap);

    return;
}
