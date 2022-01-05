#include "core/window/input_controller.h"

#include "core/engine.h"

InputController::InputController() {
  window = Engine::GetWindow();
  window->SubscribeToEvents(this);
  isAttached = true;
}

InputController::~InputController() {}

bool InputController::IsActive() const { return isAttached; }

void InputController::SetActive(bool value) {
  isAttached = value;
  value ? window->SubscribeToEvents(this) : window->UnsubscribeFromEvents(this);
}

/*
** NOTE: All the code below this comment is needed in order
**       to setup the python binding.
*/

#include <pybind11/embed.h>

namespace py = pybind11;

class PyInputController : public InputController {
protected:
  using InputController::InputController;

  virtual void OnInputUpdate(float deltaTime, int mods) {
    PYBIND11_OVERRIDE(void, InputController, OnInputUpdate, deltaTime, mods);
  }

  virtual void OnKeyPress(int key, int mods) {
    PYBIND11_OVERRIDE(void, InputController, OnKeyPress, key, mods);
  }

  virtual void OnKeyRelease(int key, int mods) {
    PYBIND11_OVERRIDE(void, InputController, OnKeyRelease, key, mods);
  }

  virtual void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) {
    PYBIND11_OVERRIDE(void, InputController, OnMouseMove, mouseX, mouseY,
                      deltaX, deltaY);
  }

  virtual void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) {
    PYBIND11_OVERRIDE(void, InputController, OnMouseBtnPress, mouseX, mouseY,
                      button, mods);
  }

  virtual void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) {
    PYBIND11_OVERRIDE(void, InputController, OnMouseBtnRelease, mouseX, mouseY,
                      button, mods);
  }

  virtual void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) {
    PYBIND11_OVERRIDE(void, InputController, OnMouseScroll, mouseX, mouseY,
                      offsetX, offsetY);
  }

  virtual void OnWindowResize(int width, int height) {
    PYBIND11_OVERRIDE(void, InputController, OnWindowResize, width, height);
  }
};

class PublicistInputController : public InputController {
public:
  using InputController::InputController;

  /* NOTE: protected methods. */
  using InputController::OnInputUpdate;
  using InputController::OnKeyPress;
  using InputController::OnKeyRelease;
  using InputController::OnMouseBtnPress;
  using InputController::OnMouseBtnRelease;
  using InputController::OnMouseMove;
  using InputController::OnMouseScroll;
  using InputController::OnWindowResize;

  /* NOTE: protected fields. */
  using InputController::window;
};

PYBIND11_EMBEDDED_MODULE(input_controller, module) {
  py::class_<InputController, PyInputController>(module, "InputController")
      .def(py::init<>())

      /* NOTE: protected methods. */
      .def("OnInputUpdate", &PublicistInputController::OnInputUpdate)
      .def("OnKeyPress", &PublicistInputController::OnKeyPress)
      .def("OnKeyRelease", &PublicistInputController::OnKeyRelease)
      .def("OnMouseMove", &PublicistInputController::OnMouseMove)
      .def("OnMouseBtnPress", &PublicistInputController::OnMouseBtnPress)
      .def("OnMouseBtnRelease", &PublicistInputController::OnMouseBtnRelease)
      .def("OnMouseScroll", &PublicistInputController::OnMouseScroll)
      .def("OnWindowResize", &PublicistInputController::OnWindowResize)

      /* NOTE: protected fields. */
      .def_readwrite("window", &PublicistInputController::window)

      /* NOTE: private fields with getter and setter. */
      .def_property("isAttached", &InputController::IsActive,
                    &InputController::SetActive);
}
