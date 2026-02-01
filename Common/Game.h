#pragma once
#include <glm/vec2.hpp>
#include <vector>

#ifdef WINDOWS_BUILD
//include glad and glfw for Windows build
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#else
#include <GLES2/gl2.h>
#endif

#ifdef Raspberry_BUILD
#include <GLES3/gl3.h>
#endif

#include <imgui.h>
#include <chrono>

#ifdef WINDOWS_BUILD
#include <backends\imgui_impl_glfw.h>
#include "WindowsGraphics.h"
#endif

#include <backends/imgui_impl_opengl3.h>

#include "block.h"
#include "player.h"
#include "chunk.h"
#include "world.h"
#include "ui_manager.h"
#include "audiomanager.h"
#include "inventory.h"
#include "sky_renderer.h"

class IGraphics;
class Input;

constexpr unsigned int WINDOW_WIDTH = 1920;
constexpr unsigned int WINDOW_HEIGHT = 1080;
constexpr float ASPECT_RATIO = WINDOW_WIDTH / WINDOW_HEIGHT;

class Game
{
public:
	Game(const Input* const input, IGraphics* graphics);
	virtual ~Game();
	void Start();
	void ImGUI(float fps);
	const Input& GetInput() const;
	void Quit();
protected:
	void ProcessInput();
	bool KeyPressedOnce(Key key, bool& keyUp);
	bool MousePressedOnce(MouseButtons mouseButton, bool& mouseButtonUp);
	void LockMouseWindows();
	void DayNightCycle();

	virtual void Update(float /*gameDeltaTime*/) {}
	virtual void Render() {}
	virtual void PostRender() {}
	
	const Input* const input;
	bool quitting{false};
	float gameDeltaTime;

	IGraphics* graphics;

private:
	void InitializeOpenGLES();
	void ClearScreen();

	int frameCount{0};

	bool enable_debugging = true;

	Player m_player;
	Chunk m_chunk;
	World m_world;
	Inventory m_inventory;
	UI_Manager m_ui_manager;
	AudioManager m_audiomanager;
	Sky_Renderer m_sky_renderer;

	#pragma region KeyUp & MouseButtonUp
	// MouseButtons
	bool middleMouseUp = false;

	// Keys
	bool keyUpF3 = false;
	bool keyUpE = false;
	bool keyUpSpace = false;

	// Numbers
	bool keyUp1 = false;
	bool keyUp2 = false;
	bool keyUp3 = false;
	bool keyUp4 = false;
	bool keyUp5 = false;
	bool keyUp6 = false;
	bool keyUp7 = false;
	bool keyUp8 = false;
	bool keyUp9 = false;
	#pragma endregion

	// Input Timers
	static constexpr std::chrono::milliseconds DESTROY_INTERVAL{250};
	std::chrono::steady_clock::time_point destroyBlockTimer = std::chrono::steady_clock::now();

	static constexpr std::chrono::milliseconds PLACE_INTERVAL{200};
	std::chrono::steady_clock::time_point placeBlockTimer = std::chrono::steady_clock::now();
	
	// When you hit your head before you can jump again
	static constexpr std::chrono::milliseconds HEADHITTER_INTERVAL{500};
	std::chrono::steady_clock::time_point headhitterTimer = std::chrono::steady_clock::now();
	
	static constexpr std::chrono::milliseconds DROP_ITEM_INTERVAL{ 50 };
	std::chrono::steady_clock::time_point dropItemTimer = std::chrono::steady_clock::now();

	// Used to toggle fly when in creative
	std::chrono::steady_clock::time_point lastJumpPress{};

	#pragma region Day Night Cycle
	// Sky color
	glm::vec3 skyColor = glm::vec3(0.47f, 0.66f, 1.0f);
	// Day duration in seconds
	float day_duration = 1200.0f;
	// Keeps going from 0 to 1 back to 0, repeat (night = 0, day = 1)
	float dayFactor = 1.0f;
	// Day / Night Timer
	std::chrono::steady_clock::time_point dayNightTimer = std::chrono::steady_clock::now();

	// Sun Direction
	glm::vec3 sunDirection = glm::vec3(0.5f, 1.0f, 0.5f);
	float sunAngle = 0.0f;
	#pragma endregion

	// Block Outline
	VAO m_outlineVAO;
	VBO m_outlineVBO;
};