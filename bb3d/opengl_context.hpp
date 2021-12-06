#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>  // for GLFWwindow
#include <sys/types.h>   // for key_t

#include <functional>   // for function
#include <glm/glm.hpp>  // for mat4
#include <memory>       // for unique_ptr
#include <queue>        // for queue

#include "bb3d/camera.hpp"  // for Camera
#include "bb3d/shader/freetype.hpp"

namespace bb3d {

struct MouseHandler {
  bool cursor_rotating;
  double cursor_rotating_previous_xpos;
  double cursor_rotating_previous_ypos;
  bool cursor_xy_translating;
  double cursor_xy_translating_previous_xpos;
  double cursor_xy_translating_previous_ypos;
  bool cursor_z_translating;
  double cursor_z_translating_previous_xpos;
  double cursor_z_translating_previous_ypos;
};

// The state which is stored by glfwSetWindowUserPointer.
class WindowState {
 public:
  WindowState() : keypress_queue{}, camera{} {};
  ~WindowState() = default;
  [[nodiscard]] bool KeypressQueueEmpty() const;
  int PopKeypressQueue();
  std::queue<int> keypress_queue;                 // to hand to user
  [[nodiscard]] const Camera &GetCamera() const;  // TODO(greg): delete me
  Camera camera;
  [[nodiscard]] bool IsDraggingOrRotating() const;
  [[nodiscard]] glm::mat4 GetViewTransformation() const;
  MouseHandler mouse_handler{};
};

class Window {
 public:
  explicit Window(char *argv0);
  ~Window();
  struct Size {
    int width;
    int height;
  };
  void Close();

  void RenderText(Freetype &freetype, const std::string &text, float x, float y, const glm::vec3 &color);

  // Find a bazel runfile. Requires Window to be initialized with argv[0] first.
  static std::string GetBazelRlocation(const std::string &path);
  [[nodiscard]] Size GetSize() const;
  [[nodiscard]] glm::mat4 GetProjectionTransformation() const;
  [[nodiscard]] glm::mat4 GetOrthographicProjection() const;
  void SwapBuffers();
  static void PollEvents();
  std::unique_ptr<WindowState> &GetWindowState() { return window_state_; };
  void Run(std::function<void(key_t key)> &handle_keypress,
           std::function<void()> &update_visualization,
           std::function<void(const glm::mat4 &view, const glm::mat4 &proj)> &draw_visualization);
  void SetCameraFocus(glm::vec3 new_focus){window_state_->camera.SetFocus(new_focus);};
  void SetCameraAzimuthDeg(float azimuth_deg){window_state_->camera.SetAzimuthDeg(azimuth_deg);};
  void SetCameraElevationDeg(float elevation_deg){window_state_->camera.SetElevationDeg(elevation_deg);};
  void SetCameraDistance(float distance){window_state_->camera.SetDistance(distance);};

 private:
  bool ShouldClose();
  std::unique_ptr<WindowState> window_state_;
  using unique_window_t = std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)>;
  unique_window_t glfw_window;
};

};  // namespace bb3d
