#pragma once
#include "IInput.h"

#include <X11/Xlib.h>

class RaspMouse : public IMouse
{
public:
	RaspMouse(Display& display, Window& window);

	bool GetButtonDown(MouseButtons button) const override;
	glm::vec2 GetPosition() const override;
	float GetScrollDelta() const override;

	float CheckScrollDelta() const override;

private:
	Display& display ;
	Window& window ; 

	static float scrollDelta;
};

