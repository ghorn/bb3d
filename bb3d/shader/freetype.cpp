#include "freetype.hpp"

#include <GL/glew.h>            // for GLchar, glBindTexture, glTexParameteri, GL_TEXTURE_2D
#include <freetype/freetype.h>  // for FT_FaceRec_, FT_GlyphSlotRec_, FT_Done_Face, FT_Done_F...
#include <freetype/ftimage.h>   // for FT_Bitmap, FT_Vector

#include <cstdlib>      // for EXIT_FAILURE
#include <glm/glm.hpp>  // for ivec2, vec<>::(anonymous), vec3, mat4
#include <iostream>     // for operator<<, endl, basic_ostream, cerr, ostream
#include <map>          // for map
#include <string>       // for basic_string, allocator, string, operator<<, char_traits
#include <utility>      // for pair

#include "bb3d/assert.hpp"         // for exit_thread_safe
#include "bb3d/shader/shader.hpp"  // for Shader

namespace bb3d {

Freetype::Freetype(int font_size) : shader_("bb3d/shader/freetype.vs", "bb3d/shader/freetype.fs") {
  // FreeType
  // --------
  FT_Library ft = nullptr;
  // All functions return a value different than 0 whenever an error occurred
  if (FT_Init_FreeType(&ft) != 0) {
    std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    exit_thread_safe(EXIT_FAILURE);
  }

  // find path to font
  // load font as face
  // std::string font_path = "/usr/share/fonts/truetype/freefont/FreeMono.ttf";
  std::string font_path = "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";
  // std::string font_path = "/usr/share/fonts/truetype/freefont/FreeSerif.ttf";
  FT_Face face = nullptr;
  if (FT_New_Face(ft, font_path.c_str(), 0, &face) != 0) {
    std::cerr << "ERROR::FREETYPE: Failed to load '" << font_path << "'." << std::endl;
    exit_thread_safe(EXIT_FAILURE);
  }
  // set size to load glyphs as
  FT_Set_Pixel_Sizes(face, 0, font_size);

  // disable byte-alignment restriction
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // load first 128 characters of ASCII set
  for (unsigned char c = 0; c < 128; c++) {
    // Load character glyph
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0) {
      std::cerr << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
      continue;
    }
    // generate texture
    unsigned int texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<GLsizei>(face->glyph->bitmap.width),
                 static_cast<GLsizei>(face->glyph->bitmap.rows), 0, GL_RED, GL_UNSIGNED_BYTE,
                 face->glyph->bitmap.buffer);
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    // now store character for later use
    Character character = {texture, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                           glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                           static_cast<unsigned int>(face->glyph->advance.x)};
    characters_.insert(std::pair<char, Character>(c, character));
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  // destroy FreeType once we're finished
  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  // configure VAO/VBO for texture quads
  // -----------------------------------
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

// render line of text
// -------------------
void Freetype::RenderText(const glm::mat4& orthographic_projection, const std::string& text,
                          float x, float y, glm::vec3 color) {
  const float scale = 1.0F;

  // activate corresponding render state
  shader_.UseProgram();

  // projection transformation
  shader_.UniformMatrix4fv("projection", orthographic_projection);

  shader_.Uniform3f("textColor", color.r, color.g, color.b);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(vao_);

  // enable blending, disable antialiasing
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_POLYGON_SMOOTH);

  // iterate through all characters
  for (char c : text) {
    Character ch = characters_[c];

    float xpos = x + static_cast<float>(ch.Bearing.x) * scale;
    float ypos = y - static_cast<float>(ch.Size.y - ch.Bearing.y) * scale;

    float w = static_cast<float>(ch.Size.x) * scale;
    float h = static_cast<float>(ch.Size.y) * scale;
    // update VBO for each character
    // clang-format off
    float vertices[6][4] = { // NOLINT
        { xpos,     ypos + h,   0.0F, 0.0F },
        { xpos,     ypos,       0.0F, 1.0F },
        { xpos + w, ypos,       1.0F, 1.0F },

        { xpos,     ypos + h,   0.0F, 0.0F },
        { xpos + w, ypos,       1.0F, 1.0F },
        { xpos + w, ypos + h,   1.0F, 0.0F }
    };
    // clang-format on
    // render glyph texture over quad
    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
    // update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices),
                    &(vertices[0][0]));  // be sure to use glBufferSubData and not glBufferData

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    x += static_cast<float>(ch.Advance >> 6) *
         scale;  // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels
                 // by 64 to get amount of pixels))
  }
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

};  // namespace bb3d
