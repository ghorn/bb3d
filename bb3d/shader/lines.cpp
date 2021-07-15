#include "lines.hpp"

#include <GL/glew.h>  // for GLint, GL_ARRAY_BUFFER, glEnable, glBindBuffer, glBindVertexArray

namespace bb3d {

Lines::Lines() : shader_("bb3d/shader/lines.vs", "bb3d/shader/lines.fs") {
  current_buffer_size_ = 0;

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);

  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure
  // vertex attributes(s).
  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, current_buffer_size_, nullptr, GL_DYNAMIC_DRAW);

  GLint posAttrib = 0;  // layout = 0 above
  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glEnableVertexAttribArray(posAttrib);

  // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex
  // attribute's bound vertex buffer object so afterwards we can safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but
  // this rarely happens. Modifying other VAOs requires a call to glBindVertexArray anyways so we
  // generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
  glBindVertexArray(0);
}

void Lines::Draw(const glm::mat4 &view, const glm::mat4 &proj, const glm::vec4 &color,
                 const GLenum mode) {
  // draw triangle
  shader_.UseProgram();
  glBindVertexArray(vao_);  // seeing as we only have a single VAO there's no need to bind it every
                            // time, but we'll do so to keep things a bit more organized

  // Set up transformations
  shader_.UniformMatrix4fv("view", view);
  shader_.UniformMatrix4fv("proj", proj);

  shader_.Uniform4f("color", color.r, color.g, color.b, color.a);
  shader_.Uniform1f("point_size", point_size_);

  // blend and antialias
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  // Draw lines one segment at a time.
  GLint offset = 0;
  for (const GLint segment_size : segment_sizes_) {
    glDrawArrays(mode, offset, segment_size);
    offset += segment_size;
  }
}

void Lines::Update(const std::vector<std::vector<glm::vec3> > &segments) {
  // Massage the data.
  // TODO(greg): static assert that std::vector<glm::vec3> is packed and just reinterpret cast
  std::vector<float> buffer_data;
  segment_sizes_.resize(0);
  for (const std::vector<glm::vec3> &segment : segments) {
    const auto segment_size = static_cast<GLint>(segment.size());
    segment_sizes_.push_back(segment_size);
    for (const glm::vec3 &vertex : segment) {
      buffer_data.push_back(vertex.x);
      buffer_data.push_back(vertex.y);
      buffer_data.push_back(vertex.z);
    }
  }

  // bind the buffer
  // glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);

  const auto buffer_size = static_cast<GLint>(sizeof(float) * buffer_data.size());
  if (buffer_size == current_buffer_size_) {
    // if the size of data is the same, just update the buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size, buffer_data.data());
  } else {
    // if the size of data has changed, we have to reallocate GPU memory
    glBufferData(GL_ARRAY_BUFFER, buffer_size, buffer_data.data(), GL_DYNAMIC_DRAW);
    current_buffer_size_ = buffer_size;
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  // glBindVertexArray(0);
}

};  // namespace bb3d
