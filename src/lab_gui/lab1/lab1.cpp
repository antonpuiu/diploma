#include "lab1.h"

#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include <fstream>
#include <iostream>
#include <vector>

#define ZEP_ROOT "/home/kn07/Documents/git/zep-imgui/zep"

using std::make_unique;

using namespace std;
using namespace gui;
using namespace Zep;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */

// NVec2f GetPixelScale()
// {
// 	float scaleX, scaleY;
// 	GLFWwindow *window = glfwGetCurrentContext();

// 	if (window == nullptr)
// 		return NVec2f(1.0f);

// 	glfwGetWindowContentScale(window, &scaleX, &scaleY);

// 	return Zep::NVec2f(scaleX, scaleY) / 96.0f;
// }

struct ZepContainer : public IZepComponent {
	ZepContainer(const std::string &startupFilePath)
		: spEditor(std::make_unique<ZepEditor_ImGui>(ZEP_ROOT))
	{
		// File watcher not used on apple yet ; needs investigating as to why it doesn't compile/run
		// #ifndef __APPLE__
		// 		MUtils::Watcher::Instance().AddWatch(
		// 			ZEP_ROOT,
		// 			[&](const ZepPath &path) {
		// 				spEditor->OnFileChanged(path);
		// 			},
		// 			false);
		// #endif

		spEditor->RegisterCallback(this);

		float scaleX, scaleY;
		GLFWwindow *window = glfwGetCurrentContext();

		glfwGetWindowContentScale(window, &scaleX, &scaleY);

		spEditor->SetPixelScale(scaleX / 96.0f);

		if (!startupFilePath.empty()) {
			spEditor->InitWithFileOrDir(startupFilePath);
		} else {
			spEditor->InitWithText("Empty file", string());
		}
	}

	ZepContainer()
	{
		spEditor->UnRegisterCallback(this);
	}

	// Inherited via IZepComponent
	virtual void Notify(std::shared_ptr<ZepMessage> message) override
	{
		if (message->messageId == Msg::Tick) {
			// #ifndef __APPLE__
			// 			MUtils::Watcher::Instance().Update();
			// #endif
		} else if (message->messageId == Msg::Quit) {
			quit = true;
		} else if (message->messageId == Msg::ToolTip) {
			auto spTipMsg =
				std::static_pointer_cast<ToolTipMessage>(
					message);
			if (spTipMsg->location != -1l && spTipMsg->pBuffer) {
				auto pSyntax = spTipMsg->pBuffer->GetSyntax();
				if (pSyntax) {
					if (pSyntax->GetSyntaxAt(
						    spTipMsg->location) ==
					    ThemeColor::Identifier) {
						auto spMarker =
							std::make_shared<
								RangeMarker>();
						spMarker->description =
							"This is an identifier";
						spMarker->highlightColor =
							ThemeColor::Identifier;
						spMarker->textColor =
							ThemeColor::Text;
						spTipMsg->spMarker = spMarker;
						spTipMsg->handled = true;
					} else if (pSyntax->GetSyntaxAt(
							   spTipMsg->location) ==
						   ThemeColor::Keyword) {
						auto spMarker =
							std::make_shared<
								RangeMarker>();
						spMarker->description =
							"This is a keyword";
						spMarker->highlightColor =
							ThemeColor::Keyword;
						spMarker->textColor =
							ThemeColor::Text;
						spTipMsg->spMarker = spMarker;
						spTipMsg->handled = true;
					}
				}
			}
		}
	}

	virtual ZepEditor &GetEditor() const override
	{
		return *spEditor;
	}

	bool quit = false;
	std::unique_ptr<ZepEditor_ImGui> spEditor;
};

Lab1::Lab1()
{
	fpsWindowFlags = 0;
	showZep = true;
	quit = false;
}

Lab1::~Lab1()
{
}

void Lab1::Init()
{
	// Load a mesh from file into GPU memory. We only need to do it once,
	// no matter how many times we want to draw this mesh.
	{
		Mesh *mesh = new Mesh("box");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir,
					 RESOURCE_PATH::MODELS, "primitives"),
			       "box.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	fpsWindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			 ImGuiWindowFlags_NoScrollbar |
			 ImGuiWindowFlags_NoCollapse |
			 ImGuiWindowFlags_AlwaysAutoResize;

	zep = new ZepContainer("");
}

void Lab1::FrameStart()
{
}

