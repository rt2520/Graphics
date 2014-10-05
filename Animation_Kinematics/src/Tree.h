#ifndef TREE_H
#define TREE_H
#include <stdint.h>
#if defined(__APPLE__) || defined(MACOSX)
#   include <OpenGL/gl.h>
#else
#   include <GL/glut.h>
#endif

#include <vector>
#include "Joint.h"

using namespace std;

class Node
{
    public:
        Joint *joint;
        std::vector<Node> children;
        //Node *parent;
        Node ()
        {
        }

        Node (Joint *joint, std::vector<Node> children);//Node *parent,
};

#endif // TREE_H
