#include "simple_mesh.hpp"
#include <iostream>

SimpleMeshData concatenate(SimpleMeshData aM, SimpleMeshData const& aN)
{
	aM.positions.insert(aM.positions.end(), aN.positions.begin(), aN.positions.end());
	aM.colors.insert(aM.colors.end(), aN.colors.begin(), aN.colors.end());
	aM.normals.insert(aM.normals.end(), aN.normals.begin(), aN.normals.end());
	return aM;
}


GLuint create_vao(SimpleMeshData const& aMeshData)
{
    // 检查数据完整性
    if (aMeshData.positions.size() != aMeshData.colors.size() ||
        aMeshData.positions.size() != aMeshData.normals.size()) {
        throw std::runtime_error("Mesh data arrays have inconsistent sizes!");
    }

    std::cout << "Creating VAO with data sizes:\n";
    std::cout << "Positions: " << aMeshData.positions.size() << "\n";
    std::cout << "Colors: " << aMeshData.colors.size() << "\n";
    std::cout << "Normals: " << aMeshData.normals.size() << "\n";
    std::cout << "Texcoords: " << aMeshData.texcoords.size() << "\n";

    // 创建位置 VBO
    GLuint posVBO = 0;
    glGenBuffers(1, &posVBO);
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferData(GL_ARRAY_BUFFER, aMeshData.positions.size() * sizeof(Vec3f), aMeshData.positions.data(), GL_STATIC_DRAW);

    // 创建颜色 VBO
    GLuint colVBO = 0;
    glGenBuffers(1, &colVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colVBO);
    glBufferData(GL_ARRAY_BUFFER, aMeshData.colors.size() * sizeof(Vec3f), aMeshData.colors.data(), GL_STATIC_DRAW);

    // 创建法线 VBO
    GLuint normVBO = 0;
    glGenBuffers(1, &normVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normVBO);
    glBufferData(GL_ARRAY_BUFFER, aMeshData.normals.size() * sizeof(Vec3f), aMeshData.normals.data(), GL_STATIC_DRAW);

    // 创建纹理坐标 VBO（如果存在）
    GLuint texCoordVBO = 0;
    if (!aMeshData.texcoords.empty()) {
        glGenBuffers(1, &texCoordVBO);
        glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
        glBufferData(GL_ARRAY_BUFFER, aMeshData.texcoords.size() * sizeof(Vec2f), aMeshData.texcoords.data(), GL_STATIC_DRAW);
    }

    // 创建 VAO
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // 绑定位置属性
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // 绑定颜色属性
    glBindBuffer(GL_ARRAY_BUFFER, colVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // 绑定法线属性
    if (!aMeshData.normals.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, normVBO);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
    }
    

    // 绑定纹理坐标属性
    if (!aMeshData.texcoords.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(3);
    }

    // 解绑 VAO 和 VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vao;
}

