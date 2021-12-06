#include "shader.hpp"

#include <GL/glew.h>  // for glGetUniformLocation, GLuint, GL_FALSE, glAttachShader

#include <cstdlib>               // for exit, NULL, EXIT_FAILURE
#include <fstream>               // for operator<<, endl, basic_ostream, ostream, ifstream, basi...
#include <glm/glm.hpp>           // for mat2, mat3, mat4, vec2, vec3, vec4
#include <glm/gtc/type_ptr.hpp>  // for value_ptr
#include <iostream>              // for cerr
#include <sstream>
#include <string>  // for string, operator<<, allocator, operator!=, char_traits

#include "bb3d/assert.hpp"

namespace bb3d {

static inline std::string ReadFile(const std::string &path) {
  std::ifstream file(path, std::ifstream::in);
  if (!file) {
    std::cerr << "Shader unable to open '" << path << "'." << std::endl;
    exit_thread_safe(EXIT_FAILURE);
  }
  std::stringstream shader_stream;
  shader_stream << file.rdbuf();
  file.close();
  return shader_stream.str();
}

static void CheckCompileErrors(GLuint shader, const std::string &type) {
  GLint success = 0;
  std::string infoLog(1024, '\0');
  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == 0) {
      glGetShaderInfoLog(shader, static_cast<GLsizei>(infoLog.size()), nullptr, infoLog.data());
      std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << std::endl;
      std::cerr << infoLog << std::endl;
      std::cerr << " -- --------------------------------------------------- -- " << std::endl;
      exit_thread_safe(EXIT_FAILURE);
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (success == 0) {
      glGetProgramInfoLog(shader, static_cast<GLsizei>(infoLog.size()), nullptr, infoLog.data());
      std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << std::endl;
      std::cerr << infoLog << std::endl;
      std::cerr << " -- --------------------------------------------------- -- " << std::endl;
      exit_thread_safe(EXIT_FAILURE);
    }
  }
}

// constructor generates the shader on the fly
// ------------------------------------------------------------------------
Shader::Shader(const std::string &vshader_path, const std::string &fshader_path,
               const std::string &gshader_path) : program_id_{} {
  // vertex shader
  const GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
  std::string vshader_code = ReadFile(vshader_path);
  const char *vshader_code_str = vshader_code.c_str();
  glShaderSource(vertex, 1, &vshader_code_str, nullptr);
  glCompileShader(vertex);
  CheckCompileErrors(vertex, "VERTEX");

  // fragment Shader
  GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
  std::string fshader_code = ReadFile(fshader_path);
  ;
  const char *fshader_code_str = fshader_code.c_str();
  glShaderSource(fragment, 1, &fshader_code_str, nullptr);
  glCompileShader(fragment);
  CheckCompileErrors(fragment, "FRAGMENT");

  // if geometry shader is given, compile geometry shader
  GLuint geometry = 0;
  if (!gshader_path.empty()) {
    const std::string gshader_code = ReadFile(gshader_path);
    const char *gshader_code_str = gshader_code.c_str();
    geometry = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry, 1, &gshader_code_str, nullptr);
    glCompileShader(geometry);
    CheckCompileErrors(geometry, "GEOMETRY");
  }

  // shader Program
  program_id_ = glCreateProgram();
  glAttachShader(program_id_, vertex);
  glAttachShader(program_id_, fragment);
  if (!gshader_path.empty()) {
    glAttachShader(program_id_, geometry);
  }
  glLinkProgram(program_id_);
  CheckCompileErrors(program_id_, "PROGRAM");

  // delete the shaders as they're linked into our program now and no longer necessery
  glDeleteShader(vertex);
  glDeleteShader(fragment);
  if (!gshader_path.empty()) {
    glDeleteShader(geometry);
  }
}

Shader::~Shader() { glDeleteProgram(program_id_); }

// activate the shader
// ------------------------------------------------------------------------
void Shader::UseProgram() const { glUseProgram(program_id_); }

void Shader::VertexAttribPointer(const char *name, GLint size, GLenum type, GLboolean normalized,
                                 GLsizei stride, const void *pointer) const {
  glVertexAttribPointer(glGetAttribLocation(program_id_, name), size, type, normalized, stride,
                        pointer);
}

// utility uniform functions
// ------------------------------------------------------------------------
void Shader::Uniform1i(const char *name, int value) const {
  glUniform1i(glGetUniformLocation(program_id_, name), value);
}
// ------------------------------------------------------------------------
void Shader::Uniform1f(const char *name, float value) const {
  glUniform1f(glGetUniformLocation(program_id_, name), value);
}
// ------------------------------------------------------------------------
void Shader::Uniform2fv(const char *name, const glm::vec2 &value) const {
  glUniform2fv(glGetUniformLocation(program_id_, name), 1, glm::value_ptr(value));
}
void Shader::Uniform2f(const char *name, float x, float y) const {
  glUniform2f(glGetUniformLocation(program_id_, name), x, y);
}
// ------------------------------------------------------------------------
void Shader::Uniform3fv(const char *name, const glm::vec3 &value) const {
  glUniform3fv(glGetUniformLocation(program_id_, name), 1, glm::value_ptr(value));
}
void Shader::Uniform3f(const char *name, float x, float y, float z) const {
  glUniform3f(glGetUniformLocation(program_id_, name), x, y, z);
}
// ------------------------------------------------------------------------
void Shader::Uniform4fv(const char *name, const glm::vec4 &value) const {
  glUniform4fv(glGetUniformLocation(program_id_, name), 1, glm::value_ptr(value));
}
void Shader::Uniform4f(const char *name, float x, float y, float z, float w) const {
  glUniform4f(glGetUniformLocation(program_id_, name), x, y, z, w);
}
// ------------------------------------------------------------------------
void Shader::UniformMatrix2fv(const char *name, const glm::mat2 &value) const {
  glUniformMatrix2fv(glGetUniformLocation(program_id_, name), 1, GL_FALSE, glm::value_ptr(value));
}
// ------------------------------------------------------------------------
void Shader::UniformMatrix3fv(const char *name, const glm::mat3 &value) const {
  glUniformMatrix3fv(glGetUniformLocation(program_id_, name), 1, GL_FALSE, glm::value_ptr(value));
}
// ------------------------------------------------------------------------
void Shader::UniformMatrix4fv(const char *name, const glm::mat4 &value) const {
  glUniformMatrix4fv(glGetUniformLocation(program_id_, name), 1, GL_FALSE, glm::value_ptr(value));
}

};  // namespace bb3d
