#pragma once

#include <GL/glew.h>  // for GLboolean, GLenum, GLint, GLsizei, GLuint
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>  // for mat4, vec3, vec4
#include <string>       // for string

namespace bb3d {

class Shader {
 public:
  // constructor generates the shader on the fly
  // ------------------------------------------------------------------------
  Shader(const std::string &vshader_path, const std::string &fshader_path,
         const std::string &gshader_path = std::string());
  ~Shader();
  // activate the shader
  // ------------------------------------------------------------------------
  void UseProgram() const;
  void VertexAttribPointer(const char *name, GLint size, GLenum type, GLboolean normalized,
                           GLsizei stride, const void *pointer) const;
  // utility uniform functions
  void Uniform1i(const char *name, int value) const;
  void Uniform1f(const char *name, float value) const;
  void Uniform2fv(const char *name, const glm::vec2 &value) const;
  void Uniform2f(const char *name, float x, float y) const;
  void Uniform3fv(const char *name, const glm::vec3 &value) const;
  void Uniform3f(const char *name, float x, float y, float z) const;
  void Uniform4fv(const char *name, const glm::vec4 &value) const;
  void Uniform4f(const char *name, float x, float y, float z, float w) const;
  void UniformMatrix2fv(const char *name, const glm::mat2 &value) const;
  void UniformMatrix3fv(const char *name, const glm::mat3 &value) const;
  void UniformMatrix4fv(const char *name, const glm::mat4 &value) const;

 private:
  GLuint program_id_;
};

};  // namespace bb3d
