#include "cylinder.hpp"

#include <numbers>

SimpleMeshData make_cylinder(bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform)
{
    std::vector<Vec3f> pos;
    std::vector<Vec3f> normals;

    // Compute the normal transformation matrix (inverse transpose of aPreTransform)
    Mat44f N = transpose(invert(aPreTransform));

    float prevY = std::cos(0);
    float prevZ = std::sin(0);

    for (std::size_t i = 0; i < aSubdivs; i++)
    {
        float const angle = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
        float y = std::cos(angle);
        float z = std::sin(angle);

        Vec3f nPrev = normalize(Vec3f{ 0, prevY, prevZ }); // Normal for the previous segment
        Vec3f nCurr = normalize(Vec3f{ 0, y, z });         // Normal for the current segment

        // Add side triangles with normals
        pos.emplace_back(Vec3f{ 0, prevY, prevZ });
        normals.emplace_back(nPrev);
        pos.emplace_back(Vec3f{ 0, y, z });
        normals.emplace_back(nCurr);
        pos.emplace_back(Vec3f{ 1, prevY, prevZ });
        normals.emplace_back(nPrev);

        pos.emplace_back(Vec3f{ 0, y, z });
        normals.emplace_back(nCurr);
        pos.emplace_back(Vec3f{ 1, y, z });
        normals.emplace_back(nCurr);
        pos.emplace_back(Vec3f{ 1, prevY, prevZ });
        normals.emplace_back(nPrev);

        prevY = y;
        prevZ = z;
    }

    if (aCapped)
    {
        Vec3f centerBottom = { 0.f, 0.f, 0.f };
        Vec3f normalBottom = { -1.f, 0.f, 0.f };

        Vec3f centerTop = { 1.f, 0.f, 0.f };
        Vec3f normalTop = { 1.f, 0.f, 0.f };

        for (std::size_t i = 0; i < aSubdivs; i++)
        {
            float const angle1 = i / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
            float const angle2 = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;

            Vec3f v1 = { 0.f, std::cos(angle1), std::sin(angle1) };
            Vec3f v2 = { 0.f, std::cos(angle2), std::sin(angle2) };

            // Bottom cap
            pos.emplace_back(centerBottom);
            normals.emplace_back(normalBottom);
            pos.emplace_back(v2);
            normals.emplace_back(normalBottom);
            pos.emplace_back(v1);
            normals.emplace_back(normalBottom);

            // Top cap
            Vec3f v1Top = { 1.f, std::cos(angle1), std::sin(angle1) };
            Vec3f v2Top = { 1.f, std::cos(angle2), std::sin(angle2) };

            pos.emplace_back(centerTop);
            normals.emplace_back(normalTop);
            pos.emplace_back(v1Top);
            normals.emplace_back(normalTop);
            pos.emplace_back(v2Top);
            normals.emplace_back(normalTop);
        }
    }

    // Transform positions and normals
    for (std::size_t i = 0; i < pos.size(); i++)
    {
        // Transform position
        Vec4f p4{ pos[i].x, pos[i].y, pos[i].z, 1.f };
        Vec4f t = aPreTransform * p4;
        t /= t.w;
        pos[i] = Vec3f{ t.x, t.y, t.z };

        // Transform and normalize normal
        Vec4f n4{ normals[i].x, normals[i].y, normals[i].z, 0.f };
        Vec4f nTransformed = N * n4;
        normals[i] = normalize(Vec3f{ nTransformed.x, nTransformed.y, nTransformed.z });
    }

    std::vector col(pos.size(), aColor);
    return SimpleMeshData{ std::move(pos), std::move(col), std::move(normals) };
}
