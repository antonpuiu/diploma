#include "lab_gui/lab1/lab1.h"

#include "GLFW/glfw3.h"
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
using namespace gui;
using namespace Zep;

const std::string shader = "\
#version 330 core\
\
	uniform mat4 Projection;\
\
// Coordinates  of the geometry\
layout(location = 0) in vec3 in_position;\
layout(location = 1) in vec2 in_tex_coord;\
layout(location = 2) in vec4 in_color;\
\
// Outputs to the pixel shader\
out vec2 frag_tex_coord;\
out vec4 frag_color;\
\
void main()\
{\
	gl_Position = Projection * vec4(in_position.xyz, 1.0);\
	frag_tex_coord = in_tex_coord;\
	frag_color = in_color;\
}\
)";

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */

NVec2f GetPixelScale()
{
	float scaleX, scaleY;
	GLFWwindow *window = glfwGetCurrentContext();

	if (window == nullptr)
		return NVec2f(1.0f);

	glfwGetWindowContentScale(window, &scaleX, &scaleY);

	return Zep::NVec2f(scaleX, scaleY) / 96.0f;
}

struct ZepContainerImGui : public IZepComponent {
	ZepContainerImGui(const std::string &startupFilePath,
			  const std::string &configPath)
		: spEditor(std::make_unique<ZepEditor_ImGui>(configPath,
							     GetPixelScale()))
	//, fileWatcher(spEditor->GetFileSystem().GetConfigPath(), std::chrono::seconds(2))
	{
		// ZepEditor_ImGui will have created the fonts for us; but we need to build
		// the font atlas
		auto fontPath =
			PATH_JOIN(RESOURCE_PATH::FONTS, "Cousine-Regular.ttf");
		auto &display =
			static_cast<ZepDisplay_ImGui &>(spEditor->GetDisplay());
		int fontPixelHeight = 15;

		auto &io = ImGui::GetIO();
		ImVector<ImWchar> ranges;
		ImFontGlyphRangesBuilder builder;
		builder.AddRanges(
			io.Fonts->GetGlyphRangesDefault()); // Add one of the default ranges
		builder.AddRanges(
			io.Fonts->GetGlyphRangesCyrillic()); // Add one of the default ranges
		builder.AddRanges(greek_range);
		builder.BuildRanges(
			&ranges); // Build the final result (ordered ranges with all the unique characters submitted)

		ImFontConfig cfg;
		cfg.OversampleH = 4;
		cfg.OversampleV = 4;

		// auto pImFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(
		// 	fontPath.c_str(), float(fontPixelHeight), &cfg,
		// 	ranges.Data);

		// display.SetFont(ZepTextType::UI,
		// 		std::make_shared<ZepFont_ImGui>(
		// 			display, pImFont, fontPixelHeight));
		// display.SetFont(ZepTextType::Text,
		// 		std::make_shared<ZepFont_ImGui>(
		// 			display, pImFont, fontPixelHeight));
		// display.SetFont(ZepTextType::Heading1,
		// 		std::make_shared<ZepFont_ImGui>(
		// 			display, pImFont,
		// 			int(fontPixelHeight * 1.75)));
		// display.SetFont(ZepTextType::Heading2,
		// 		std::make_shared<ZepFont_ImGui>(
		// 			display, pImFont,
		// 			int(fontPixelHeight * 1.5)));
		// display.SetFont(ZepTextType::Heading3,
		// 		std::make_shared<ZepFont_ImGui>(
		// 			display, pImFont,
		// 			int(fontPixelHeight * 1.25)));

		unsigned int flags = 0; // ImGuiFreeType::NoHinting;
		ImGuiFreeType::BuildFontAtlas(ImGui::GetIO().Fonts, flags);

		spEditor->RegisterCallback(this);

		ZepRegressExCommand::Register(*spEditor);

		if (!startupFilePath.empty()) {
			spEditor->InitWithFileOrDir(startupFilePath);
		} else {
			spEditor->InitWithText("Shader.vert", shader);
		}

		// File watcher not used on apple yet ; needs investigating as to why it doesn't compile/run
		// The watcher is being used currently to update the config path, but clients may want to do more interesting things
		// by setting up watches for the current dir, etc.
		/*fileWatcher.start([=](std::string path, FileStatus status) {
            if (spEditor)
            {
                ZLOG(DBG, "Config File Change: " << path);
                spEditor->OnFileChanged(spEditor->GetFileSystem().GetConfigPath() / path);
            }
        });*/
	}

	~ZepContainerImGui()
	{
	}

	void Destroy()
	{
		spEditor->UnRegisterCallback(this);
		spEditor.reset();
	}

