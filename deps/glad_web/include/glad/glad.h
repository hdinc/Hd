// glad's header for the web build. webgl 2 is gles 3.0, its functions need
// no loading.
#pragma once

#include <GLES3/gl3.h>

#define GLAD_GL_VERSION_4_3 0

// shader stages and buffers webgl doesn't have, only named by code that
// isn't used there
#ifndef GL_GEOMETRY_SHADER
#define GL_GEOMETRY_SHADER 0x8DD9
#endif
#ifndef GL_COMPUTE_SHADER
#define GL_COMPUTE_SHADER 0x91B9
#endif
#ifndef GL_SHADER_STORAGE_BUFFER
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#endif

static inline int gladLoadGL(void) { return 1; }
