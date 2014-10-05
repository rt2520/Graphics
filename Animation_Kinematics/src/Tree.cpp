#include "Tree.h"

Node::Node (Joint *joint, std::vector<Node> children)//Node *parent,
{
    this->joint = joint;
    this->children = children;
    //this->parent = parent;
}
