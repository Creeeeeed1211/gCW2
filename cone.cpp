#include "cone.hpp"

#include <numbers>
#include "../vmlib/mat33.hpp"

SimpleMeshData make_cone(bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform)
{
    std::vector<Vec3f> pos;
    std::vector<Vec3f> normals;

    // Define the apex point of the cone
    Vec3f apex = { 1.f, 0.f, 0.f };

    // Generate cone sides
    for (std::size_t i = 0; i < aSubdivs; ++i) {
        float angle1 = i / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
        float angle2 = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;

        Vec3f base1 = { 0.f, std::cos(angle1), std::sin(angle1) };
        Vec3f base2 = { 0.f, std::cos(angle2), std::sin(angle2) };

        // Position
        pos.push_back(base1);
        pos.push_back(base2);
        pos.push_back(apex);

        // Normal
        Vec3f normal1 = normalize(cross(apex - base1, base2 - base1));
        normals.push_back(normal1);
        normals.push_back(normal1);
        normals.push_back(normal1);
    }

    // Generate cap if needed
    if (aCapped) {
        Vec3f center = { 0.f, 0.f, 0.f };
        Vec3f normal = { -1.f, 0.f, 0.f };

        for (std::size_t i = 0; i < aSubdivs; ++i) {
            float angle1 = i / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
            float angle2 = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;

            Vec3f base1 = { 0.f, std::cos(angle1), std::sin(angle1) };
            Vec3f base2 = { 0.f, std::cos(angle2), std::sin(angle2) };

            // Position
            pos.push_back(center);
            pos.push_back(base2);
            pos.push_back(base1);

            // Normal
            normals.push_back(normal);
            normals.push_back(normal);
            normals.push_back(normal);
        }
    }

    // Apply transformation
    for (auto& p : pos) {
        Vec4f p4{ p.x, p.y, p.z, 1.f };
        Vec4f transformed = aPreTransform * p4;
        p = Vec3f{ transformed.x, transformed.y, transformed.z };
    }

    // Colors
    std::vector<Vec3f> colors(pos.size(), aColor);

    return SimpleMeshData{ pos, colors, normals };
}
SimpleMeshData make_square_cone(bool aCapped, Vec3f aColor, Mat44f aPreTransform) {
    std::vector<Vec3f> positions;
    std::vector<Vec3f> normals;

    // 顶点定义
    Vec3f apex = { 0.f, 0.f, 1.f };  // 锥顶点
    Vec3f base0 = { -0.5f, -0.5f, 0.f }; // 四边形底面顶点 1
    Vec3f base1 = { 0.5f, -0.5f, 0.f }; // 四边形底面顶点 2
    Vec3f base2 = { 0.5f,  0.5f, 0.f }; // 四边形底面顶点 3
    Vec3f base3 = { -0.5f,  0.5f, 0.f }; // 四边形底面顶点 4

    // 锥体侧面（4 个三角形）
    positions.push_back(base0);
    positions.push_back(base1);
    positions.push_back(apex);

    positions.push_back(base1);
    positions.push_back(base2);
    positions.push_back(apex);

    positions.push_back(base2);
    positions.push_back(base3);
    positions.push_back(apex);

    positions.push_back(base3);
    positions.push_back(base0);
    positions.push_back(apex);

    // 法线计算（针对每个三角形）
    for (size_t i = 0; i < positions.size(); i += 3) {
        Vec3f p0 = positions[i];
        Vec3f p1 = positions[i + 1];
        Vec3f p2 = positions[i + 2];

        Vec3f normal = normalize(cross(p1 - p0, p2 - p0));
        normals.push_back(normal);
        normals.push_back(normal);
        normals.push_back(normal);
    }

    // 底面
    if (aCapped) {
        Vec3f baseNormal = { 0.f, 0.f, -1.f }; // 底面朝下的法线
        positions.push_back(base0);
        positions.push_back(base3);
        positions.push_back(base2);

        positions.push_back(base2);
        positions.push_back(base1);
        positions.push_back(base0);

        normals.push_back(baseNormal);
        normals.push_back(baseNormal);
        normals.push_back(baseNormal);
        normals.push_back(baseNormal);
        normals.push_back(baseNormal);
        normals.push_back(baseNormal);
    }

    // 应用变换矩阵
    for (auto& pos : positions) {
        Vec4f pos4{ pos.x, pos.y, pos.z, 1.f };
        Vec4f transformed = aPreTransform * pos4;
        pos = { transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w };
    }

    // 颜色赋值
    std::vector<Vec3f> colors(positions.size(), aColor);

    return SimpleMeshData{
        std::move(positions),
        std::move(colors),
        std::move(normals),
        {} // 无纹理坐标
    };
}

