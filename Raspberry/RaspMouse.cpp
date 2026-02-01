#include "RaspMouse.h"

RaspMouse::RaspMouse(Display& display, Window& window) :
	display(display),
	window(window)
{
}

bool RaspMouse::GetButtonDown(MouseButtons button) const
{
	int root_x;
	int root_y;
	int win_x;
	int win_y;
	uint mask_return;
	Window window_returned;
	XQueryPointer(
		&display,
		window,
		&window_returned,
		&window_returned,

		&root_x,
		&root_y,
		&win_x,
		&win_y,
		&mask_return);

	switch (button) 
	{
		case MouseButtons::LEFT:
		{
			return mask_return & Button1MotionMask;
		}
		case MouseButtons::RIGHT:
		{
			return mask_return & Button3MotionMask;
		}
		case MouseButtons::MIDDLE:
		{
			return mask_return & Button2MotionMask;
		}
		default:
		{
			return false;
		}
	}
}

glm::vec2 RaspMouse::GetPosition() const
{
	int root_x;
	int root_y;
	int win_x;
	int win_y;
	uint mask_return;
	Window window_returned;
	XQueryPointer(
		&display,
		window,
		&window_returned,
		&window_returned,

		&root_x,
		&root_y,
		&win_x,
		&win_y,
		&mask_return);

	return glm::vec2{win_x, win_y};
}

float RaspMouse::GetScrollDelta() const
{
	return 0;
}

float RaspMouse::CheckScrollDelta() const
{
	float d = scrollDelta;
	scrollDelta = 0.0f;
	return d;
}

float RaspMouse::scrollDelta{ 0.0f };
