#include "bb3d/opengl_context.hpp"

#include <GL/glew.h>  // for glEnable, GL_TRUE, GL_DONT_CARE, glClear, glClea...

#include <algorithm>  // for max
#include <chrono>     // for duration, duration_cast, operator-, high_resolut...
#include <cstdio>     // for fprintf, stderr, sprintf
#include <cstdlib>    // for EXIT_FAILURE
#include <iostream>
#include <queue>   // for queue
#include <string>  // for string
#include <vector>  // for vector

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>  // for glfwWindowHint, glfwGetWindowUserPointer, glfwGe...

#include <glm/glm.hpp>                   // for operator+, vec3, mat4, radians, vec4, vec<>::(an...
#include <glm/gtc/matrix_transform.hpp>  // for lookAt, ortho, perspective

#include "bb3d/assert.hpp"             // for exit_thread_safe
#include "bb3d/camera.hpp"             // for Camera
#include "bb3d/gl_error.hpp"           // for GlDebugOutput
#include "bb3d/shader/colorlines.hpp"  // for ColoredVec3, ColorLines
#include "bb3d/shader/freetype.hpp"    // for Freetype
#include "tools/cpp/runfiles/runfiles.h"

namespace bb3d {
static GLFWwindow *OpenglSetup(WindowState *window_state);

std::string g_argv0;

std::string Window::GetBazelRlocation(const std::string &path) {
  using bazel::tools::cpp::runfiles::Runfiles;

  if (g_argv0.empty()) {
    std::cerr << "Aborting because Window has not been initialized. Argv[0] needed for runfiles."
              << std::endl;
    bb3d::exit_thread_safe(EXIT_FAILURE);
  }

  std::string error;
  std::unique_ptr<Runfiles> bazel_runfiles(Runfiles::Create(g_argv0));
  if (bazel_runfiles == nullptr) {
    std::cerr << error << std::endl;
    std::cerr << "Aborting because Runfiles not initialized." << std::endl;
    bb3d::exit_thread_safe(EXIT_FAILURE);
  }

  std::string runfile_path = bazel_runfiles->Rlocation("bb3d/" + path);
  if (runfile_path.empty()) {
    std::cerr << "Can't find bazel runfile for '" << path << "'" << std::endl;
    exit_thread_safe(EXIT_FAILURE);
  }

  return runfile_path;
}

Window::Window(char *argv0)
  : window_state_(std::make_unique<bb3d::WindowState>(bb3d::WindowState())),
    glfw_window(OpenglSetup(window_state_.get()), &glfwDestroyWindow) {
  g_argv0 = argv0;
};

Window::~Window() {
  glfwTerminate();
};

void Window::Close() {
  glfwSetWindowShouldClose(glfw_window.get(), GLFW_TRUE);
}

bool Window::ShouldClose() { return glfwWindowShouldClose(glfw_window.get()) != 0; }

void Window::SwapBuffers() { glfwSwapBuffers(glfw_window.get()); }

void Window::PollEvents() { glfwPollEvents(); }

Window::Size Window::GetSize() const {
  Window::Size window_size{};
  glfwGetWindowSize(glfw_window.get(), &window_size.width, &window_size.height);
  return window_size;
}

const Camera &WindowState::GetCamera() const { return camera; }

static void KeyCallback(GLFWwindow *glfw_window, int key, int scancode, int action,
                        int mods __attribute__((unused))) {
  WindowState &window_state =
      *reinterpret_cast<WindowState *>(glfwGetWindowUserPointer(glfw_window));
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(glfw_window, GLFW_TRUE);
  } else if (action == GLFW_PRESS) {
    window_state.keypress_queue.push(key);
    fprintf(stderr, "Key press! %s (%d)\n", glfwGetKeyName(key, scancode), key);
  }
}

static void WindowSizeCallback(GLFWwindow *window __attribute__((unused)), int width, int height) {
  glViewport(0, 0, width, height);
}

static void CursorPositionCallback(GLFWwindow *glfw_window, double xpos, double ypos) {
  WindowState &window_state =
      *reinterpret_cast<WindowState *>(glfwGetWindowUserPointer(glfw_window));
  if (window_state.mouse_handler.cursor_rotating) {
    window_state.camera.Rotate(
        static_cast<float>(xpos - window_state.mouse_handler.cursor_rotating_previous_xpos),
        static_cast<float>(ypos - window_state.mouse_handler.cursor_rotating_previous_ypos));
    window_state.mouse_handler.cursor_rotating_previous_xpos = xpos;
    window_state.mouse_handler.cursor_rotating_previous_ypos = ypos;
  }
  if (window_state.mouse_handler.cursor_xy_translating) {
    window_state.camera.TranslateXy(
        static_cast<float>(xpos - window_state.mouse_handler.cursor_xy_translating_previous_xpos),
        static_cast<float>(ypos - window_state.mouse_handler.cursor_xy_translating_previous_ypos));
    window_state.mouse_handler.cursor_xy_translating_previous_xpos = xpos;
    window_state.mouse_handler.cursor_xy_translating_previous_ypos = ypos;
  }
  if (window_state.mouse_handler.cursor_z_translating) {
    window_state.camera.TranslateZ(
        static_cast<float>(xpos - window_state.mouse_handler.cursor_z_translating_previous_xpos),
        static_cast<float>(ypos - window_state.mouse_handler.cursor_z_translating_previous_ypos));
    window_state.mouse_handler.cursor_z_translating_previous_xpos = xpos;
    window_state.mouse_handler.cursor_z_translating_previous_ypos = ypos;
  }
}

