#pragma once

#include <components/simple_scene.h>
#include <fstream>

#include <imgui/display_imgui.h>
#include <imgui/editor_imgui.h>
#include <mode_standard.h>
#include <mode_vim.h>
#include <tab_window.h>
#include <theme.h>
#include <window.h>

#include <imgui/imgui_freetype.h>

using std::fstream;

struct ZepContainer;

namespace gui
{
class Lab1 : public gfxc::SimpleScene {
    public:
	Lab1();
	~Lab1();

	void Init() override;

    private:
	void FrameStart() override;
	void Update(float deltaTimeSeconds) override;
	void FrameEnd() override;

	void OnInputUpdate(float deltaTime, int mods) override;
	void OnKeyPress(int key, int mods) override;
	void OnKeyRelease(int key, int mods) override;
	void OnMouseMove(int mouseX, int mouseY, int deltaX,
			 int deltaY) override;
	void OnMouseBtnPress(int mouseX, int mouseY, int button,
			     int mods) override;
	void OnMouseBtnRelease(int mouseX, int mouseY, int button,
			       int mods) override;
	void OnMouseScroll(int mouseX, int mouseY, int offsetX,
			   int offsetY) override;
	void OnWindowResize(int width, int height) override;

	ImGuiWindowFlags fpsWindowFlags;
	bool showZep;
	const char *fileToEdit;
	bool quit;
	fstream file;
	ZepContainer *zep;
};
} // namespace gui