void Lab1::Update(float deltaTimeSeconds)
{
	glm::ivec2 resolution = window->props.resolution;

	// Sets the clear color for the color buffer
	glClearColor(0, 0, 0, 1);

	// Clears the color buffer (using the previously set color) and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);

	// ImGui::ShowDemoWindow();

	// if (showFPS) {
	// 	ImGui::Begin("FPS", &showFPS, fpsWindowFlags);
	// 	std::string fps_text =
	// 		"FPS: " +
	// 		to_string(static_cast<int>(1.0f / deltaTimeSeconds));

	// 	ImGui::Text("%s", fps_text.c_str());

	// 	ImGui::End();
	// }

	if (showZep) {
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open")) {
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Settings")) {
				if (ImGui::BeginMenu("Editor Mode")) {
					bool enabledVim =
						strcmp(zep->GetEditor()
							       .GetCurrentMode()
							       ->Name(),
						       Zep::ZepMode_Vim::
							       StaticName()) ==
						0;
					bool enabledNormal = !enabledVim;
					if (ImGui::MenuItem("Vim", "CTRL+2",
							    &enabledVim)) {
						zep->GetEditor().SetMode(
							Zep::ZepMode_Vim::
								StaticName());
					} else if (ImGui::MenuItem(
							   "Standard", "CTRL+1",
							   &enabledNormal)) {
						zep->GetEditor().SetMode(
							Zep::ZepMode_Standard::
								StaticName());
					}
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Theme")) {
					bool enabledDark =
						zep->GetEditor().GetTheme()
									.GetThemeType() ==
								ThemeType::Dark ?
							      true :
							      false;
					bool enabledLight = !enabledDark;

					if (ImGui::MenuItem("Dark", "",
							    &enabledDark)) {
						zep->GetEditor()
							.GetTheme()
							.SetThemeType(
								ThemeType::Dark);
					} else if (ImGui::MenuItem(
							   "Light", "",
							   &enabledLight)) {
						zep->GetEditor()
							.GetTheme()
							.SetThemeType(
								ThemeType::Light);
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window")) {
				auto pTabWindow =
					zep->GetEditor().GetActiveTabWindow();
				if (ImGui::MenuItem("Horizontal Split")) {
					pTabWindow->AddWindow(
						&pTabWindow->GetActiveWindow()
							 ->GetBuffer(),
						pTabWindow->GetActiveWindow(),
						false);
				} else if (ImGui::MenuItem("Vertical Split")) {
					pTabWindow->AddWindow(
						&pTabWindow->GetActiveWindow()
							 ->GetBuffer(),
						pTabWindow->GetActiveWindow(),
						true);
				}
				ImGui::EndMenu();
			}

			// Helpful for diagnostics
			// Make sure you run a release build; iterator debugging makes the debug build much slower
			// Currently on a typical file, editor display time is < 1ms, and editor editor time is < 2ms
			if (ImGui::BeginMenu("Timings")) {
				for (auto &p : globalProfiler.timerData) {
					std::ostringstream strval;
					strval << p.first << " : "
					       << p.second.current / 1000.0
					       << "ms"; // << " Last: " << p.second.current / 1000.0 << "ms";
					ImGui::MenuItem(strval.str().c_str());
				}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		int w, h;
		w = resolution.x;
		h = resolution.y;

		// This is a bit messy; and I have no idea why I don't need to remove the menu fixed_size from the calculation!
		// The point of this code is to fill the main window with the Zep window
		// It is only done once so the user can play with the window if they want to for testing
		// auto menuSize =
		// 	ImGui::GetStyle().FramePadding.y * 2 + ImGui::GetFontSize();
		// ImGui::SetNextWindowPos(ImVec2(0, menuSize), ImGuiCond_Always);
		// ImGui::SetNextWindowSize(ImVec2(float(w), float(h - menuSize)),
		// 			 ImGuiCond_Always);

		// ImGui::PushStyleColor(ImGuiCol_WindowBg,
		// 		      ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		// ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		// ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		// ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		// ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		// ImGui::Begin(
		// 	"Zep", nullptr,
		// 	ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		// 		ImGuiWindowFlags_NoResize |
		// 		ImGuiWindowFlags_NoTitleBar /*| ImGuiWindowFlags_NoScrollbar*/);
		// // ImGui::Begin("Zep", nullptr,
		// // 	     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
		// // 		     ImGuiWindowFlags_NoScrollbar);

		// auto min = ImGui::GetCursorScreenPos();
		// auto max = ImGui::GetContentRegionAvail();
		// max.x = std::max(1.0f, max.x);
		// max.y = std::max(1.0f, max.y);

		// // Fill the window
		// max.x = min.x + max.x;
		// max.y = min.y + max.y;
		// zep->spEditor->SetDisplayRegion(Zep::NVec2f(min.x, min.y),
		// 				Zep::NVec2f(max.x, max.y));

		// // Display the editor inside this window
		// zep->spEditor->Display();
		// zep->spEditor->HandleInput();
		// ImGui::End();
		// ImGui::PopStyleVar(4);
		// ImGui::PopStyleColor(1);

		auto menuSize = ImGui::GetStyle().FramePadding.y * 2 +
				ImGui::GetFontSize();
		ImGui::SetNextWindowPos(ImVec2(0, menuSize));
		ImGui::SetNextWindowSize(
			ImVec2(float(w), float(h))); // -menuSize)));

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
				    ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::Begin(
			"Zep", nullptr,
			ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoTitleBar /*| ImGuiWindowFlags_NoScrollbar*/);
		ImGui::PopStyleVar(4);

		// 'hide' the window contents from ImGui, so it doesn't try dragging when we move our scrollbar, etc.
		ImGui::InvisibleButton("ZepContainer", ImGui::GetWindowSize());

		// TODO: Change only when necessray
		zep->spEditor->SetDisplayRegion(
			toNVec2f(ImGui::GetWindowPos()),
			toNVec2f(ImGui::GetWindowSize()));

		// Display the editor inside this window
		zep->spEditor->Display();
		zep->spEditor->HandleInput();

		ImGui::End();
	}
}

void Lab1::FrameEnd()
{
	// DrawCoordinateSystem();
}

/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */

void Lab1::OnInputUpdate(float deltaTime, int mods)
{
	// Treat continuous update based on input
}

void Lab1::OnKeyPress(int key, int mods)
{
	// Add key press event
	if (key == GLFW_KEY_F) {
		showZep = !showZep;
	}
}

void Lab1::OnKeyRelease(int key, int mods)
{
	// Add key release event
}

void Lab1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// Add mouse move event
}

void Lab1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// Add mouse button press event
}

void Lab1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// Add mouse button release event
}

void Lab1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
	// Treat mouse scroll event
}

void Lab1::OnWindowResize(int width, int height)
{
	// Treat window resize event
}
