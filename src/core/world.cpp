#include "core/world.h"

#include "components/camera_input.h"
#include "components/transform.h"
#include "core/engine.h"

World::World() {
  previousTime = 0;
  elapsedTime = 0;
  deltaTime = 0;
  paused = false;
  shouldClose = false;

  window = Engine::GetWindow();
}

void World::Run() {
  if (!window)
    return;

  while (!window->ShouldClose()) {
    LoopUpdate();
  }
}

void World::Pause() { paused = !paused; }

void World::Exit() {
  shouldClose = true;
  window->Close();
}

double World::GetLastFrameTime() { return deltaTime; }

void World::ComputeFrameDeltaTime() {
  elapsedTime = Engine::GetElapsedTime();
  deltaTime = elapsedTime - previousTime;
  previousTime = elapsedTime;
}

void World::LoopUpdate() {
  // Polls and buffers the events
  window->PollEvents();

  // Computes frame deltaTime in seconds
  ComputeFrameDeltaTime();

  // Calls the methods of the instance of InputController in the following order
  // OnWindowResize, OnMouseMove, OnMouseBtnPress, OnMouseBtnRelease,
  // OnMouseScroll, OnKeyPress, OnMouseScroll, OnInputUpdate OnInputUpdate will
  // be called each frame, the other functions are called only if an event is
  // registered
  window->UpdateObservers();

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // Frame processing
  FrameStart();
  Update(static_cast<float>(deltaTime));
  FrameEnd();

  // ImGui rendering
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // Swap front and back buffers - image will be displayed to the screen
  window->SwapBuffers();
}

/*
** NOTE: All the code below this comment is needed
**       in order to setup the python binding.
*/

#include <pybind11/embed.h>

namespace py = pybind11;

class PyWorld : public World {
  using World::World;

public:
  virtual void Init() { PYBIND11_OVERRIDE(void, World, Init); }

  virtual void FrameStart() { PYBIND11_OVERRIDE(void, World, FrameStart); }

  virtual void Update(float deltaTimeSeconds) {
    PYBIND11_OVERRIDE(void, World, Update, deltaTimeSeconds);
  }

  virtual void FrameEnd() { PYBIND11_OVERRIDE(void, World, FrameEnd); }

  // virtual void OnInputUpdate(float deltaTime, int mods) {
  //   PYBIND11_OVERRIDE(void, InputController, OnInputUpdate, deltaTime, mods);
  // }

  // virtual void OnKeyPress(int key, int mods) {
  //   PYBIND11_OVERRIDE(void, InputController, OnKeyPress, key, mods);
  // }

  // virtual void OnKeyRelease(int key, int mods) {
  //   PYBIND11_OVERRIDE(void, InputController, OnKeyRelease, key, mods);
  // }

  // virtual void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) {
  //   PYBIND11_OVERRIDE(void, InputController, OnMouseMove, mouseX, mouseY,
  //                     deltaX, deltaY);
  // }

  // virtual void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
  // {
  //   PYBIND11_OVERRIDE(void, InputController, OnMouseBtnPress, mouseX, mouseY,
  //                     button, mods);
  // }

  // virtual void OnMouseBtnRelease(int mouseX, int mouseY, int button, int
  // mods) {
  //   PYBIND11_OVERRIDE(void, InputController, OnMouseBtnRelease, mouseX,
  //   mouseY,
  //                     button, mods);
  // }

  // virtual void OnMouseScroll(int mouseX, int mouseY, int offsetX, int
  // offsetY) {
  //   PYBIND11_OVERRIDE(void, InputController, OnMouseScroll, mouseX, mouseY,
  //                     offsetX, offsetY);
  // }

  // virtual void OnWindowResize(int width, int height) {
  //   PYBIND11_OVERRIDE(void, InputController, OnWindowResize, width, height);
  // }
};

PYBIND11_EMBEDDED_MODULE(world, module) {
  py::class_<World, PyWorld>(module, "World")
      .def(py::init<>())
      .def("Run", &World::Run)
      .def("Init", &World::Init)
      .def("FrameStart", &World::FrameStart)
      .def("Update", &World::Update)
      .def("FrameEnd", &World::FrameEnd)
      .def("Run", &World::Run)
      .def("Pause", &World::Pause)
      .def("Exit", &World::Exit)
      .def("GetLastFrameTime", &World::GetLastFrameTime);
  // .def("IsActive", &World::IsActive)
  // .def("SetActive", &World::SetActive)
  // .def("OnInputUpdate", &World::OnInputUpdate)
  // .def("OnKeyPress", &World::OnKeyPress)
  // .def("OnKeyRelease", &World::OnKeyRelease)
  // .def("OnMouseMove", &World::OnMouseMove)
  // .def("OnMouseBtnPress", &World::OnMouseBtnPress)
  // .def("OnMouseBtnRelease", &World::OnMouseBtnRelease)
  // .def("OnMouseScroll", &World::OnMouseScroll)
  // .def("OnWindowResize", &World::OnWindowResize);
}
