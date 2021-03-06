/*
 * 2020 Tarpeeksi Hyvae Soft et al.
 *
 * Software: Vond
 *
 */

#include <cstdlib>
#include "vond/rasterize_triangle_scanline.h"
#include "vond/triangle.h"
#include "vond/image.h"

// Parameters to be interpolated - during rendering - vertically along triangle
// edges and horizontally along pixel spans.
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

static void fill_triangle_row(const unsigned row,
                              const interpolation_params_s &rowLeftParams,
                              const interpolation_params_s &rowRightParams,
                              const vond::triangle_material &triangleMaterial,
                              vond::image<uint8_t, 4> &dstPixelmap,
                              vond::image<double, 1> &dstDepthmap)
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

        if (dstDepthmap.pixel_at(x, row)[0] > depth)
        {
            const double u = ((currentParams.u / currentParams.invW) * triangleMaterial.texture->width());
            const double v = ((currentParams.v / currentParams.invW) * triangleMaterial.texture->height());
            const vond::color<uint8_t, 4> color = triangleMaterial.texture
                                          ? triangleMaterial.texture->bilinear_sample(u, v)
                                          : triangleMaterial.baseColor;

            dstPixelmap.pixel_at(x, row) = color;
            dstDepthmap.pixel_at(x, row) = {depth};
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
static void fill_split_triangle(const vond::vertex *peak,
                                const vond::vertex *base1,
                                const vond::vertex *base2,
                                const vond::triangle_material &triangleMaterial,
                                vond::image<uint8_t, 4> &dstPixelmap,
                                vond::image<double, 1> &dstDepthmap)
{
    vond_assert((base1->position[1] == base2->position[1]),
             "The software triangle filler was given a malformed triangle. Expected a flat base.");

    // Values to be interpolated vertically as we render the triangle.
    interpolation_params_s paramsLeft, paramsRight, deltasLeft, deltasRight;

    // Whether the triangle peak is above or below the base, in screen space.
    bool isDownTri = false;

    // Figure out which corner of the base is on the left/right in screen space.
    const vond::vertex *leftVert = base2;
    const vond::vertex *rightVert = base1;
    if (base1->position[0] < base2->position[0])
    {
        leftVert = base1;
        rightVert = base2;
    }

    // The top and bottom y coordinates of the triangle.
    int startRow = peak->position[1];
    int endRow = base1->position[1];

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
        std::tie(paramsLeft.startX, deltasLeft.startX) = make_params(leftVert->position[0], peak->position[0], !isDownTri);
        std::tie(paramsRight.startX, deltasRight.startX) = make_params(rightVert->position[0], peak->position[0], !isDownTri);
        std::tie(paramsLeft.u, deltasLeft.u) = make_params((leftVert->uv[0] / leftVert->w), (peak->uv[0] / peak->w), !isDownTri);
        std::tie(paramsLeft.v, deltasLeft.v) = make_params((leftVert->uv[1] / leftVert->w), (peak->uv[1] / peak->w), !isDownTri);
        std::tie(paramsRight.u, deltasRight.u) = make_params((rightVert->uv[0] / rightVert->w), (peak->uv[0] / peak->w), !isDownTri);
        std::tie(paramsRight.v, deltasRight.v) = make_params((rightVert->uv[1] / rightVert->w), (peak->uv[1] / peak->w), !isDownTri);
        std::tie(paramsLeft.depth, deltasLeft.depth) = make_params((leftVert->position[2] / leftVert->w), (peak->position[2] / peak->w), !isDownTri);
        std::tie(paramsRight.depth, deltasRight.depth) = make_params((rightVert->position[2] / rightVert->w), (peak->position[2] / peak->w), !isDownTri);
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

void vond::rasterize_triangle::scanline(const vond::triangle &tri,
                                        vond::image<uint8_t, 4> &dstPixelmap,
                                        vond::image<double, 1> &dstDepthmap)
{
    // Sort the triangle's vertices by height. ('High' here means low y, such that
    // y = 0 is the top of the screen.)
    const vond::vertex *high = &tri.v[0];
    const vond::vertex *mid = &tri.v[1];
    const vond::vertex *low = &tri.v[2];
    if (low->position[1] < mid->position[1])
    {
        std::swap(low, mid);
    }
    if (mid->position[1] < high->position[1])
    {
        std::swap(mid, high);
    }
    if (low->position[1] < mid->position[1])
    {
        std::swap(low, mid);
    }

    // Split the triangle into two parts, one pointing up and the other down.
    // (Split algo from Bastian Molkenthin's www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html.)
    vond::vertex splitBase;
    const double splitRatio = ((mid->position[1] - high->position[1]) / (double)(low->position[1] - high->position[1]));
    splitBase.position[0] = (high->position[0] + ((low->position[0] - high->position[0]) * splitRatio));
    splitBase.position[1] = mid->position[1];
    splitBase.position[2] = std::lerp(high->position[2], low->position[2], splitRatio);
    splitBase.w = std::lerp(high->w, low->w, splitRatio);
    splitBase.uv[0] = std::lerp(high->uv[0], low->uv[0], splitRatio);
    splitBase.uv[1] = std::lerp(high->uv[1], low->uv[1], splitRatio);

    // Fill in the up and down triangles, respectively.
    fill_split_triangle(high, mid, &splitBase, tri.material, dstPixelmap, dstDepthmap);
    fill_split_triangle(low, mid, &splitBase, tri.material, dstPixelmap, dstDepthmap);

    return;
}
