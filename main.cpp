#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"



#include <numbers>
#include <typeinfo>
#include <stdexcept>

#include <cstdio>
#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"

#include "defaults.hpp"
#include "loadObj.hpp"
#include <algorithm>
#include "cube.hpp"
#include "cylinder.hpp"
#include "cone.hpp"

namespace
{
	bool onF = false;
	constexpr float kMovementPerSecond_ = 1.5f; // units per second
	constexpr float kMouseSensitivity_ = 0.01f; // radians per pixel

	constexpr float shiftAcc = 4.f;
	constexpr float ctrlAcc = 0.2f;

	constexpr char const* kWindowTitle = "COMP3811 - CW2";

	void glfw_callback_error_(int, char const*);

	void glfw_callback_key_(GLFWwindow*, int, int, int, int);

	struct GLFWCleanupHelper
	{
		~GLFWCleanupHelper();
	};
	struct GLFWWindowDeleter
	{
		~GLFWWindowDeleter();
		GLFWwindow* window;
	};

	struct State_
	{
		ShaderProgram* prog;

		struct CamCtrl_
		{
			bool cameraActive;
			bool actionZoomIn, actionZoomOut;
			bool moveForward;
			bool moveBackward;
			bool moveLeft;
			bool moveRight;
			bool moveUp, moveDown;
			bool speedUp, speedDown;

			float posX;
			float posY;
			float posZ;

			float phi, theta;
			float radius;

			float lastX, lastY;

		} camControl;

		int cameraMode = 0; // 0: Free camera, 1: Follow rocket, 2: Ground-fixed camera

		Vec3f groundCameraPosition = { 6.f, 0.f, 2.0f }; // Position of ground-fixed camera
		Vec3f fixedCameraPosition = { 4.5f, -0.005f, -5.0f }; // Fixed camera position

		// Animation
		float rocketYPosition = 0.25f;
		float rocketXPosition = 4.5f;
		bool rocketFlying = false;
		const float rocketInitialY = 0.25f;
		const float rocketInitialX = 4.5f;
		float rocketYVelocity = 1;
		float rocketXVelocity = 2;
		float dir = 45;
		float rocketFlyTime = 0.0f;
	};


	void glfw_callback_error_(int, char const*);

	void glfw_callback_key_(GLFWwindow*, int, int, int, int);
	void glfw_callback_motion_(GLFWwindow*, double, double);
	void glfw_cb_button_(GLFWwindow*, int, int, int);
}

GLuint load_texture_2d(const char* filepath)
{
	stbi_set_flip_vertically_on_load(true);

	int width, height, channels;
	unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
	if (!data) {
		throw std::runtime_error("Failed to load texture");
	}

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);
	return texture;
}



int main() try
{
	// Initialize GLFW
	if (GLFW_TRUE != glfwInit())
	{
		char const* msg = nullptr;
		int ecode = glfwGetError(&msg);
		throw Error("glfwInit() failed with '%s' (%d)", msg, ecode);
	}

	// Ensure that we call glfwTerminate() at the end of the program.
	GLFWCleanupHelper cleanupHelper;

	// Configure GLFW and create window
	glfwSetErrorCallback(&glfw_callback_error_);

	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_DEPTH_BITS, 32);