static void DescribeNewCameraFocus(const Camera &camera) {
  const glm::vec3 focus_position = camera.Center();
  fprintf(stderr, "Camera focus moved to {%.1f, %.1f, %.1f}\n", focus_position.x, focus_position.y,
          focus_position.z);
}

static void MouseButtonCallback(GLFWwindow *glfw_window, int button, int action,
                                int mods __attribute__((unused))) {
  WindowState &window_state =
      *reinterpret_cast<WindowState *>(glfwGetWindowUserPointer(glfw_window));

  // fprintf(stderr, "Mouse button pressed: %d %d\n", button, action);

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    // emable drag state
    window_state.mouse_handler.cursor_rotating = true;

    // set previous position
    double xpos{};
    double ypos{};
    glfwGetCursorPos(glfw_window, &xpos, &ypos);
    window_state.mouse_handler.cursor_rotating_previous_xpos = xpos;
    window_state.mouse_handler.cursor_rotating_previous_ypos = ypos;
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    // emable drag state
    window_state.mouse_handler.cursor_xy_translating = true;

    // set previous position
    double xpos{};
    double ypos{};
    glfwGetCursorPos(glfw_window, &xpos, &ypos);
    window_state.mouse_handler.cursor_xy_translating_previous_xpos = xpos;
    window_state.mouse_handler.cursor_xy_translating_previous_ypos = ypos;
  }

  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
    // emable drag state
    window_state.mouse_handler.cursor_z_translating = true;

    // set previous position
    double xpos{};
    double ypos{};
    glfwGetCursorPos(glfw_window, &xpos, &ypos);
    window_state.mouse_handler.cursor_z_translating_previous_xpos = xpos;
    window_state.mouse_handler.cursor_z_translating_previous_ypos = ypos;
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    // disable drag state
    window_state.mouse_handler.cursor_rotating = false;

    // initialize previous position for determinism
    window_state.mouse_handler.cursor_rotating_previous_xpos = 0;
    window_state.mouse_handler.cursor_rotating_previous_ypos = 0;
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    // disable drag state
    window_state.mouse_handler.cursor_xy_translating = false;

    // initialize previous position for determinism
    window_state.mouse_handler.cursor_xy_translating_previous_xpos = 0;
    window_state.mouse_handler.cursor_xy_translating_previous_ypos = 0;
    DescribeNewCameraFocus(window_state.camera);
  }

  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
    // disable drag state
    window_state.mouse_handler.cursor_z_translating = false;

    // initialize previous position for determinism
    window_state.mouse_handler.cursor_z_translating_previous_xpos = 0;
    window_state.mouse_handler.cursor_z_translating_previous_ypos = 0;
    DescribeNewCameraFocus(window_state.camera);
  }
}

static void ScrollCallback(GLFWwindow *glfw_window, double xoffset __attribute__((unused)),
                           double yoffset) {
  WindowState &window_state =
      *reinterpret_cast<WindowState *>(glfwGetWindowUserPointer(glfw_window));
  window_state.camera.Scroll(static_cast<float>(yoffset));
}

static void ErrorCallback(int error, const char *description) {
  fprintf(stderr, "Error (%d): %s\n", error, description);
}

