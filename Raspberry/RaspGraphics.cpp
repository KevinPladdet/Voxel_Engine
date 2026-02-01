#include "RaspGraphics.h"
#include <EGL/egl.h>
#include "EGLState.h"
#include "RaspMouse.h"
#include "XWindow.h"

#include "ImGui/imgui_impl_x11.h" // Makes ImGui compatible with directx11, made with claude ai
#include <ImGui-master/backends/imgui_impl_opengl3.h>

RaspGraphics::RaspGraphics() : window(new XWindow())
{
	window->CreateWindow();
}

void RaspGraphics::Quit()
{
	XDestroyWindow(&window->GetDisplay(), window->GetWindow());
}

void RaspGraphics::SwapBuffer()
{
	EGLState state = window->GetState();
	eglSwapBuffers(state.display, state.surface);
}

void RaspGraphics::LockMouse() {}
void RaspGraphics::UnlockMouse() {}


void RaspGraphics::InitImGUI()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 1.3f; // Increase ImGui font for UI by 30%
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

	ImGui_ImplX11_Init(&window->GetDisplay(), window->GetWindow());

	// Setup Platform/Renderer backends
	//ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init("#version 100");
}

void RaspGraphics::BeginFrameImGUI() 
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplX11_NewFrame();
	ImGui::NewFrame();
}
void RaspGraphics::EndFrameImGUI()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void RaspGraphics::ShutDownImGUI()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplX11_Shutdown();
	ImGui::DestroyContext();
}

XWindow& RaspGraphics::Window() const
{
	return *window;
}