#	if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#	endif // ~ !NDEBUG

	GLFWwindow* window = glfwCreateWindow(
		1280,
		720,
		kWindowTitle,
		nullptr, nullptr
	);

	State_ state{};
	if (!window)
	{
		char const* msg = nullptr;
		int ecode = glfwGetError(&msg);
		throw Error("glfwCreateWindow() failed with '%s' (%d)", msg, ecode);
	}

	GLFWWindowDeleter windowDeleter{ window };


	// Set up event handling
	// TODO: Additional event handling setup
	// Set the user pointer to the state object
	glfwSetWindowUserPointer(window, &state);

	glfwSetKeyCallback(window, &glfw_callback_key_);
	glfwSetCursorPosCallback(window, &glfw_callback_motion_);
	glfwSetMouseButtonCallback(window, &glfw_cb_button_);




	// Set up drawing stuff
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if (!gladLoadGLLoader((GLADloadproc)&glfwGetProcAddress))
		throw Error("gladLoaDGLLoader() failed - cannot load GL API!");

	std::printf("RENDERER %s\n", glGetString(GL_RENDERER));
	std::printf("VENDOR %s\n", glGetString(GL_VENDOR));
	std::printf("VERSION %s\n", glGetString(GL_VERSION));
	std::printf("SHADING_LANGUAGE_VERSION %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Ddebug output
#	if !defined(NDEBUG)
	setup_gl_debug_output();
#	endif // ~ !NDEBUG

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	// TODO: global GL setup goes here

	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize(window, &iwidth, &iheight);

	glViewport(0, 0, iwidth, iheight);

	// Load shader program
	ShaderProgram prog({
		{ GL_VERTEX_SHADER, "assets/cw2/default.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/default.frag" }
		});

	state.prog = &prog;
	state.camControl.radius = 10.f;

	GLuint texture = load_texture_2d("assets/cw2/L3211E-4k.jpg");
	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();

	// TODO: global GL setup goes here
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // 剔除背面
	glFrontFace(GL_CCW); // 确保逆时针为正面

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f); // RGBA values for grey
	// 
	// TODO: global GL setup goes here
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f); // RGBA values for grey
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	OGL_CHECKPOINT_ALWAYS();

	GLuint posVBO = 0;
	glGenBuffers(1, &posVBO);
	glBindBuffer(GL_ARRAY_BUFFER, posVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kCubePositions), kCubePositions, GL_STATIC_DRAW);

	GLuint colVBO = 0;
	glGenBuffers(1, &colVBO);
	glBindBuffer(GL_ARRAY_BUFFER, colVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeColors), kCubeColors, GL_STATIC_DRAW);

	GLuint norVBO = 0;
	glGenBuffers(1, &norVBO);
	glBindBuffer(GL_ARRAY_BUFFER, norVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeNormals), kCubeNormals, GL_STATIC_DRAW);

	GLuint posVBOP = 0;
	glGenBuffers(1, &posVBOP);
	glBindBuffer(GL_ARRAY_BUFFER, posVBOP);
	glBufferData(GL_ARRAY_BUFFER, sizeof(k2CubePositions), k2CubePositions, GL_STATIC_DRAW);

	GLuint colVBOP = 0;
	glGenBuffers(1, &colVBOP);
	glBindBuffer(GL_ARRAY_BUFFER, colVBOP);
	glBufferData(GL_ARRAY_BUFFER, sizeof(k2CubeColors), k2CubeColors, GL_STATIC_DRAW);

	GLuint posVBOS = 0;
	glGenBuffers(1, &posVBOS);
	glBindBuffer(GL_ARRAY_BUFFER, posVBOS);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kCubePositions), kCubePositions, GL_STATIC_DRAW);

	GLuint colVBOS = 0;
	glGenBuffers(1, &colVBOS);
	glBindBuffer(GL_ARRAY_BUFFER, colVBOS);
	glBufferData(GL_ARRAY_BUFFER, sizeof(k3CubeColors), k3CubeColors, GL_STATIC_DRAW);


	//VAO
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, posVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, colVBO);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, norVBO);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	GLuint vaoPtr = 0;
	glGenVertexArrays(1, &vaoPtr);
	glBindVertexArray(vaoPtr);
	glBindBuffer(GL_ARRAY_BUFFER, posVBOP);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, colVBOP);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	GLuint vaoS = 0;
	glGenVertexArrays(1, &vaoS);
	glBindVertexArray(vaoS);
	glBindBuffer(GL_ARRAY_BUFFER, posVBOS);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, colVBOS);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);



	auto model = load_wavefront_obj("assets/cw2/langerso.obj");
	auto launchpadMesh = load_wavefront_obj("assets/cw2/landingpad.obj");
	GLuint vaoModels = create_vao(model);
	GLuint launchpadVAO = create_vao(launchpadMesh);
	std::size_t vertexCounterM = model.positions.size();
	std::size_t vertexCounterL = launchpadMesh.positions.size();


	auto cylinderW = make_cylinder(true, 16, { 1.0f, 1.0f, 1.0f }, make_translation({ -0.075f,0.09f,0.f }) * make_scaling(0.15f, 0.05f, 0.05f)); // ���˸ǣ�16��ϸ�֣���ɫ
	GLuint vaoCylinder = create_vao(cylinderW);
	std::size_t cylinderVertexCount = cylinderW.positions.size();

	auto cylinder2 = make_cylinder(true, 16, { .0f, .0f, .0f }, make_translation({ -0.02f,0.09f,0.f }) * make_scaling(0.22f, 0.025f, 0.025f));
	GLuint vaoCylinder2 = create_vao(cylinder2);
	std::size_t cylinderVertexCount2 = cylinder2.positions.size();

	//cones

	auto cone1 = make_cone(
		true,
		16,
		{ 0.0f, 0.0f, 0.0f },
		make_translation({ 0.2f, .09f, 0.f }) *
		make_scaling(0.04f, 0.025f, 0.025f)
	);

	GLuint vaoCone1 = create_vao(cone1);
	std::size_t coneVertexCount1 = cone1.positions.size();

	auto squareCone = make_square_cone(
		true,
		{ 0.4039f, 0.3294f, 0.2627f, }, make_rotation_x(-std::numbers::pi_v<float> / 2.f) *
		make_scaling(0.125, 0.125, 0.2) * make_translation({ 0,0.f,1 }));

	GLuint vaoSquareCone = create_vao(squareCone);
	std::size_t squareConeVertexCount = squareCone.positions.size();


	double lastTime = glfwGetTime(); // Initialize with the current time


	// Main loop
	while (!glfwWindowShouldClose(window))
	{

		//move with time 
		double currentTime = glfwGetTime();
		float deltaTime = static_cast<float>(currentTime - lastTime);
		lastTime = currentTime;
		//speed control
		float currentSpeed = kMovementPerSecond_;
		if (state.camControl.speedUp)
		{
			currentSpeed *= shiftAcc;
		}
		if (state.camControl.speedDown)
		{
			currentSpeed *= ctrlAcc;
		}
		// Let GLFW process events
		glfwPollEvents();

		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize(window, &nwidth, &nheight);

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if (0 == nwidth || 0 == nheight)
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize(window, &nwidth, &nheight);
				} while (0 == nwidth || 0 == nheight);
			}

			glViewport(0, 0, nwidth, nheight);
		}

		if (state.rocketFlying)
		{
			// 更新飞行时间
			state.rocketFlyTime += deltaTime;

			//acc
			float speedMultiplier = 1.0f + (state.rocketFlyTime - 2) * 0.5f; // 线性增长
			// float speedMultiplier = 1.0f * std::exp(state.rocketFlyTime * 0.2f); // 指数增长（可选）

			// 控制火箭运动逻辑
			if (state.rocketFlyTime <= 1.5f)
			{
				// 前1.5秒垂直上升，速度随时间增长
				state.rocketYPosition += deltaTime * 1.0f;
				state.dir = 0;
			}
			else
			{
				state.dir = std::numbers::pi_v<float> / 4.f;
				// 1.5秒后火箭沿曲线运动
				float a = -0.02f;  // 控制曲线的弯曲程度
				float b = -1.0f;   // 控制曲线的倾斜方向
				float c = state.rocketInitialY + 1.5; // 初始高度

				// 更新 X 和 Y 轴位置
				state.rocketXPosition -= deltaTime * state.rocketXVelocity * speedMultiplier; // X 轴负方向移动
				state.rocketYPosition = a * (state.rocketXPosition - state.rocketInitialX) * (state.rocketXPosition - state.rocketInitialX)
					+ b * (state.rocketXPosition - state.rocketInitialX) + c;
			}

			// 检查停止条件
			if (state.rocketYPosition < state.rocketInitialY || state.rocketXPosition < -15.0f)
			{
				// 停止飞行并重置状态
				state.rocketFlying = false;
				state.rocketYPosition = state.rocketInitialY;
				state.rocketXPosition = state.rocketInitialX;
				state.rocketFlyTime = 0;
			}
		}








		// ws moving

		float forwardX = std::sin(state.camControl.phi) * std::cos(state.camControl.theta);
		float forwardY = std::sin(state.camControl.theta);
		float forwardZ = -std::cos(state.camControl.phi) * std::cos(state.camControl.theta);
		if (state.camControl.moveForward)
		{
			state.camControl.posX += forwardX * kMovementPerSecond_ * deltaTime * currentSpeed;
			state.camControl.posY -= forwardY * kMovementPerSecond_ * deltaTime * currentSpeed;
			state.camControl.posZ += forwardZ * kMovementPerSecond_ * deltaTime * currentSpeed;

		}
		if (state.camControl.moveBackward)
		{
			state.camControl.posX -= forwardX * kMovementPerSecond_ * deltaTime * currentSpeed;
			state.camControl.posY += forwardY * kMovementPerSecond_ * deltaTime * currentSpeed;
			state.camControl.posZ -= forwardZ * kMovementPerSecond_ * deltaTime * currentSpeed;

		}

		//a d moving

		if (state.camControl.moveLeft)
		{
			float leftX = -std::cos(state.camControl.phi);
			float leftZ = -std::sin(state.camControl.phi);

			state.camControl.posX += leftX * kMovementPerSecond_ * deltaTime * currentSpeed;
			state.camControl.posZ += leftZ * kMovementPerSecond_ * deltaTime * currentSpeed;
		}

		if (state.camControl.moveRight)
		{
			float rightX = std::cos(state.camControl.phi);
			float rightZ = std::sin(state.camControl.phi);

			state.camControl.posX += rightX * kMovementPerSecond_ * deltaTime * currentSpeed;
			state.camControl.posZ += rightZ * kMovementPerSecond_ * deltaTime * currentSpeed;
		}
		//q e moving
		if (state.camControl.moveUp)
		{
			state.camControl.posY += kMovementPerSecond_ * deltaTime * currentSpeed;
		}
		if (state.camControl.moveDown)
		{
			state.camControl.posY -= kMovementPerSecond_ * deltaTime * currentSpeed;
		}

		// Update the world-to-camera transformation
		Mat44f world2camera;
		if (state.cameraMode == 0)
		{
			glFrontFace(GL_CCW); // 其他相机模式使用逆时针
			// Free camera mode
			Mat44f T = make_translation({ -state.camControl.posX, -state.camControl.posY, -state.camControl.posZ });
			Mat44f Rx = make_rotation_x(state.camControl.theta);
			Mat44f Ry = make_rotation_y(state.camControl.phi);
			world2camera = Rx * Ry * T;
		}
		else if (state.cameraMode == 1)
		{
			glFrontFace(GL_CCW); // 其他相机模式使用逆时针
			// Follow rocket camera mode
			world2camera = make_rotation_x(std::numbers::pi_v<float> / 3.f) *
				make_translation({ -state.rocketXPosition, -state.rocketYPosition - 1.8f, 3.0f });
		}
		else if (state.cameraMode == 2)
		{
			glFrontFace(GL_CW); // 地面固定相机需要顺时针
			// Ground-fixed camera setup
			Vec3f camPos = state.groundCameraPosition;
			Vec3f rocketPos = { state.rocketXPosition, state.rocketYPosition, -5.0f };

			// Compute forward, right, and up vectors
			Vec3f forward = normalize(rocketPos - camPos); // Ensure forward points from camera to rocket
			Vec3f up = { 0.0f, 1.0f, 0.0f };              // World up vector
			Vec3f right = normalize(cross(up, forward));  // Compute right vector
			up = normalize(cross(forward, right));        // Recompute up to ensure orthogonality

			// Build the view matrix
			world2camera = {
				 right.x,    right.y,    right.z,   0.0f ,
				 up.x,       up.y,       up.z,      0.0f,
				 -forward.x, -forward.y, -forward.z, 0.0f ,
				 0.0f,       0.0f,       0.0f,      1.0f 
			};

			// Translate to camera position
			world2camera = world2camera * make_translation(-camPos);
		}




		//Mat44f world2camera = make_translation({ 0.f, 0.f, 0.f });

		Mat44f projection = make_perspective_projection(
			60.f * std::numbers::pi_v<float> / 180.f,
			fbwidth / float(fbheight),
			0.1f, 100.0f
		);
		Mat44f model2world = make_translation({ 0,0,0 });
		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		//TODO: draw frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Mat44f projCameraWorld = projection * world2camera * model2world;
		// Bind shader program
		glUseProgram(prog.programId());

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(prog.programId(), "uTexture"), 0);



		// Light direction (normalized)
		Vec3f lightDir = normalize(Vec3f{ 0.f, 1.f, -1.f });
		glUniform3fv(2, 1, &lightDir.x); // Light direction (location 2)
		glUniform3f(3, 1.0f, 1.0f, 1.0f); // Diffuse light color (white)
		glUniform3f(4, 0.2f, 0.2f, 0.2f); // Ambient light color (dim grey)




		GLint useTextureLoc = glGetUniformLocation(prog.programId(), "useTexture");
		GLint projCameraWorldLoc = glGetUniformLocation(prog.programId(), "uProjCameraWorld");
		GLint uNormalMatrixLoc = glGetUniformLocation(prog.programId(), "uNormalMatrix");




		Mat33f normalMatrix = mat44_to_mat33(transpose(invert(model2world)));

		glUniformMatrix3fv(uNormalMatrixLoc // uNormalMatrixLoc = 1
			, 1, GL_TRUE, normalMatrix.v);



		// land draw
		glUniform1i(useTextureLoc, GL_TRUE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(vaoModels);
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorld.v);
		glDrawArrays(GL_TRIANGLES, 0, vertexCounterM);

		// Point light parameters
		Vec3f pointLightPositions[3] = {
		{13.5f, 5.0f, 0.0f},     // Position of the first point light
		{0.0f, 5.0f, 13.5f},   // Position of the second point light
		{-13.5f, 5.0f, 0.0f}      // Position of the third point light};
		};

		Vec3f pointLightColors[3] = {
		{1.0f, 0.0f, 0.0f},     // Red color for the first point light
		{0.0f, 1.0f, 0.0f},     // Green color for the second point light
		{0.0f, 0.0f, 1.0f}      // Blue color for the third point light
		};

		float pointLightIntensities[3] = { 1.0f, 0.8f, 1.2f };   // Intensities for the point lights
		float pointLightAttenuations[3] = { 0.1f, 0.2f, 0.15f }; // Attenuation factors for the point lights

		// Pass point light data to the shader
		for (int i = 0; i < 3; ++i) {
			glUniform3fv(7 + i, 1, &pointLightPositions[i].x);   // Set position of the i-th point light (uPointLightPos)
			glUniform3fv(10 + i, 1, &pointLightColors[i].x);     // Set color of the i-th point light (uPointLightColor)
			glUniform1f(13 + i, pointLightIntensities[i]);       // Set intensity of the i-th point light (uPointLightIntensity)
			glUniform1f(16 + i, pointLightAttenuations[i]);      // Set attenuation of the i-th point light (uPointLightAttenuation)
		}

		glUniform1i(useTextureLoc, GL_FALSE);
		//rocket draw 
		Mat44f rocketModel2World = make_translation({ state.rocketXPosition,state.rocketYPosition, -5.0f });
		Mat44f projCameraWorldRocket = projection * world2camera * rocketModel2World;
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorldRocket.v);
		//CUBE
		glBindVertexArray(vao);

		glDrawArrays(GL_TRIANGLES, 0, sizeof(kCubePositions));


		glBindVertexArray(vaoCylinder);
		glDrawArrays(GL_TRIANGLES, 0, cylinderVertexCount);
		glBindVertexArray(0);

		glBindVertexArray(vaoSquareCone);
		glDrawArrays(GL_TRIANGLES, 0, squareConeVertexCount);
		glBindVertexArray(0);
		//watch
		glUniform1i(useTextureLoc, GL_FALSE);
		Mat44f YModel2World = make_translation({ state.rocketXPosition, state.rocketYPosition, -5.0f }) * make_rotation_y(std::numbers::pi_v<float> / 2.f);
		Mat44f projCameraWorldY = projection * world2camera * YModel2World;
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorldY.v);


		Mat33f normalMatrixY = mat44_to_mat33(transpose(invert(YModel2World)));
		glUniformMatrix3fv(uNormalMatrixLoc, 1, GL_TRUE, normalMatrixY.v);

		glBindVertexArray(vaoCylinder);
		glDrawArrays(GL_TRIANGLES, 0, cylinderVertexCount);
		glBindVertexArray(0);

		//pad
		Mat44f Model2WorldPad = make_translation({ 4.5f,0.005f, -5.0f });
		Mat44f projCameraWorldPad = projection * world2camera * Model2WorldPad;
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorldPad.v);
		glBindVertexArray(launchpadVAO);
		glDrawArrays(GL_TRIANGLES, 0, vertexCounterL);
		glBindVertexArray(0);



		//square struct
		Mat44f Model2WorldS = make_translation({ state.rocketXPosition,state.rocketYPosition + 0.1f, -5.0f }) * make_rotation_y(std::numbers::pi_v<float> / 2.f) * make_scaling(1.1f, 0.28f, 1.1f);
		Mat44f projCameraWorldS = projection * world2camera * Model2WorldS;
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorldS.v);
		glBindVertexArray(vaoS);
		glDrawArrays(GL_TRIANGLES, 0, cylinderVertexCount);
		glBindVertexArray(0);


		//ptr
		Mat44f Model2WorldP = make_translation({ state.rocketXPosition, 0.1f + state.rocketYPosition, -5.0f }) * make_scaling(2.5f, .1f, .1f);
		Mat44f projCameraWorldP = projection * world2camera * Model2WorldP;
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorldP.v);

		glBindVertexArray(vaoPtr);

		glDrawArrays(GL_TRIANGLES, 0, sizeof(k2CubePositions));

		Mat44f Model2WorldP2 = make_translation({ state.rocketXPosition, 0.1f + state.rocketYPosition, -5.0f }) * make_rotation_y(std::numbers::pi_v<float> / 2.f) * make_scaling(2.5f, .1f, .1f);
		Mat44f projCameraWorldP2 = projection * world2camera * Model2WorldP2;
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorldP2.v);

		glBindVertexArray(vaoPtr);

		glDrawArrays(GL_TRIANGLES, 0, sizeof(k2CubePositions));

		Mat44f Model2WorldP3 = make_translation({ state.rocketXPosition, 0.1f + state.rocketYPosition, -5.015 }) * make_rotation_x(std::numbers::pi_v<float> / 1.5f) * make_scaling(2.5f, .18f, .1f);
		Mat44f projCameraWorldP3 = projection * world2camera * Model2WorldP3;
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorldP3.v);

		glBindVertexArray(vaoPtr);

		glDrawArrays(GL_TRIANGLES, 0, sizeof(k2CubePositions));

		Mat44f Model2WorldP4 = make_translation({ state.rocketXPosition - 0.015f, 0.1f + state.rocketYPosition, -5.0 }) * make_rotation_z(std::numbers::pi_v<float> / 3.5f) * make_rotation_y(std::numbers::pi_v<float> / 2.f) * make_scaling(2.5f, .18f, .1f);
		Mat44f projCameraWorldP4 = projection * world2camera * Model2WorldP4;
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorldP4.v);
		glBindVertexArray(vaoPtr);
		glDrawArrays(GL_TRIANGLES, 0, sizeof(k2CubePositions));


		//engine1 
		glUniform1i(useTextureLoc, GL_FALSE);
		Mat44f Eng1Model2World = make_translation({ state.rocketXPosition + 0.015f, state.rocketYPosition - 0.235f, -5.065f }) * make_rotation_z(std::numbers::pi_v<float> / 2.f);//
		Mat44f projCameraWorldEng1 = projection * world2camera * Eng1Model2World;
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorldEng1.v);
		glBindVertexArray(vaoCylinder2);
		glDrawArrays(GL_TRIANGLES, 0, cylinderVertexCount2);
		glBindVertexArray(0);

		glBindVertexArray(vaoCone1);
		glDrawArrays(GL_TRIANGLES, 0, sizeof(kCubePositions));
		glBindVertexArray(0);

		//engine2		 
		Mat44f Model2WorldEng2 = make_translation({ state.rocketXPosition + 0.015f, state.rocketYPosition - 0.235f, -4.935f }) * make_rotation_z(std::numbers::pi_v<float> / 2.f);
		Mat44f projCameraWorldEng2 = projection * world2camera * Model2WorldEng2;
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorldEng2.v);
		glBindVertexArray(vaoCylinder2);
		glDrawArrays(GL_TRIANGLES, 0, cylinderVertexCount2);
		glBindVertexArray(0);
		glBindVertexArray(vaoCone1);
		glDrawArrays(GL_TRIANGLES, 0, coneVertexCount1);
		glBindVertexArray(0);

		//engine3	 
		Mat44f Model2WorldEng3 = make_translation({ state.rocketXPosition + 0.15f, state.rocketYPosition - 0.235f, -4.935f }) * make_rotation_z(std::numbers::pi_v<float> / 2.f);
		Mat44f projCameraWorldEng3 = projection * world2camera * Model2WorldEng3;
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorldEng3.v);
		glBindVertexArray(vaoCylinder2);
		glDrawArrays(GL_TRIANGLES, 0, cylinderVertexCount2);
		glBindVertexArray(0);
		glBindVertexArray(vaoCone1);
		glDrawArrays(GL_TRIANGLES, 0, coneVertexCount1);
		glBindVertexArray(0);
		//engine4		  
		Mat44f Model2WorldEng4 = make_translation({ state.rocketXPosition + 0.15f,state.rocketYPosition - 0.235f,  -5.065f }) * make_rotation_z(std::numbers::pi_v<float> / 2.f);
		Mat44f projCameraWorldEng4 = projection * world2camera * Model2WorldEng4;
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorldEng4.v);
		glBindVertexArray(vaoCylinder2);
		glDrawArrays(GL_TRIANGLES, 0, cylinderVertexCount2);
		glBindVertexArray(0);
		glBindVertexArray(vaoCone1);
		glDrawArrays(GL_TRIANGLES, 0, coneVertexCount1);
		glBindVertexArray(0);


		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers(window);
	}

	// Cleanup.
	//TODO: additional cleanup

	return 0;
}
catch (std::exception const& eErr)
{
	std::fprintf(stderr, "Top-level Exception (%s):\n", typeid(eErr).name());
	std::fprintf(stderr, "%s\n", eErr.what());
	std::fprintf(stderr, "Bye.\n");
	return 1;
}