static GLFWwindow *OpenglSetup(WindowState *window_state) {
  glfwSetErrorCallback(ErrorCallback);

  // Load GLFW and Create a Window
  if (glfwInit() == 0) {
    fprintf(stderr, "Failed to initialize glfw");
    exit_thread_safe(EXIT_FAILURE);
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  // Create window.
  GLFWwindow *const window = glfwCreateWindow(0.7 * 1920, 0.7 * 1080, "bb3d", nullptr, nullptr);

  if (window == nullptr) {
    fprintf(stderr, "Failed to Create OpenGL Context");
    glfwTerminate();
    exit_thread_safe(EXIT_FAILURE);
  }

  // Set user pointer.
  glfwSetWindowUserPointer(window, window_state);

  // Set callbacks.
  glfwSetKeyCallback(window, KeyCallback);
  glfwSetCursorPosCallback(window, CursorPositionCallback);
  glfwSetMouseButtonCallback(window, MouseButtonCallback);
  glfwSetScrollCallback(window, ScrollCallback);
  glfwSetWindowSizeCallback(window, WindowSizeCallback);

  // Create Context and Load OpenGL Functions
  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  glewInit();
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_PROGRAM_POINT_SIZE);

  // Debugging
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(GlDebugOutput, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

  fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
  glfwSwapInterval(1);

  return window;
}

glm::mat4 WindowState::GetViewTransformation() const {
  return glm::lookAt(camera.Eye(), camera.Center(), glm::vec3(0.0F, 0.0F, -1.0F));
}

glm::mat4 Window::GetProjectionTransformation() const {
  const float min_clip = 1e-3F;
  const float max_clip = 1e4F;
  Window::Size window_size = GetSize();
  window_size.width = std::max(window_size.width, 1);
  window_size.height = std::max(window_size.height, 1);
  const float aspect_ratio =
      static_cast<float>(window_size.width) / static_cast<float>(window_size.height);
  return glm::perspective(glm::radians(45.0F), aspect_ratio, min_clip, max_clip);
}

glm::mat4 Window::GetOrthographicProjection() const {
  // projection transformation
  Window::Size window_size = GetSize();
  glfwGetWindowSize(glfw_window.get(), &window_size.width, &window_size.height);
  window_size.width = std::max(window_size.width, 1);
  window_size.height = std::max(window_size.height, 1);
  return glm::ortho(0.0F, static_cast<float>(window_size.width), 0.0F,
                    static_cast<float>(window_size.height));
}

bool WindowState::IsDraggingOrRotating() const {
  return mouse_handler.cursor_rotating || mouse_handler.cursor_xy_translating ||
         mouse_handler.cursor_z_translating;
}

bool WindowState::KeypressQueueEmpty() const { return keypress_queue.empty(); }
int WindowState::PopKeypressQueue() {
  const int next = keypress_queue.front();
  keypress_queue.pop();
  return next;
}

static std::vector<std::vector<bb3d::ColoredVec3>> AxesLines(const bb3d::Camera &camera) {
  constexpr glm::vec4 red = {1, 0, 0, 1};
  constexpr glm::vec4 green = {0, 1, 0, 1};
  constexpr glm::vec4 blue = {0, 0, 1, 1};
  const glm::vec3 focus_pos = camera.Center();
  const double distance = camera.Distance();
  const double scale = distance * 0.1;

  const glm::vec3 x = {scale, 0, 0};
  const glm::vec3 y = {0, scale, 0};
  const glm::vec3 z = {0, 0, scale};

  std::vector<std::vector<bb3d::ColoredVec3>> segments;
  segments.push_back({{focus_pos, red}, {focus_pos + x, red}});
  segments.push_back({{focus_pos, green}, {focus_pos + y, green}});
  segments.push_back({{focus_pos, blue}, {focus_pos + z, blue}});

  return segments;
}

void Window::RenderText(Freetype &freetype, const std::string &text, float x, float y, const glm::vec3 &color) {
  freetype.RenderText(GetOrthographicProjection(), text, x, y, color);
}

void Window::Run(
    std::function<void(key_t key)> &handle_keypress, std::function<void()> &update_visualization,
    std::function<void(const glm::mat4 &view, const glm::mat4 &proj)> &draw_visualization) {
  bb3d::ColorLines axes;
  bb3d::Freetype textbox(18);

  std::chrono::time_point t_last = std::chrono::high_resolution_clock::now();

  while (!ShouldClose()) {
    // Send keypress events to visualization to update state.
    while (!window_state_->KeypressQueueEmpty()) {
      handle_keypress(window_state_->PopKeypressQueue());
    }

    update_visualization();

    std::chrono::time_point t_now = std::chrono::high_resolution_clock::now();
    float frame_time =
        std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_last).count();
    t_last = t_now;

    // Clear the screen to black
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera transformation
    glm::mat4 view = window_state_->GetViewTransformation();

    // projection transformation
    glm::mat4 proj = GetProjectionTransformation();

    draw_visualization(view, proj);

    // draw axes if we're dragging or rotating
    if (window_state_->IsDraggingOrRotating()) {
      axes.Update(bb3d::AxesLines(window_state_->GetCamera()));
      axes.Draw(view, proj, GL_LINE_STRIP);
    }

    // Draw some dummy text.
    std::string fps_string(80, '\0');
    sprintf(fps_string.data(), "%.1f fps", 1 / frame_time);
    const bb3d::Window::Size window_size = GetSize();

    textbox.RenderText(GetOrthographicProjection(), fps_string, 25.0F,
                       static_cast<float>(window_size.height) - 25.0F, glm::vec3(1, 1, 1));

    // Swap buffers and poll events
    SwapBuffers();
    bb3d::Window::PollEvents();
  }
}

};  //  namespace bb3d
