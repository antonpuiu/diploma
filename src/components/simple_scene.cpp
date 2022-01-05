#include "components/simple_scene.h"

#include <iostream>
#include <vector>

#include "components/camera_input.h"
#include "components/scene_input.h"
#include "components/transform.h"

using namespace gfxc;

SimpleScene::SimpleScene() { InitResources(); }

SimpleScene::~SimpleScene() {}

void SimpleScene::glLineWidth(GLfloat width) {
#ifdef __APPLE__
  ::glLineWidth(1);
#else
  ::glLineWidth(width);
#endif
}

void SimpleScene::InitResources() {
  // Sets common GL states
  glClearColor(0, 0, 0, 1);

  drawGroundPlane = true;

  objectModel = new Transform();

  camera = new Camera();
  camera->SetPerspective(60, window->props.aspectRatio, 0.01f, 200);
  camera->m_transform->SetMoveSpeed(2);
  camera->m_transform->SetWorldPosition(glm::vec3(0, 1.6f, 2.5));
  camera->m_transform->SetWorldRotation(glm::vec3(-15, 0, 0));
  camera->Update();

  cameraInput = new CameraInput(camera);
  window = Engine::GetWindow();

  SceneInput *SI = new SceneInput(this);
  (void)SI;

  xozPlane = new Mesh("plane");
  xozPlane->LoadMesh(
      PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"),
      "plane50.obj");

  {
    std::vector<VertexFormat> vertices = {
        VertexFormat(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0, 1, 0), glm::vec3(0, 1, 0)),
    };
    std::vector<unsigned int> indices = {0, 1};

    simpleLine = new Mesh("line");
    simpleLine->InitFromData(vertices, indices);
    simpleLine->SetDrawMode(GL_LINES);
  }

  // Create a shader program for drawing face polygon with the color of the
  // normal
  {
    Shader *shader = new Shader("Simple");
    shader->AddShader(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::SHADERS,
                                "MVP.Texture.VS.glsl"),
                      GL_VERTEX_SHADER);
    shader->AddShader(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::SHADERS,
                                "Default.FS.glsl"),
                      GL_FRAGMENT_SHADER);
    shader->CreateAndLink();
    shaders[shader->GetName()] = shader;
  }

  // Create a shader program for drawing vertex colors
  {
    Shader *shader = new Shader("Color");
    shader->AddShader(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::SHADERS,
                                "MVP.Texture.VS.glsl"),
                      GL_VERTEX_SHADER);
    shader->AddShader(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::SHADERS,
                                "Color.FS.glsl"),
                      GL_FRAGMENT_SHADER);
    shader->CreateAndLink();
    shaders[shader->GetName()] = shader;
  }

  // Create a shader program for drawing face polygon with the color of the
  // normal
  {
    Shader *shader = new Shader("VertexNormal");
    shader->AddShader(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::SHADERS,
                                "MVP.Texture.VS.glsl"),
                      GL_VERTEX_SHADER);
    shader->AddShader(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::SHADERS,
                                "Normals.FS.glsl"),
                      GL_FRAGMENT_SHADER);
    shader->CreateAndLink();
    shaders[shader->GetName()] = shader;
  }

  // Create a shader program for drawing vertex colors
  {
    Shader *shader = new Shader("VertexColor");
    shader->AddShader(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::SHADERS,
                                "MVP.Texture.VS.glsl"),
                      GL_VERTEX_SHADER);
    shader->AddShader(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::SHADERS,
                                "VertexColor.FS.glsl"),
                      GL_FRAGMENT_SHADER);
    shader->CreateAndLink();
    shaders[shader->GetName()] = shader;
  }

  // Default rendering mode will use depth buffer
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
}

void SimpleScene::AddMeshToList(Mesh *mesh) {
  if (mesh->GetMeshID()) {
    meshes[mesh->GetMeshID()] = mesh;
  }
}

