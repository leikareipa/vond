/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * Transforms and rasterizes the given triangles into the given frame buffer.
 *
 */

#include <cmath>
#include "../../src/matrix44.h"
#include "../../src/camera.h"
#include "../../src/render.h"
#include "../../src/image.h"

#define DEG_TO_RAD(deg) ((deg) * (M_PI / 180.0))

// The near and far clipping planes.
static const real Z_NEAR = 0.1;
static const real Z_FAR = 1000;

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
                                                                    (real(screenWidth) / screenHeight),
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

            // Transform to screen space.
            tri.v[0].transform(toScreenSpace);
            tri.v[1].transform(toScreenSpace);
            tri.v[2].transform(toScreenSpace);

            // Perspective division.
            for (uint i = 0; i < 3; i++)
            {
                tri.v[i].pos.x /= tri.v[i].w;
                tri.v[i].pos.y /= tri.v[i].w;
            }

            // Cull triangles that are entirely outside the screen.
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

            transformedTris.push_back(tri);
        }
    }

    return transformedTris;
}

static void rs_fill_tri_row(const uint row,
                            int startX,
                            int endX,
                            real leftU,
                            real leftV,
                            real rightU,
                            real rightV,
                            real leftDepth,
                            real rightDepth,
                            const asset_s<polygon_material_s> &triangleMaterial,
                            image_s<u8> &dstPixelmap,
                            image_s<double> &dstDepthmap)
{
    const real uDelta = ((rightU - leftU) / real((endX - startX) + 1)); // Amount by which to change the u,v coordinates each pixel on the row.
    const real vDelta = ((rightV - leftV) / real((endX - startX) + 1));

    const real depthDelta = ((rightDepth - leftDepth) / real((endX - startX) + 1));

    // Bounds-check, clip against the screen.
    if (startX < 0)
    {
        // Move the parameters accordingly.
        const uint diff = abs(startX);
        leftU += (uDelta * diff);
        leftV += (vDelta * diff);
        leftDepth += (depthDelta * diff);

        startX = 0;
    }
    if (endX >= int(dstPixelmap.width()))
    {
        endX = (dstPixelmap.width() - 1);
    }

    if (endX < startX)
    {
        return;
    }

    uint screenPixIdx = (startX + row * dstPixelmap.width());

    // Solid fill.
    if (!triangleMaterial().texture)
    {
        const color_rgba_s<u8> color = triangleMaterial().baseColor;

        for (int x = startX; x <= endX; x++)
        {
            if (dstDepthmap.pixel_at(x, row).r > leftDepth)
            {
                dstPixelmap.pixel_at(x, row) = color;
                dstDepthmap.pixel_at(x, row) = {leftDepth, leftDepth, leftDepth};
            }

            leftDepth += depthDelta;
            screenPixIdx++;
        }
    }
    // Textured fill.
    else
    {
        for (int x = startX; x <= endX; x++)
        {
            const uint u = (leftU * triangleMaterial().texture->width());
            const uint v = (leftV * triangleMaterial().texture->height());
            const color_rgba_s<u8> color = triangleMaterial().texture->pixel_at(u, v);

            if (dstDepthmap.pixel_at(x, row).r > leftDepth)
            {
                dstPixelmap.pixel_at(x, row) = color;
                dstDepthmap.pixel_at(x, row) = {leftDepth, leftDepth, leftDepth};
            }

            leftU += uDelta;
            leftV += vDelta;
            leftDepth += depthDelta;
            screenPixIdx++;
        }
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
                                const asset_s<polygon_material_s> &triangleMaterial,
                                image_s<u8> &dstPixelmap,
                                image_s<double> &dstDepthmap)
{
    k_assert((base1->pos.y == base2->pos.y),
             "The software triangle filler was given a malformed triangle. Expected a flat base.");

    int startRow, endRow;                   // The top and bottom y coordinates of the triangle.
    int triHeight;
    bool isDownTri = false;                 // Whether the triangle peak is above or below the base, in screen space.

    // We'll fill the triangle in row by row, adjusting the row parameters by
    // a delta per every time we move down one row.
    decltype(peak->pos.x) pixLeft, pixRight, dPixLeft, dPixRight;   // A row of pixels defined by its left and right extrema; and deltas for that row.
    real leftU, leftV, rightU, rightV;      // Texture coordinates for the pixel row.
    real dLeftU, dLeftV, dRightU, dRightV;  // Deltas for texture coordinates.
    decltype(peak->pos.x) leftDepth, rightDepth, dLeftDepth, dRightDepth;

    // Figure out which corner of the base is on the left/right in screen space.
    const vertex4_s *leftVert, *rightVert;
    leftVert = base2;
    rightVert = base1;
    if (base1->pos.x < base2->pos.x)
    {
        leftVert = base1;
        rightVert = base2;
    }

    startRow = peak->pos.y;
    endRow = base1->pos.y;

    // Detect whether the triangle is the top or bottom half of the split.
    if (startRow > endRow)
    {
        const int temp = startRow;
        startRow = endRow;
        endRow = temp;

        // Don't draw the base row twice; i.e. skip it for the down-triangle.
        startRow++;

        isDownTri = 1;
    }
    // If the triangle is very thin vertically, don't bother drawing it.
    else if ((startRow == endRow) ||
             (endRow - startRow) <= 0)
    {
        return;
    }

    triHeight = endRow - startRow;

    // Establish row parameters.
    if (isDownTri)
    {
        pixLeft = (leftVert->pos.x);
        pixRight = (rightVert->pos.x);
        dPixLeft = (peak->pos.x - leftVert->pos.x) / real(triHeight + 1);
        dPixRight = (peak->pos.x - rightVert->pos.x) / real(triHeight + 1);

        leftU = (leftVert->uv[0]);
        leftV = (leftVert->uv[1]);
        rightU = (rightVert->uv[0]);
        rightV = (rightVert->uv[1]);
        dLeftU = (peak->uv[0] - leftVert->uv[0]) / real(triHeight + 1);
        dLeftV = (peak->uv[1] - leftVert->uv[1]) / real(triHeight + 1);
        dRightU = (peak->uv[0] - rightVert->uv[0]) / real(triHeight + 1);
        dRightV = (peak->uv[1] - rightVert->uv[1]) / real(triHeight + 1);

        leftDepth = (leftVert->depth);
        rightDepth = (rightVert->depth);
        dLeftDepth = (peak->depth - leftVert->depth) / (triHeight + 1);
        dRightDepth = (peak->depth - rightVert->depth) / (triHeight + 1);
    }
    else
    {
        pixLeft = (peak->pos.x);
        pixRight = (peak->pos.x);
        dPixLeft = (leftVert->pos.x - peak->pos.x) / real(triHeight + 1);
        dPixRight = (rightVert->pos.x - peak->pos.x) / real(triHeight + 1);

        leftU = (peak->uv[0]);
        leftV = (peak->uv[1]);
        rightU = (peak->uv[0]);
        rightV = (peak->uv[1]);
        dLeftU = (leftVert->uv[0] - peak->uv[0]) / real(triHeight + 1);
        dLeftV = (leftVert->uv[1] - peak->uv[1]) / real(triHeight + 1);
        dRightU = (rightVert->uv[0] - peak->uv[0]) / real(triHeight + 1);
        dRightV = (rightVert->uv[1] - peak->uv[1]) / real(triHeight + 1);

        leftDepth = (peak->depth);
        rightDepth = (peak->depth);
        dLeftDepth = (leftVert->depth - peak->depth) / (triHeight + 1);
        dRightDepth = (rightVert->depth - peak->depth) / (triHeight + 1);
    }

    #define INCREMENT_PARAMS(steps)  pixLeft  += dPixLeft  * steps;\
                                     pixRight += dPixRight * steps;\
                                     leftU    += dLeftU    * steps;\
                                     leftV    += dLeftV    * steps;\
                                     rightU   += dRightU   * steps;\
                                     rightV   += dRightV   * steps;\
                                     leftDepth += dLeftDepth * steps;\
                                     rightDepth+= dRightDepth* steps;

    // Bounds-check to make sure we're not going to draw outside of the screen area
    // horizontally.
    if (startRow < 0)
    {
        const uint steps = abs(startRow);
        INCREMENT_PARAMS(steps)

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
        INCREMENT_PARAMS(1)

        rs_fill_tri_row(row,
                        pixLeft, pixRight,
                        leftU, leftV, rightU, rightV,
                        leftDepth, rightDepth,
                        triangleMaterial,
                        dstPixelmap,
                        dstDepthmap);
    }

    #undef INCREMENT_PARAMS

    return;
}

void kr_rasterize_triangle(const triangle_s &tri,
                           image_s<u8> &dstPixelmap,
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
    const real splitRatio = ((mid->pos.y - high->pos.y) / (real)(low->pos.y - high->pos.y));
    splitBase.pos.x = high->pos.x + ((low->pos.x - high->pos.x) * splitRatio);
    splitBase.pos.y = mid->pos.y;
    splitBase.depth = LERP(high->depth, low->depth, splitRatio);
    splitBase.uv[0] = LERP(high->uv[0], low->uv[0], splitRatio);
    splitBase.uv[1] = LERP(high->uv[1], low->uv[1], splitRatio);

    // Fill the split triangle.
    fill_split_triangle(high, mid, &splitBase, tri.material, dstPixelmap, dstDepthmap);  // Up triangle.
    fill_split_triangle(low, mid, &splitBase, tri.material, dstPixelmap, dstDepthmap);   // Down triangle.

    return;
}

void kr_draw_triangles(const std::vector<triangle_s> &triangles,
                       image_s<u8> &dstPixelmap,
                       image_s<double> &dstDepthmap,
                       const camera_s &camera)
{
    const auto transformedTriangles = transform_triangles(triangles, dstPixelmap.width(), dstPixelmap.height(), camera);

    for (const auto &tri: transformedTriangles)
    {
        kr_rasterize_triangle(tri, dstPixelmap, dstDepthmap);
    }

    return;
}
