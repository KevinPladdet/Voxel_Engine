// CrossPiProject.cpp : This file contains the PC 'main' function. Program execution begins and ends there.
//
#include "Input.h"
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include "Game.h"
#include "WindowsGraphics.h"
#include "WindowsInput.h"

#ifdef WINDOWS_BUILD
// in case of a laptop with poor on board GPU, lets make sure we switch on any power gpu's we have to get max compatability
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

extern "C"
{
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif


//
WindowsGraphics* WinGraphics;
Input* input;
Game* game;

// Mains only purpose is to set up OGL and then jump to general game code
int main()
{
	WinGraphics = new WindowsGraphics();
	glfwSwapInterval(1); // stop the windows build updating without vblank so its the same speed as pi



	input = new Input(new WindowsKeyboard(WinGraphics->Window()), new WindowsMouse(WinGraphics->Window()));

	game = new Game(input, WinGraphics);

	game->Start();

	delete WinGraphics;
	delete input;
	delete game;

	return 0;
}

void framebufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
	glViewport(0, 0, width, height);
}
