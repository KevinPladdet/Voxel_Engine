#pragma once
#include "IGraphics.h"

class XWindow;
struct EGLState;

class RaspGraphics : public IGraphics
{
public:
	RaspGraphics();
	~RaspGraphics() {};

	void Quit() override;

	void SwapBuffer() override;

	void LockMouse() override;
	void UnlockMouse() override;

	void InitImGUI() override;
	void BeginFrameImGUI() override;
	void EndFrameImGUI() override;
	void ShutDownImGUI() override;

	XWindow& Window() const;

private:
	XWindow* window;
};