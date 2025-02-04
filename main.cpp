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


namespace
{

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

	glfwWindowHint(GLFW_DEPTH_BITS, 24);



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
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f); // RGBA values for grey
	// 
	// TODO: global GL setup goes here
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f); // RGBA values for grey
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	OGL_CHECKPOINT_ALWAYS();

	//VAO
	auto model = load_wavefront_obj("assets/cw2/langerso.obj");
	GLuint vaoModels = create_vao(model);
	std::size_t vertexCounterM = model.positions.size();

	auto modelRocket = load_wavefront_obj("assets/cw2/rocket.obj");
	GLuint vaoModelRocket = create_vao(modelRocket);
	std::size_t vertexCounterMRocket = model.positions.size();


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
		Mat44f T = make_translation({ -state.camControl.posX, -state.camControl.posY, -state.camControl.posZ });
		Mat44f Rx = make_rotation_x(state.camControl.theta);
		Mat44f Ry = make_rotation_y(state.camControl.phi);
		Mat44f world2camera = Rx * Ry * T;






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



		GLint projCameraWorldLoc = glGetUniformLocation(prog.programId(), "uProjCameraWorld");
		//glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorld.v);

		Mat33f normalMatrix = mat44_to_mat33(transpose(invert(model2world)));
		GLint uNormalMatrixLoc = glGetUniformLocation(prog.programId(), "uNormalMatrix");
		glUniformMatrix3fv(uNormalMatrixLoc // uNormalMatrixLoc = 1
			, 1, GL_TRUE, normalMatrix.v);

		// land draw
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		//lUniform1i(glGetUniformLocation(prog.programId(), "uTexture"), 0);
		glUniform1i(glGetUniformLocation(prog.programId(), "useTexture"), GL_TRUE); 
		Mat44f langersoModel2World = make_translation({ 0, 0, 0 });
		Mat44f projCameraWorldLangerso = projection * world2camera * langersoModel2World;
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorldLangerso.v);

		glBindVertexArray(vaoModels);
		glDrawArrays(GL_TRIANGLES, 0, vertexCounterM);
		glBindVertexArray(0);

		//rocket draw 
		Mat44f rocketModel2World = make_translation({ 0.0f, 5.0f, -10.0f }) ;
		Mat44f projCameraWorldRocket = projection * world2camera * rocketModel2World;
		glUniformMatrix4fv(projCameraWorldLoc, 1, GL_TRUE, projCameraWorldRocket.v);		
		//no texture
		glUniform1i(glGetUniformLocation(prog.programId(), "useTexture"), GL_FALSE); // 
		glBindVertexArray(vaoModelRocket);
		glDrawArrays(GL_TRIANGLES, 0, vertexCounterMRocket);
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