void SimpleScene::DrawCoordinateSystem() {
  DrawCoordinateSystem(camera->GetViewMatrix(), camera->GetProjectionMatrix());
}

void SimpleScene::DrawCoordinateSystem(const glm::mat4 &viewMatrix,
                                       const glm::mat4 &projectionMaxtix) {
  glLineWidth(1);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // Render the coordinate system
  {
    Shader *shader = shaders["Color"];
    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE,
                       glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE,
                       glm::value_ptr(projectionMaxtix));

    if (drawGroundPlane) {
      objectModel->SetScale(glm::vec3(1));
      objectModel->SetWorldPosition(glm::vec3(0));
      glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                         glm::value_ptr(objectModel->GetModel()));
      glUniform3f(shader->GetUniformLocation("color"), 0.5f, 0.5f, 0.5f);
      xozPlane->Render();
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glLineWidth(3);
    objectModel->SetScale(glm::vec3(1, 25, 1));
    objectModel->SetWorldRotation(glm::quat());
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                       glm::value_ptr(objectModel->GetModel()));
    glUniform3f(shader->GetUniformLocation("color"), 0, 1, 0);
    simpleLine->Render();

    objectModel->SetWorldRotation(glm::vec3(0, 0, -90));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                       glm::value_ptr(objectModel->GetModel()));
    glUniform3f(shader->GetUniformLocation("color"), 1, 0, 0);
    simpleLine->Render();

    objectModel->SetWorldRotation(glm::vec3(90, 0, 0));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                       glm::value_ptr(objectModel->GetModel()));
    glUniform3f(shader->GetUniformLocation("color"), 0, 0, 1);
    simpleLine->Render();

    objectModel->SetWorldRotation(glm::quat());

    glLineWidth(1);
  }
}

void SimpleScene::RenderMesh(Mesh *mesh, Shader *shader, glm::vec3 position,
                             glm::vec3 scale) {
  if (!mesh || !shader || !shader->program)
    return;

  // Render an object using the specified shader and the specified position
  shader->Use();
  glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE,
                     glm::value_ptr(camera->GetViewMatrix()));
  glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE,
                     glm::value_ptr(camera->GetProjectionMatrix()));

  glm::mat4 model(1);
  model = glm::translate(model, position);
  model = glm::scale(model, scale);
  glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                     glm::value_ptr(model));
  mesh->Render();
}

void SimpleScene::RenderMesh(Mesh *mesh, glm::vec3 position, glm::vec3 scale) {
  RenderMesh(mesh, shaders["Simple"], position, scale);
}

void SimpleScene::RenderMesh2D(Mesh *mesh, Shader *shader,
                               const glm::mat3 &modelMatrix) {
  if (!mesh || !shader || !shader->program)
    return;

  shader->Use();
  glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE,
                     glm::value_ptr(camera->GetViewMatrix()));
  glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE,
                     glm::value_ptr(camera->GetProjectionMatrix()));

  glm::mat3 mm = modelMatrix;
  glm::mat4 model =
      glm::mat4(mm[0][0], mm[0][1], mm[0][2], 0.f, mm[1][0], mm[1][1], mm[1][2],
                0.f, 0.f, 0.f, mm[2][2], 0.f, mm[2][0], mm[2][1], 0.f, 1.f);

  glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                     glm::value_ptr(model));
  mesh->Render();
}

void SimpleScene::RenderMesh2D(Mesh *mesh, const glm::mat3 &modelMatrix,
                               const glm::vec3 &color) const {
  Shader *shader = shaders.at("Color");

  if (!mesh || !shader || !shader->program)
    return;

  glm::mat3 mm = modelMatrix;
  glm::mat4 model =
      glm::mat4(mm[0][0], mm[0][1], mm[0][2], 0.f, mm[1][0], mm[1][1], mm[1][2],
                0.f, 0.f, 0.f, mm[2][2], 0.f, mm[2][0], mm[2][1], 0.f, 1.f);

  // Render an object using the specified shader and the specified position
  shader->Use();
  glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE,
                     glm::value_ptr(camera->GetViewMatrix()));
  glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE,
                     glm::value_ptr(camera->GetProjectionMatrix()));
  glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                     glm::value_ptr(model));
  glUniform3f(shader->GetUniformLocation("color"), color.r, color.g, color.b);

  mesh->Render();
}

