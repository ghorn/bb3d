#pragma once

#include <GL/glew.h>  // for GLchar, GLuint

#include <glm/glm.hpp>  // for vec3
#include <map>          // for map
#include <string>       // for string

#include "bb3d/shader/shader.hpp"  // for GLFWwindow, Shader

namespace bb3d {

class Freetype {
 public:
  explicit Freetype(int font_size);
  ~Freetype() = default;
  void RenderText(const glm::mat4& orthographic_projection, const std::string& text, float x,
                  float y, glm::vec3 color);

 private:
  Shader shader_;
  GLuint vao_{};
  GLuint vbo_{};

  // Holds all state information relevant to a character as loaded using FreeType
  struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2 Size;         // Size of glyph
    glm::ivec2 Bearing;      // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Horizontal offset to advance to next glyph
  };

  std::map<GLchar, Character> characters_;
};

};  // namespace bb3d
