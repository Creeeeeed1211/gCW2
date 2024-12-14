#ifndef CUBE_HPP_6874B39C_112D_4D34_BD85_AB81A730955B
#define CUBE_HPP_6874B39C_112D_4D34_BD85_AB81A730955B

// This defines the vertex data for a colored unit cube.

constexpr float const kCubePositions[] = {
	+0.0625f, +0.25f, -0.0625f,  // Top face
	-0.0625f, +0.25f, -0.0625f,
	-0.0625f, +0.25f, +0.0625f,
	+0.0625f, +0.25f, -0.0625f,
	-0.0625f, +0.25f, +0.0625f,
	+0.0625f, +0.25f, +0.0625f,

	+0.0625f, -0.25f, +0.0625f,  // Front face
	+0.0625f, +0.25f, +0.0625f,
	-0.0625f, +0.25f, +0.0625f,
	+0.0625f, -0.25f, +0.0625f,
	-0.0625f, +0.25f, +0.0625f,
	-0.0625f, -0.25f, +0.0625f,

	-0.0625f, -0.25f, +0.0625f,  // Left face
	-0.0625f, +0.25f, +0.0625f,
	-0.0625f, +0.25f, -0.0625f,
	-0.0625f, -0.25f, +0.0625f,
	-0.0625f, +0.25f, -0.0625f,
	-0.0625f, -0.25f, -0.0625f,

	-0.0625f, -0.25f, -0.0625f,  // Bottom face
	+0.0625f, -0.25f, -0.0625f,
	+0.0625f, -0.25f, +0.0625f,
	-0.0625f, -0.25f, -0.0625f,
	+0.0625f, -0.25f, +0.0625f,
	-0.0625f, -0.25f, +0.0625f,

	+0.0625f, -0.25f, -0.0625f,  // Right face
	+0.0625f, +0.25f, -0.0625f,
	+0.0625f, +0.25f, +0.0625f,
	+0.0625f, -0.25f, -0.0625f,
	+0.0625f, +0.25f, +0.0625f,
	+0.0625f, -0.25f, +0.0625f,

	-0.0625f, -0.25f, -0.0625f,  // Back face
	-0.0625f, +0.25f, -0.0625f,
	+0.0625f, +0.25f, -0.0625f,
	-0.0625f, -0.25f, -0.0625f,
	+0.0625f, +0.25f, -0.0625f,
	+0.0625f, -0.25f, -0.0625f,
};


constexpr float const k2CubePositions[] = {
	+0.03125f, +0.125f, -0.03125f,  // Top face
	-0.03125f, +0.125f, -0.03125f,
	-0.03125f, +0.125f, +0.03125f,
	+0.03125f, +0.125f, -0.03125f,
	-0.03125f, +0.125f, +0.03125f,
	+0.03125f, +0.125f, +0.03125f,

	+0.03125f, -0.125f, +0.03125f,  // Front face
	+0.03125f, +0.125f, +0.03125f,
	-0.03125f, +0.125f, +0.03125f,
	+0.03125f, -0.125f, +0.03125f,
	-0.03125f, +0.125f, +0.03125f,
	-0.03125f, -0.125f, +0.03125f,

	-0.03125f, -0.125f, +0.03125f,  // Left face
	-0.03125f, +0.125f, +0.03125f,
	-0.03125f, +0.125f, -0.03125f,
	-0.03125f, -0.125f, +0.03125f,
	-0.03125f, +0.125f, -0.03125f,
	-0.03125f, -0.125f, -0.03125f,

	-0.03125f, -0.125f, -0.03125f,  // Bottom face
	+0.03125f, -0.125f, -0.03125f,
	+0.03125f, -0.125f, +0.03125f,
	-0.03125f, -0.125f, -0.03125f,
	+0.03125f, -0.125f, +0.03125f,
	-0.03125f, -0.125f, +0.03125f,

	+0.03125f, -0.125f, -0.03125f,  // Right face
	+0.03125f, +0.125f, -0.03125f,
	+0.03125f, +0.125f, +0.03125f,
	+0.03125f, -0.125f, -0.03125f,
	+0.03125f, +0.125f, +0.03125f,
	+0.03125f, -0.125f, +0.03125f,

	-0.03125f, -0.125f, -0.03125f,  // Back face
	-0.03125f, +0.125f, -0.03125f,
	+0.03125f, +0.125f, -0.03125f,
	-0.03125f, -0.125f, -0.03125f,
	+0.03125f, +0.125f, -0.03125f,
	+0.03125f, -0.125f, -0.03125f,
};


constexpr float const kCubeColors[] = {
	 0.4039f, 0.3294f, 0.2627f, // Brown for all vertices
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,

	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,

	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,

	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,

	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,

	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
	0.4039f, 0.3294f, 0.2627f,
};
constexpr float const k2CubeColors[] = {
	0.0f, 0.0f, 0.0f, 
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
};
constexpr float const k3CubeColors[] = {
	0.8039f, 0.7020f, 0.4019f, // Golden yellow for all vertices
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,

	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,

	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,

	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,

	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,

	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
	0.8039f, 0.7020f, 0.4019f,
};



static_assert( sizeof(kCubeColors) == sizeof(kCubePositions),
	"Size of cube colors and cube positions do not match. Both are 3D vectors."
);

#endif // CUBE_HPP_6874B39C_112D_4D34_BD85_AB81A730955B