void SimpleScene::RenderMesh(Mesh *mesh, Shader *shader,
                             const glm::mat4 &modelMatrix) {
  if (!mesh || !shader || !shader->program)
    return;

  // Render an object using the specified shader and the specified position
  shader->Use();
  glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE,
                     glm::value_ptr(camera->GetViewMatrix()));
  glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE,
                     glm::value_ptr(camera->GetProjectionMatrix()));
  glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                     glm::value_ptr(modelMatrix));

  mesh->Render();
}

void SimpleScene::ReloadShaders() const {
  std::cout << std::endl;
  std::cout << "=============================" << std::endl;
  std::cout << "Reloading Shaders" << std::endl;
  std::cout << "=============================" << std::endl;
  std::cout << std::endl;

  for (auto &shader : shaders) {
    shader.second->Reload();
  }
}

Camera *SimpleScene::GetSceneCamera() const { return camera; }

InputController *SimpleScene::GetCameraInput() const { return cameraInput; }

void SimpleScene::ClearScreen(const glm::vec3 &color) {
  glm::ivec2 resolution = window->props.resolution;

  // Sets the clear color for the color buffer
  glClearColor(color.x, color.y, color.z, 1);

  // Clears the color buffer (using the previously set color) and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Sets the screen area where to draw
  glViewport(0, 0, resolution.x, resolution.y);
}

bool SimpleScene::ToggleGroundPlane() {
  drawGroundPlane = !drawGroundPlane;
  return drawGroundPlane;
}

void SimpleScene::Update(float deltaTimeSeconds) {
  ClearScreen();
  DrawCoordinateSystem();
}

/*
** NOTE: All the code below this comment is needed
**       in order to setup the python binding.
*/

#include <pybind11/embed.h>

namespace py = pybind11;

class PySimpleScene : public SimpleScene {
  using SimpleScene::SimpleScene;

  /* NOTE: SimpleScene virtual functions. */
protected:
  virtual void AddMeshToList(Mesh *mesh) {
    PYBIND11_OVERRIDE(void, SimpleScene, AddMeshToList, mesh);
  }

  virtual void DrawCoordinateSystem() {
    PYBIND11_OVERRIDE(void, SimpleScene, DrawCoordinateSystem);
  }

  virtual void DrawCoordinateSystem(const glm::mat4 &viewMatrix,
                                    const glm::mat4 &projectionMaxtix) {
    PYBIND11_OVERRIDE(void, SimpleScene, DrawCoordinateSystem, viewMatrix,
                      projectionMaxtix);
  }

  virtual void RenderMesh(Mesh *mesh, Shader *shader, glm::vec3 position,
                          glm::vec3 scale = glm::vec3(1)) {
    PYBIND11_OVERRIDE(void, SimpleScene, RenderMesh, mesh, shader, position,
                      scale);
  }

  virtual void RenderMesh(Mesh *mesh, glm::vec3 position,
                          glm::vec3 scale = glm::vec3(1)) {
    PYBIND11_OVERRIDE(void, SimpleScene, RenderMesh, mesh, position, scale);
  }

  virtual void RenderMesh2D(Mesh *mesh, Shader *shader,
                            const glm::mat3 &modelMatrix) {
    PYBIND11_OVERRIDE(void, SimpleScene, RenderMesh2D, mesh, shader,
                      modelMatrix);
  }

