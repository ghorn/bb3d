#pragma once

#include <GL/glew.h>  // for GLenum, GLsizei
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace bb3d {

GLenum GlCheckError_(const char *file, int line);
#define GL_CHECK_ERROR() glCheckError_(__FILE__, __LINE__)

void GlDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
                   const char *message, const void *userParam);

};  // namespace bb3d