namespace
{
	void glfw_callback_error_(int aErrNum, char const* aErrDesc)
	{
		std::fprintf(stderr, "GLFW error: %s (%d)\n", aErrDesc, aErrNum);
	}

	void glfw_callback_key_(GLFWwindow* aWindow, int aKey, int, int aAction, int)
	{
		if (GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction)
		{
			glfwSetWindowShouldClose(aWindow, GLFW_TRUE);
			return;
		}

		if (auto* state = static_cast<State_*>(glfwGetWindowUserPointer(aWindow)))
		{
			if (GLFW_KEY_C == aKey && aAction == GLFW_PRESS)
			{
				state->cameraMode = (state->cameraMode + 1) % 3; // Cycle through camera modes (0, 1, 2)

				if (state->cameraMode == 0)
					std::printf("Switched to Free Camera Mode\n");
				else if (state->cameraMode == 1)
					std::printf("Switched to Follow Rocket Camera Mode\n");
				else if (state->cameraMode == 2)
					std::printf("Switched to Ground-Fixed Camera Mode\n");
			}

			//Moving
			if (GLFW_KEY_W == aKey)
			{
				if (aAction == GLFW_PRESS)
					state->camControl.moveForward = true;
				else if (aAction == GLFW_RELEASE)
					state->camControl.moveForward = false;
			}
			if (GLFW_KEY_A == aKey)
			{
				if (aAction == GLFW_PRESS)
					state->camControl.moveLeft = true;
				else if (aAction == GLFW_RELEASE)
					state->camControl.moveLeft = false;
			}
			if (GLFW_KEY_S == aKey)
			{
				if (aAction == GLFW_PRESS)
					state->camControl.moveBackward = true;
				else if (aAction == GLFW_RELEASE)
					state->camControl.moveBackward = false;
				//std::fprintf(stderr, "S\n");
			}
			if (GLFW_KEY_D == aKey)
			{
				if (aAction == GLFW_PRESS)
					state->camControl.moveRight = true;
				else if (aAction == GLFW_RELEASE)
					state->camControl.moveRight = false;
			}
			if (GLFW_KEY_Q == aKey)
			{
				if (aAction == GLFW_PRESS)
					state->camControl.moveDown = true;
				else if (aAction == GLFW_RELEASE)
					state->camControl.moveDown = false;
			}
			if (GLFW_KEY_E == aKey)
			{
				if (aAction == GLFW_PRESS)
					state->camControl.moveUp = true;
				else if (aAction == GLFW_RELEASE)
					state->camControl.moveUp = false;
			}
			//Speed
			if (GLFW_KEY_LEFT_CONTROL == aKey)
			{
				if (aAction == GLFW_PRESS)
					state->camControl.speedDown = true;
				else if (aAction == GLFW_RELEASE)
					state->camControl.speedDown = false;
			}

			if (GLFW_KEY_LEFT_SHIFT == aKey)
			{
				if (aAction == GLFW_PRESS)
					state->camControl.speedUp = true;
				else if (aAction == GLFW_RELEASE)
					state->camControl.speedUp = false;
			}
			//fire
			if (auto* state = static_cast<State_*>(glfwGetWindowUserPointer(aWindow)))
			{
				// 其他按键处理...

				if (GLFW_KEY_F == aKey && aAction == GLFW_PRESS) {
					// 切换火箭飞行状态
					state->rocketFlying = !state->rocketFlying;
				}
			}

			if (GLFW_KEY_R == aKey && aAction == GLFW_PRESS)
			{
				state->rocketFlying = false;  // 停止飞行
				state->rocketYPosition = state->rocketInitialY;  // 重置高度
				state->rocketXPosition = state->rocketInitialX;  // 重置高度
				state->rocketFlyTime = 0;
			}


		}
	}

