#pragma once

#include <GL/glew.h>

#include <vector>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "bb3d/shader/shader.hpp"

namespace bb3d {

class Lines {
 public:
  Lines();
  ~Lines() = default;
  void Update(const std::vector<std::vector<glm::vec3> > &segments);
  void Draw(const glm::mat4 &view, const glm::mat4 &proj, const glm::vec4 &color, GLenum mode);
  void SetPointSize(float point_size) { point_size_ = point_size; };

 private:
  float point_size_ = 1;

  Shader shader_;
  GLuint vao_{};
  GLuint vbo_{};
  std::vector<GLint> segment_sizes_;
  GLint current_buffer_size_;
};

};  // namespace bb3d
