#include "Joint.h"

using namespace std;

Joint::Joint(GLfloat axisX, GLfloat axisY, GLfloat axisZ,
        GLfloat endX, GLfloat endY, GLfloat endZ, GLfloat rotation, int selName)
{
    endPoint(0) = endX;
    endPoint(1) = endY;
    endPoint(2) = endZ;
    axis(0) = axisX;
    axis(1) = axisY;
    axis(2) = axisZ;
    this->rotation = rotation;
    this->selName = selName;
}
