#pragma once

#include <components/simple_scene.h>
#include <fstream>

#include <zep/filesystem.h>
#include <zep/imgui/display_imgui.h>
#include <zep/imgui/editor_imgui.h>
#include <zep/mode_standard.h>
#include <zep/mode_vim.h>
#include <zep/regress.h>
#include <zep/tab_window.h>
#include <zep/theme.h>
#include <zep/window.h>

#include "zep/filesystem.h"
#include "zep/imgui/display_imgui.h"
#include "zep/imgui/editor_imgui.h"
#include "zep/mode_standard.h"
#include "zep/mode_vim.h"
#include "zep/tab_window.h"
#include "zep/theme.h"
#include "zep/window.h"
#include <zep/regress.h>

#include <imgui/imgui_freetype.h>

using std::fstream;

struct ZepContainerImGui;

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
	bool showFPS;
	const char *fileToEdit;
	bool quit;
	fstream file;
	ZepContainerImGui *zep;
};
} // namespace gui
