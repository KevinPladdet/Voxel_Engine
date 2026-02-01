#include "Input.h"

#include "IInput.h"

Input::Input(const IKeyboard* const keyboard, const IMouse* const mouse) :
	mouse(mouse),
	keyboard(keyboard)
{
}

Input::~Input()
{
	delete(mouse);
	delete(keyboard);
}


const IKeyboard& Input::GetKeyboard() const
{
	return *keyboard;
}

const IMouse& Input::GetMouse() const
{
	return *mouse;
}
