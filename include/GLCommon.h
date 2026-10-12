#ifndef SHADOWFORGE_GL_COMMON_H
#define SHADOWFORGE_GL_COMMON_H

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>
#else
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#endif

#endif // SHADOWFORGE_GL_COMMON_H