	void glfw_callback_motion_(GLFWwindow* aWindow, double aX, double aY)
	{
		if (auto* state = static_cast<State_*>(glfwGetWindowUserPointer(aWindow)))
		{
			if (state->camControl.cameraActive)
			{

				int width, height;
				glfwGetWindowSize(aWindow, &width, &height);
				double centerX = width / 2.0;
				double centerY = height / 2.0;


				auto const dx = float(aX - state->camControl.lastX);
				auto const dy = float(aY - state->camControl.lastY);


				state->camControl.phi += dx * kMouseSensitivity_;
				state->camControl.theta += dy * kMouseSensitivity_;


				if (state->camControl.theta > std::numbers::pi_v<float> / 2.f)
					state->camControl.theta = std::numbers::pi_v<float> / 2.f;
				else if (state->camControl.theta < -std::numbers::pi_v<float> / 2.f)
					state->camControl.theta = -std::numbers::pi_v<float> / 2.f;

				// re-place 
				if (std::abs(aX - centerX) > 1 || std::abs(aY - centerY) > 1)
				{
					glfwSetCursorPos(aWindow, centerX, centerY);
					state->camControl.lastX = float(centerX);
					state->camControl.lastY = float(centerY);
				}
				else
				{
					state->camControl.lastX = float(aX);
					state->camControl.lastY = float(aY);
				}
			}
		}
	}
	void glfw_cb_button_(GLFWwindow* aWindow, int aButton, int aAction, int)
	{
		if (auto* state = static_cast<State_*>(glfwGetWindowUserPointer(aWindow)))
		{
			if (aAction == GLFW_PRESS && aButton == GLFW_MOUSE_BUTTON_RIGHT) {
				state->camControl.cameraActive = !state->camControl.cameraActive;

				if (state->camControl.cameraActive)
				{
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

					int width, height;
					glfwGetWindowSize(aWindow, &width, &height);
					glfwSetCursorPos(aWindow, width / 2.0, height / 2.0);

					state->camControl.lastX = width / 2.0f;
					state->camControl.lastY = height / 2.0f;
				}
				else
				{
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
			}
		}
	}


}

namespace
{
	GLFWCleanupHelper::~GLFWCleanupHelper()
	{
		glfwTerminate();
	}

	GLFWWindowDeleter::~GLFWWindowDeleter()
	{
		if (window)
			glfwDestroyWindow(window);
	}

}
