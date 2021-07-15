#pragma once

#include <GL/glew.h>

#include <vector>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "bb3d/shader/shader.hpp"

namespace bb3d {

struct ColoredVec3 {
  glm::vec3 position;
  glm::vec4 color;
};

struct ColorLines {
 public:
  ColorLines();
  ~ColorLines() = default;
  void Update(const std::vector<std::vector<ColoredVec3> > &segments);
  void Draw(const glm::mat4 &view, const glm::mat4 &proj, GLenum mode);
  void SetPointSize(float point_size) { point_size_ = point_size; };

 private:
  float point_size_ = 1;

  Shader shader_;
  GLuint vao_{};
  GLuint vbo_{};
  GLint current_buffer_size_;
  std::vector<GLint> segment_sizes_;
};

};  // namespace bb3d