  virtual void RenderMesh2D(Mesh *mesh, const glm::mat3 &modelMatrix,
                            const glm::vec3 &color) const {
    PYBIND11_OVERRIDE(void, SimpleScene, RenderMesh2D, mesh, modelMatrix,
                      color);
  }

  virtual void RenderMesh(Mesh *mesh, Shader *shader,
                          const glm::mat4 &modelMatrix) {
    PYBIND11_OVERRIDE(void, SimpleScene, RenderMesh, mesh, shader, modelMatrix);
  }

  /* NOTE: World virtual functions. */
public:
  virtual void Init() { PYBIND11_OVERRIDE(void, SimpleScene, Init); }

  virtual void FrameStart() {
    PYBIND11_OVERRIDE(void, SimpleScene, FrameStart);
  }

  virtual void Update(float deltaTimeSeconds) {
    PYBIND11_OVERRIDE(void, SimpleScene, World::Update, deltaTimeSeconds);
  }

  virtual void FrameEnd() { PYBIND11_OVERRIDE(void, SimpleScene, FrameEnd); }

  /* NOTE: InputController virtual functions. */
protected:
  virtual void OnInputUpdate(float deltaTime, int mods) {
    PYBIND11_OVERRIDE(void, SimpleScene, OnInputUpdate, deltaTime, mods);
  }

  virtual void OnKeyPress(int key, int mods) {
    PYBIND11_OVERRIDE(void, SimpleScene, OnKeyPress, key, mods);
  }

  virtual void OnKeyRelease(int key, int mods) {
    PYBIND11_OVERRIDE(void, SimpleScene, OnKeyRelease, key, mods);
  }

  virtual void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) {
    PYBIND11_OVERRIDE(void, SimpleScene, OnMouseMove, mouseX, mouseY, deltaX,
                      deltaY);
  }

  virtual void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) {
    PYBIND11_OVERRIDE(void, SimpleScene, OnMouseBtnPress, mouseX, mouseY,
                      button, mods);
  }

  virtual void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) {
    PYBIND11_OVERRIDE(void, SimpleScene, OnMouseBtnRelease, mouseX, mouseY,
                      button, mods);
  }

  virtual void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) {
    PYBIND11_OVERRIDE(void, SimpleScene, OnMouseScroll, mouseX, mouseY, offsetX,
                      offsetY);
  }

  virtual void OnWindowResize(int width, int height) {
    PYBIND11_OVERRIDE(void, SimpleScene, OnWindowResize, width, height);
  }
};

class PublicistSimpleScene : public SimpleScene {
public:
  /* NOTE: SimpleScene methods. */
  using SimpleScene::AddMeshToList;
  using SimpleScene::ClearScreen;
  using SimpleScene::DrawCoordinateSystem;
  using SimpleScene::GetCameraInput;
  using SimpleScene::GetSceneCamera;
  using SimpleScene::glLineWidth;
  using SimpleScene::RenderMesh;
  using SimpleScene::RenderMesh2D;

  /* NOTE: SimpleScene fields. */
  using SimpleScene::meshes;
  using SimpleScene::shaders;

  /* NOTE: InputController methods. */
  using SimpleScene::OnInputUpdate;
  using SimpleScene::OnKeyPress;
  using SimpleScene::OnKeyRelease;
  using SimpleScene::OnMouseBtnPress;
  using SimpleScene::OnMouseBtnRelease;
  using SimpleScene::OnMouseMove;
  using SimpleScene::OnMouseScroll;
  using SimpleScene::OnWindowResize;

  /* NOTE: InputController fields. */
  using SimpleScene::window;

  /* NOTE: World methods. */
  using World::Update;

  /* NOTE: World fields. */
};

