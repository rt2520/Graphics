#ifndef TEXTURE_H
#define TEXTURE_H
#include <stdint.h>
#if defined(__APPLE__) || defined(MACOSX)
#   include <OpenGL/gl.h>
#else
#   include <GL/glut.h>
#endif

GLuint loadTexture(const char * imagepath);

#endif // TEXTURE_H
