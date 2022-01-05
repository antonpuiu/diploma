#include "core/engine.h"

#include <iostream>

#include "core/managers/texture_manager.h"
#include "utils/gl_utils.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <pybind11/embed.h>

WindowObject *Engine::window = nullptr;

WindowObject *Engine::Init(const WindowProperties &props) {
  pybind11::initialize_interpreter();

  /* Initialize the library */
  if (!glfwInit())
    exit(0);

  window = new WindowObject(props);

  glewExperimental = true;
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    // Serious problem
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    exit(0);
  }

  TextureManager::Init(window->props.selfDir);

  return window;
}

WindowObject *Engine::GetWindow() { return window; }

void Engine::Exit() {
  std::cout << "====================================================="
            << std::endl;
  std::cout << "Engine closed. Exit" << std::endl;

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwTerminate();
}

double Engine::GetElapsedTime() { return glfwGetTime(); }
