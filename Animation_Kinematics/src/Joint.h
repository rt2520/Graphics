#ifndef JOINT_H
#define JOINT_H
#include <stdint.h>
#if defined(__APPLE__) || defined(MACOSX)
#   include <OpenGL/gl.h>
#else
#   include <GL/glut.h>
#endif

#include <vector>
#include "utils/Eigen/Dense"

using namespace Eigen;

class Joint
{
    public:
        Joint()
        {

        }

        Joint(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, int);
        Vector3f endPoint;
        Vector3f axis;
        GLfloat rotation;
        int selName;
};

#endif // JOINT_H
