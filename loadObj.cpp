#include "loadObj.hpp"
#include <rapidobj/rapidobj.hpp>
#include "../support/error.hpp"

SimpleMeshData load_wavefront_obj(char const* aPath)
{
    auto result = rapidobj::ParseFile(aPath);
    if (result.error)
    {
        throw Error("Unable to load OBJ file '%s': %s", aPath, result.error.code.message().c_str());
    }
    rapidobj::Triangulate(result);

    SimpleMeshData ret;

    for (const auto& shape : result.shapes)
    {
        for (const auto& idx : shape.mesh.indices)
        {
            // Vertex position
            ret.positions.emplace_back(Vec3f{
                result.attributes.positions[idx.position_index * 3 + 0],
                result.attributes.positions[idx.position_index * 3 + 1],
                result.attributes.positions[idx.position_index * 3 + 2]
                });

            // Normal vector
            if (idx.normal_index >= 0)
            {
                ret.normals.emplace_back(Vec3f{
                    result.attributes.normals[idx.normal_index * 3 + 0],
                    result.attributes.normals[idx.normal_index * 3 + 1],
                    result.attributes.normals[idx.normal_index * 3 + 2]
                    });
            }
            else
            {
                // Default normal vector
                ret.normals.emplace_back(Vec3f{ 0.0f, 1.0f, 0.0f });
            }

            // Texture coordinates
            if (idx.texcoord_index >= 0)
            {
                ret.texcoords.emplace_back(Vec2f{
                    result.attributes.texcoords[idx.texcoord_index * 2 + 0],
                    result.attributes.texcoords[idx.texcoord_index * 2 + 1]
                    });
            }
            else
            {
                // Default texture coordinates
                ret.texcoords.emplace_back(Vec2f{ 0.0f, 0.0f });
            }

            // Default color
            ret.colors.emplace_back(Vec3f{ 1.0f, 1.0f, 1.0f });
        }
    }

    return ret;
}

