#pragma once


class IMouse;
class IKeyboard;

class Input
{
public:
	Input(const IKeyboard* const keyboard, const IMouse* const mouse);
	~Input();

	const IKeyboard& GetKeyboard() const;
	const IMouse& GetMouse() const;

private:
	const IMouse* const mouse;
	const IKeyboard* const keyboard;
};

