#ifndef GLAD_GLAD_H
#define GLAD_GLAD_H

/**
 * Convenience compatibility header for OpenGL 3.3 Core profile.
 * Standard GLAD v2 generates glad/gl.h.
 * This header forwards to glad/gl.h so that standard code including
 * <glad/glad.h> or calling gladLoadGLLoader compiles seamlessly.
 */
#include <glad/gl.h>

#ifndef gladLoadGLLoader
typedef GLADapiproc (*GLADloadproc)(const char *name);
inline int gladLoadGLLoader(GLADloadproc load) {
    return gladLoadGL((GLADloadfunc)load);
}
#endif

#endif // GLAD_GLAD_H
