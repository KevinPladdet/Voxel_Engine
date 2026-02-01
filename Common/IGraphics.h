// a simple interface class its basically just simple access to graphics
#pragma once
class IGraphics
{
public:
	virtual ~IGraphics() = default;
	virtual void SwapBuffer() = 0;
	virtual void Quit() = 0;

	virtual void LockMouse() = 0;
	virtual void UnlockMouse() = 0;

	virtual void InitImGUI() = 0;
	virtual void BeginFrameImGUI() = 0;
	virtual void EndFrameImGUI() = 0;
	virtual void ShutDownImGUI() = 0;
};