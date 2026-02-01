#pragma once
// 2 input classes
#include "IInput.h"
#include <GLFW/glfw3.h>

#include <iostream>

class WindowsKeyboard : public IKeyboard
{
public:
	WindowsKeyboard(GLFWwindow& window);
	bool GetKey(Key key) const override;

private:
	GLFWwindow& window;
};


class WindowsMouse : public IMouse
{
public:
	WindowsMouse(GLFWwindow& window);

	bool GetButtonDown(MouseButtons button) const override;
	glm::vec2 GetPosition() const override;
	float GetScrollDelta() const override;

	float CheckScrollDelta() const override;

private:
	GLFWwindow& window;

	static float scrollDelta;
	static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
};
