/*
 * OBJLoader.h
 * author: Rahul Tewari: rt2520
 */
#ifndef OBJLOADER_H
#   define OBJLOADER_H

#include <stdint.h>
#if defined(__APPLE__) || defined(MACOSX)
#   include <OpenGL/gl.h>
#else
#   include <GL/glut.h>
#endif

void load_obj(const char* filename, vector<glm::vec4> &vertices, vector<glm::vec3> &normals, vector<GLushort> &elements);

#endif
