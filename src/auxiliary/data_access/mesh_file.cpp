/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * Parses a mesh config file to extract the text-based 3d mesh data (vertices,
 * uv coordinates, etc.) contained therein.
 *
 */

#include <unordered_map>
#include <fstream>
#include <vector>
#include <regex>
#include "auxiliary/data_access/mesh_file.h"
#include "auxiliary/config_file_read.h"
#include "vond/triangle.h"
#include "vond/render.h"

// Returns the triangles stored in the given mesh config file.
//
std::vector<triangle_s> kmesh_mesh_triangles(const char *const meshFilename)
{
    config_file_read_c meshFile(meshFilename);

    std::vector<triangle_s> meshTriangles;
    std::unordered_map<std::string/*material name*/, triangle_material_s> knownMaterials;

    // Parse the mesh file to extract the materials and meshes in it.
    config_file_line_s line = meshFile.next_line();
    while (!meshFile.file_is_at_end())
    {
        //const std::string lineString = line.first;
        const unsigned parentIndentLevel = line.indentLevel;

        // Skip empty lines.
        if (line.command == 0)
        {
            line = meshFile.next_line();
            continue;
        }

        meshFile.error_if_not((parentIndentLevel == 0), "Expected indent level to be zero.");

        // Parse the line.
        switch (line.command)
        {
            case '?':       // Comment line.
            {
                line = meshFile.next_line();
                break;
            }
            case 'f':       // File format.
            {
                /// TODO: Make sure this is a compatible file version.

                line = meshFile.next_line();

                break;
            }
            case 'm':       // Material.
            {
                meshFile.error_if_not((line.indentLevel == 0), "Expected the material block to have 0-indent.");
                meshFile.error_if_not((line.params.size() == 1), "Expected material line to have one parameter.");

                const std::string materialName = line.params.at(0);
                meshFile.error_if_not((!materialName.empty()), "Encountered an empty material name.");

                // Get the material's properties.
                triangle_material_s material;
                material.baseColor = {127, 127, 127, 255};
                material.name = materialName;
                while (!meshFile.file_is_at_end())
                {
                    line = meshFile.next_line();

                    // Validate the indent level. If 0, we're finished with the
                    // material block. If higher than the current level + 1, we
                    // got a problem.
                    meshFile.error_if_not(line.indentLevel <= (parentIndentLevel + 1), "Can't have sub-indents for material.");
                    if (line.indentLevel == 0)
                    {
                        //add_to_known_materials({materialName, material});

                        knownMaterials.insert({materialName, material});

                        break;
                    }

                    switch (line.command)
                    {
                        case 'c':       // Color.
                        {
                            meshFile.error_if_not((line.params.size() == 3), "Expected three parameters for the material's color.");

                            material.baseColor.r = std::stoi(line.params.at(0));
                            material.baseColor.g = std::stoi(line.params.at(1));
                            material.baseColor.b = std::stoi(line.params.at(2));

                            break;
                        }
                        case 't':       // Texture.
                        {
                            meshFile.error_if_not(!material.texture, "Can't re-define the texture for a material.");
                            meshFile.error_if_not((line.params.size() == 1), "Expected one parameters for the material's texture filename.");

                            material.texture = new image_s<uint8_t>(QImage(QString::fromStdString(line.params.at(0))));

                            break;
                        }

                        default:
                        {
                            meshFile.error_if_not(0, "Unrecognized line in mesh file.");
                            break;
                        }
                    }
                }

                break;
            }
            case 'o':       // Object.
            {
                // Get all polygons (for now, we assume they're all triangles).
                std::vector<triangle_s> triangles;
                line = meshFile.next_line();
                while (!meshFile.file_is_at_end() &&
                       line.command == 'p' &&
                       line.indentLevel == (parentIndentLevel + 1))
                {
                    const unsigned indentLevel = line.indentLevel;

                    meshFile.error_if_not(line.params.size() == 1, "Expected the polygon line to have one parameter.");

                    /// For now, assume we always have triangles rathern than other types of polygons.
                    triangle_s tri;
                    triangle_material_s material = knownMaterials.at(line.params.at(0));

                    // Get all vertices of this polygon.
                    std::vector<vertex4_s> vertices;
                    line = meshFile.next_line();
                    while (!meshFile.file_is_at_end() &&
                           line.command == 'v' &&
                           line.indentLevel == (indentLevel + 1))
                    {
                        const unsigned indentLevel = line.indentLevel;

                        meshFile.error_if_not(line.params.size() == 3, "Expected vertices to have three coordinates.");

                        // Extract the vertex data from the line.
                        vertex4_s vert;
                        {
                            // In case no u,v coordinates are defined for this
                            // vertex later on, pre-initialize them to 0 here.
                            vert.uv[0] = 0;
                            vert.uv[1] = 0;

                            vert.pos.x = std::stoi(line.params.at(0));
                            vert.pos.y = std::stoi(line.params.at(1));
                            vert.pos.z = std::stoi(line.params.at(2));
                            vert.w = 1;
                        }

                        // Get the vertice's u,v coordinates.
                        line = meshFile.next_line();
                        while (!meshFile.file_is_at_end() &&
                               line.command == '!' &&
                               line.indentLevel == (indentLevel + 1))
                        {
                            const unsigned indentLevel = line.indentLevel;

                            meshFile.error_if_not(line.params.size() == 2, "Expected each vertex u,v coordinate pair to have only two entries.");

                            // Extract the u,v data from the line. We clamp the values
                            // to a maximum of 1-(float)eps to prevent out-of-bounds
                            // access of texture data.
                            static_assert(std::is_same<double, std::remove_all_extents<decltype(vert.uv)>::type>::value, "Expected a floating-point variable.");
                            vert.uv[0] = std::min((std::stoi(line.params.at(0)) / 1000000.0), (1.0 - std::numeric_limits<float>::epsilon()));
                            vert.uv[1] = std::min((std::stoi(line.params.at(1)) / 1000000.0), (1.0 - std::numeric_limits<float>::epsilon()));

                            line = meshFile.next_line();
                            if (meshFile.file_is_at_end())
                            {
                                break;
                            }
                            meshFile.error_if_not((line.indentLevel < indentLevel), "Did not expect another level of indentation here.");
                        }

                        vertices.push_back(vert);
                    }

                    // Save the new polygon.
                    {
                        meshFile.error_if_not((vertices.size() == 3), "Encountered a non-triangle polygon. They're not supported.");

                        tri.v[0] = vertices.at(0);
                        tri.v[1] = vertices.at(1);
                        tri.v[2] = vertices.at(2);
                        tri.material = material;

                        triangles.push_back(tri);
                    }
                }

                meshTriangles.insert(meshTriangles.end(), triangles.begin(), triangles.end());

                break;
            }

            default:
            {
                meshFile.error_if_not(0, "Unexpected data in mesh file."); break;
            }
        }
    }

    return meshTriangles;
}
