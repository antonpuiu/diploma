#pragma once

#include "window/input_controller.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

class World : public InputController {
    public:
	World();
	virtual ~World()
	{
	}
	virtual void Init()
	{
	}
	virtual void FrameStart()
	{
	}
	virtual void Update(float deltaTimeSeconds)
	{
	}
	virtual void FrameEnd()
	{
	}

	void Run();
	void Pause();
	void Exit();

	double GetLastFrameTime();

    private:
	void ComputeFrameDeltaTime();
	void LoopUpdate();

    private:
	double previousTime;
	double elapsedTime;
	double deltaTime;
	bool paused;
	bool shouldClose;
};