PYBIND11_EMBEDDED_MODULE(simple_scene, module) {
  py::class_<SimpleScene, PySimpleScene>(module, "SimpleScene")
      .def(py::init<>())

      /* NOTE: SimpleScene public methods. */
      .def("ToggleGroundPlane", &SimpleScene::ToggleGroundPlane)
      .def("ReloadShaders", &SimpleScene::ReloadShaders)

      /* NOTE: SimpleScene protected methods. */
      .def("DrawCoordinateSystem",
           static_cast<void (SimpleScene::*)()>(
               &PublicistSimpleScene::DrawCoordinateSystem))
      .def("DrawCoordinateSystem",
           static_cast<void (SimpleScene::*)(const glm::mat4 &,
                                             const glm::mat4 &)>(
               &PublicistSimpleScene::DrawCoordinateSystem))

      .def("RenderMesh", static_cast<void (SimpleScene::*)(
                             Mesh *, Shader *, glm::vec3, glm::vec3)>(
                             &PublicistSimpleScene::RenderMesh))
      .def("RenderMesh",
           static_cast<void (SimpleScene::*)(Mesh *, glm::vec3, glm::vec3)>(
               &PublicistSimpleScene::RenderMesh))
      .def("RenderMesh", static_cast<void (SimpleScene::*)(Mesh *, Shader *,
                                                           const glm::mat4 &)>(
                             &PublicistSimpleScene::RenderMesh))

      .def("RenderMesh2D", static_cast<void (SimpleScene::*)(
                               Mesh *, Shader *, const glm::mat3 &)>(
                               &PublicistSimpleScene::RenderMesh2D))
      /* BUG: not recognizing the function overload. */
      // .def("RenderMesh2D", static_cast<void (SimpleScene::*)(
      //                          Mesh *, const glm::mat3 &, const glm::vec3
      //                          &)>( &PublicistSimpleScene::RenderMesh2D))

      .def("GetSceneCamera", &PublicistSimpleScene::GetSceneCamera)
      .def("GetCameraInput", &PublicistSimpleScene::GetCameraInput)
      .def("ClearScreen", &PublicistSimpleScene::ClearScreen)
      .def("glLineWidth", &PublicistSimpleScene::glLineWidth)

      /* NOTE: SimpleScene protected fields. */
      .def_readwrite("meshes", &PublicistSimpleScene::meshes)
      .def_readwrite("shaders", &PublicistSimpleScene::shaders)

      /* NOTE: World public methods. */
      .def("Run", &SimpleScene::Run)
      .def("Init", &SimpleScene::Init)
      .def("FrameStart", &SimpleScene::FrameStart)
      .def("Update", &PublicistSimpleScene::Update)
      .def("FrameEnd", &SimpleScene::FrameEnd)
      .def("Run", &SimpleScene::Run)
      .def("Pause", &SimpleScene::Pause)
      .def("Exit", &SimpleScene::Exit)
      .def("GetLastFrameTime", &SimpleScene::GetLastFrameTime)

      /* NOTE: InputController protected methods. */
      .def("IsActive", &SimpleScene::IsActive)
      .def("SetActive", &SimpleScene::SetActive)
      .def("OnInputUpdate", &PublicistSimpleScene::OnInputUpdate)
      .def("OnKeyPress", &PublicistSimpleScene::OnKeyPress)
      .def("OnKeyRelease", &PublicistSimpleScene::OnKeyRelease)
      .def("OnMouseMove", &PublicistSimpleScene::OnMouseMove)
      .def("OnMouseBtnPress", &PublicistSimpleScene::OnMouseBtnPress)
      .def("OnMouseBtnRelease", &PublicistSimpleScene::OnMouseBtnRelease)
      .def("OnMouseScroll", &PublicistSimpleScene::OnMouseScroll)
      .def("OnWindowResize", &PublicistSimpleScene::OnWindowResize)

      /* NOTE: InputController protected fields. */
      .def_readwrite("window", &PublicistSimpleScene::window)

      /* NOTE: InputController private fields with getter and setter. */
      .def_property("isAttached", &SimpleScene::IsActive,
                    &SimpleScene::SetActive);
}
