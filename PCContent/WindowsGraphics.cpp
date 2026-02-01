#include "IGraphics.h"
#include "WindowsGraphics.h"
#include "Game.h"
#include <iostream>
#include <glad/glad.h>
#include "WindowsInput.h"

void WindowsGraphics::SwapBuffer()
{
	glFlush();
	glfwSwapBuffers(window);
	glfwPollEvents();
}

GLFWwindow& WindowsGraphics::Window() const
{
	return *window;
}

WindowsGraphics::WindowsGraphics()
{
	// Initialize GLFW and set window properties.
	glfwInit();
	glfwWindowHint(GL_DEPTH_BUFFER_BIT, 16);
	glfwWindowHint(GL_DEPTH_BITS, 16);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // we need a base OpenGL 3.3 to emulate ES, otherwise use 3.1 for ES
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // for normal opengl
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
	// Creates the window.
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Voxel Engine", NULL, NULL);

	// Error handling for if window creation failed.
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window!" << std::endl;
		glfwTerminate();
	}

	// Set the window to be the current context.
	glfwMakeContextCurrent(window);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Error handling for if GLAD failed to initialize.
	if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD!" << std::endl;
	}
}

void WindowsGraphics::Quit()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void WindowsGraphics::LockMouse()
{
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void WindowsGraphics::UnlockMouse()
{
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Set mouse to center
	glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
}

void WindowsGraphics::InitImGUI()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 1.3f; // Increase ImGui font for UI by 30%
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init("#version 100");
}

void WindowsGraphics::BeginFrameImGUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	//ImGui::ShowDemoWindow();
}

void WindowsGraphics::EndFrameImGUI()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void WindowsGraphics::ShutDownImGUI()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}