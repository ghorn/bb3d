#include "cubemesh.hpp"

#include <GL/glew.h>  // for GLuint, GL_ARRAY_BUFFER, glBindBuffer, glDisable, GL_ELEME...

#include <ext/alloc_traits.h>  // for __alloc_traits<>::value_type
#include <vector>              // for vector, allocator

#include "bb3d/assert.hpp"  // for ASSERT

namespace bb3d {

Cubemesh::Cubemesh() : shader_("bb3d/shader/cubemesh.vs", "bb3d/shader/cubemesh.fs") {
  num_indices_ = 0;
  vertex_buffer_size_ = 0;
  index_buffer_size_ = 0;

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
  glGenBuffers(1, &ebo_);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure
  // vertex attributes(s).
  glBindVertexArray(vao_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size_, nullptr, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size_, nullptr, GL_DYNAMIC_DRAW);

  shader_.VertexAttribPointer("position", 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              (void *)(0 * sizeof(GLfloat)));  // NOLINT
  glEnableVertexAttribArray(0);

  shader_.VertexAttribPointer("color", 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              (void *)(3 * sizeof(GLfloat)));  // NOLINT
  glEnableVertexAttribArray(1);

  // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex
  // attribute's bound vertex buffer object so afterwards we can safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);  // NOLINT

  // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS
  // stored in the VAO; keep the EBO bound.
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but
  // this rarely happens. Modifying other VAOs requires a call to glBindVertexArray anyways so we
  // generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
  glBindVertexArray(0);
}

void Cubemesh::Draw(const glm::mat4 &view, const glm::mat4 &proj) {
  // render
  shader_.UseProgram();
  glBindVertexArray(vao_);

  // Set up transformations
  shader_.UniformMatrix4fv("view", view);
  shader_.UniformMatrix4fv("proj", proj);

  // disable blending and polygon antialiasing
  glDisable(GL_BLEND);
  glDisable(GL_POLYGON_SMOOTH);

  // Draw triangles
  glDrawElements(GL_TRIANGLES, num_indices_, GL_UNSIGNED_INT, nullptr);

  // unbind vao
  glBindVertexArray(0);
}

void Cubemesh::Update(
    const Eigen::Matrix<std::pair<float, glm::vec3>, Eigen::Dynamic, Eigen::Dynamic> &grid,
    const float min_x, const float max_x, const float min_y, const float max_y) {
  const int nx = static_cast<int>(grid.rows());  // readability below
  const int ny = static_cast<int>(grid.cols());  // readability below

  ASSERT(nx >= 2);
  ASSERT(ny >= 2);

  const float dx = 0.5F * (max_x - min_x) / (static_cast<float>(nx) - 1);
  const float dy = 0.5F * (max_y - min_y) / (static_cast<float>(ny) - 1);

  // Massage the data.
  std::vector<float> vertices;
  for (int kx = 0; kx < nx; kx++) {
    const float x = min_x + (max_x - min_x) * static_cast<float>(kx) / static_cast<float>(nx - 1);
    for (int ky = 0; ky < ny; ky++) {
      const float y = min_y + (max_y - min_y) * static_cast<float>(ky) / static_cast<float>(ny - 1);
      const std::pair<float, glm::vec3> zcol = grid(kx, ky);
      const float z = zcol.first;
      const glm::vec3 color = zcol.second;
      vertices.insert(vertices.end(), {x + dx, y - dy, z, color.r, color.g, color.b});
      vertices.insert(vertices.end(), {x + dx, y + dy, z, color.r, color.g, color.b});
      vertices.insert(vertices.end(), {x - dx, y + dy, z, color.r, color.g, color.b});
      vertices.insert(vertices.end(), {x - dx, y - dy, z, color.r, color.g, color.b});
    }
  }

  std::vector<GLuint> indices;
  for (int kx = 0; kx < nx - 1; kx++) {
    for (int ky = 0; ky < ny - 1; ky++) {
      //
      //     1u  2u
      //   ---------
      //   | 1   2 | 2r
      //   |       |
      //   | 4   3 | 3r
      //   ---------
      //
      //  ^ x
      //  |
      //  |   y
      //  ---->
      //
      const int i1 = 4 * (kx * ny + ky);
      const int i2 = i1 + 1;
      const int i3 = i2 + 1;
      const int i4 = i3 + 1;
      const int i2r = i4 + 1;
      const int i3r = i4 + 4;
      const int i2u = 4 * ((kx + 1) * ny + ky) + 2;
      const int i1u = i2u + 1;
      // flat 1
      indices.push_back(i3);
      indices.push_back(i2);
      indices.push_back(i1);
      // flat 2
      indices.push_back(i1);
      indices.push_back(i4);
      indices.push_back(i3);
      // right side triangle 1
      indices.push_back(i3r);
      indices.push_back(i2r);
      indices.push_back(i2);
      // right side triangle 2
      indices.push_back(i2);
      indices.push_back(i3);
      indices.push_back(i3r);
      // top side triangle 1
      indices.push_back(i2);
      indices.push_back(i2u);
      indices.push_back(i1u);
      // bottom side triangle 2
      indices.push_back(i1u);
      indices.push_back(i1);
      indices.push_back(i2);
    }
  }

  const auto num_indices = static_cast<GLint>(indices.size());
  const auto vertex_buffer_size = static_cast<GLint>(sizeof(vertices[0]) * vertices.size());
  const auto index_buffer_size = static_cast<GLint>(sizeof(indices[0]) * indices.size());

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

  if (index_buffer_size == index_buffer_size_) {
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_size, indices.data());
  } else {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, indices.data(), GL_DYNAMIC_DRAW);
    index_buffer_size_ = index_buffer_size;
  }

  if (vertex_buffer_size == vertex_buffer_size_) {
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer_size, vertices.data());
  } else {
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertices.data(), GL_DYNAMIC_DRAW);
    vertex_buffer_size_ = vertex_buffer_size;
  }

  if (num_indices != num_indices_) {
    num_indices_ = num_indices;
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Cubemesh::~Cubemesh() {
  // de-allocate all resources once they've outlived their purpose:
  // ------------------------------------------------------------------------
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vbo_);
  glDeleteBuffers(1, &ebo_);
}

};  // namespace bb3d