	// Inherited via IZepComponent
	virtual void Notify(std::shared_ptr<ZepMessage> message) override
	{
		if (message->messageId == Msg::GetClipBoard) {
			// clip::get_text(message->str);
			message->handled = true;
		} else if (message->messageId == Msg::SetClipBoard) {
			// clip::set_text(message->str);
			message->handled = true;
		} else if (message->messageId == Msg::RequestQuit) {
			quit = true;
		} else if (message->messageId == Msg::ToolTip) {
			auto spTipMsg =
				std::static_pointer_cast<ToolTipMessage>(
					message);
			if (spTipMsg->location.Valid() && spTipMsg->pBuffer) {
				auto pSyntax = spTipMsg->pBuffer->GetSyntax();
				if (pSyntax) {
					if (pSyntax->GetSyntaxAt(
							   spTipMsg->location)
						    .foreground ==
					    ThemeColor::Identifier) {
						auto spMarker = std::make_shared<
							RangeMarker>(
							*spTipMsg->pBuffer);
						spMarker->SetDescription(
							"This is an identifier");
						spMarker->SetHighlightColor(
							ThemeColor::Identifier);
						spMarker->SetTextColor(
							ThemeColor::Text);
						spTipMsg->spMarker = spMarker;
						spTipMsg->handled = true;
					} else if (pSyntax->GetSyntaxAt(
								  spTipMsg->location)
							   .foreground ==
						   ThemeColor::Keyword) {
						auto spMarker = std::make_shared<
							RangeMarker>(
							*spTipMsg->pBuffer);
						spMarker->SetDescription(
							"This is a keyword");
						spMarker->SetHighlightColor(
							ThemeColor::Keyword);
						spMarker->SetTextColor(
							ThemeColor::Text);
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
	//FileWatcher fileWatcher;
};

Lab1::Lab1()
{
	fpsWindowFlags = 0;
	showFPS = true;
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

	zep = new ZepContainerImGui(string(), string());
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

	ImGui::ShowDemoWindow();

	// if (showFPS) {
	// 	ImGui::Begin("FPS", &showFPS, fpsWindowFlags);
	// 	std::string fps_text =
	// 		"FPS: " +
	// 		to_string(static_cast<int>(1.0f / deltaTimeSeconds));

	// 	ImGui::Text("%s", fps_text.c_str());

	// 	ImGui::End();
	// }

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open")) {
				// TODO
				// auto openFileName =
				// 	tinyfd_openFileDialog("Choose a file",
				// 			      "", 0, nullptr,
				// 			      nullptr, 0);
				// if (openFileName != nullptr) {
				// 	auto pBuffer =
				// 		zep.GetEditor().GetFileBuffer(
				// 			openFileName);
				// 	zep.GetEditor()
				// 		.GetActiveTabWindow()
				// 		->GetActiveWindow()
				// 		->SetBuffer(pBuffer);
				// }
			}
			ImGui::EndMenu();
		}

		const auto &buffer = zep->GetEditor()
					     .GetActiveTabWindow()
					     ->GetActiveWindow()
					     ->GetBuffer();

		if (ImGui::BeginMenu("Settings")) {
			if (ImGui::BeginMenu("Editor Mode")) {
				bool enabledVim =
					strcmp(buffer.GetMode()->Name(),
					       Zep::ZepMode_Vim::StaticName()) ==
					0;
				bool enabledNormal = !enabledVim;
				if (ImGui::MenuItem("Vim", "CTRL+2",
						    &enabledVim)) {
					zep->GetEditor().SetGlobalMode(
						Zep::ZepMode_Vim::StaticName());
				} else if (ImGui::MenuItem("Standard", "CTRL+1",
							   &enabledNormal)) {
					zep->GetEditor().SetGlobalMode(
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

				if (ImGui::MenuItem("Dark", "", &enabledDark)) {
					zep->GetEditor().GetTheme().SetThemeType(
						ThemeType::Dark);
				} else if (ImGui::MenuItem("Light", "",
							   &enabledLight)) {
					zep->GetEditor().GetTheme().SetThemeType(
						ThemeType::Light);
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window")) {
			auto pTabWindow = zep->GetEditor().GetActiveTabWindow();
			if (ImGui::MenuItem("Horizontal Split")) {
				pTabWindow->AddWindow(
					&pTabWindow->GetActiveWindow()
						 ->GetBuffer(),
					pTabWindow->GetActiveWindow(),
					RegionLayoutType::VBox);
			} else if (ImGui::MenuItem("Vertical Split")) {
				pTabWindow->AddWindow(
					&pTabWindow->GetActiveWindow()
						 ->GetBuffer(),
					pTabWindow->GetActiveWindow(),
					RegionLayoutType::HBox);
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
	auto menuSize =
		ImGui::GetStyle().FramePadding.y * 2 + ImGui::GetFontSize();
	ImGui::SetNextWindowPos(ImVec2(0, menuSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(float(w), float(h - menuSize)),
				 ImGuiCond_Always);

	ImGui::PushStyleColor(ImGuiCol_WindowBg,
			      ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::Begin("Zep", nullptr,
		     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			     ImGuiWindowFlags_NoScrollbar);

	auto min = ImGui::GetCursorScreenPos();
	auto max = ImGui::GetContentRegionAvail();
	max.x = std::max(1.0f, max.x);
	max.y = std::max(1.0f, max.y);

	// Fill the window
	max.x = min.x + max.x;
	max.y = min.y + max.y;
	zep->spEditor->SetDisplayRegion(Zep::NVec2f(min.x, min.y),
					Zep::NVec2f(max.x, max.y));

	// Display the editor inside this window
	zep->spEditor->Display();
	zep->spEditor->HandleInput();
	ImGui::End();
	ImGui::PopStyleVar(4);
	ImGui::PopStyleColor(1);
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
		showFPS = !showFPS;
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
