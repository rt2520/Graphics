/* Simple opengl demo program.
 */

//#elif defined(__linux)
#if defined(__APPLE__) || defined(MACOSX)
#   include <GLUT/glut.h>
#else
#   include <GL/glut.h>
#endif

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include "GLScreenCapturer.h"
#include "trackball.h"
#include "Texture.h"
#include "Tree.h"


using namespace std;


#define BUFFER_LENGTH 64
#define drawOneLine(x1,y1,x2,y2)  glBegin(GL_LINES);  \
    glVertex2f ((x1),(y1)); glVertex2f ((x2),(y2)); glEnd();

#define degree(x) (x * 180 / 3.1416)
#define radian(x) (x * 3.1416 / 180)


const float pinDia = 0.8;
//GLfloat camRotX, camRotY, camPosX, camPosY, camPosZ;
enum mode {FORWARD, INVERSE};
int currMode = FORWARD;
enum selectedModel {MODEL1 , MODEL2};
int currentModel = MODEL1;
enum selectedJoint1 {JOINT1, JOINT2, JOINT3, JOINT4};
int currentJoint1 = JOINT1;
enum selectedJoint2 {JOINT11 = 4, JOINT12, JOINT13, JOINT21, JOINT22, JOINT23, JOINT24,
                        JOINT31, JOINT32, JOINT33, JOINT41, JOINT42, JOINT43, JOINT44, JOINT45};
int currentJoint2 = JOINT12;

bool animating = false;

enum ikChain {CHAIN1, CHAIN2, CHAIN3, CHAIN4, CHAIN5};
int currIKChain = CHAIN1;

float near = 0.5f;
float far = 100.0f;
float camZ = 15.0;

Vector3f targetPos(6.0 * cos(0.5), 6.0* sin(0.5), 0.0);
Vector3f currentPos[5];
float smallIncrementIK = 0.0;

Joint joint1(0.0, 0.0, 1.0, 2.75, 0.0, 0.0, 0.0, JOINT1);
Joint joint2(0.0, 0.0, 1.0, 2.75, 0.0, 0.0, 10.0, JOINT2);
Joint joint3(0.0, 0.0, 1.0, 2.75, 0.0, 0.0, 0.0, JOINT3);
Joint joint4(0.0, 0.0, 1.0, 2.75, 0.0, 0.0, 0.0, JOINT4);
Node model1;

Joint joint11(0.0, 0.0, 1.0, 2.75, 0.0, 0.0, 0.0, JOINT11);
Joint joint12(0.0, 0.0, 1.0, 4.0, 0.0, 0.0, 10.0, JOINT12);
Joint joint13(0.0, 0.0, 1.0, 2.75, 0.0, 0.0, 0.0, JOINT13);
Node model2;

Joint joint21(0.0, 0.0, 1.0, 2.5, 0.0, 0.0, 60.0, JOINT21);
Joint joint22(0.0, 0.0, 1.0, 2.75, 0.0, 0.0, 10.0, JOINT22);
Joint joint23(0.0, 0.0, 1.0, 1.5, 0.0, 0.0, 0.0, JOINT23);
Joint joint24(0.0, 0.0, 1.0, 2.0, 0.0, 0.0, 0.0, JOINT24);
Node model3;

Joint joint31(0.0, 0.0, 1.0, 2.75, 0.0, 0.0, -60.0, JOINT31);
Joint joint32(0.0, 0.0, 1.0, 2.75, 0.0, 0.0, 10.0, JOINT32);
Joint joint33(0.0, 0.0, 1.0, 2.75, 0.0, 0.0, 0.0, JOINT33);
Node model4;

Joint joint41(0.0, 0.0, 1.0, 2.0, 0.0, 0.0, 180.0, JOINT41);
Joint joint42(0.0, 0.0, 1.0, 2.5, 0.0, 0.0, 10.0, JOINT42);
Joint joint43(0.0, 0.0, 1.0, 1.5, 0.0, 0.0, 0.0, JOINT43);
Joint joint44(0.0, 0.0, 1.0, 3.0, 0.0, 0.0, 0.0, JOINT44);
Joint joint45(0.0, 0.0, 1.0, 2.5, 0.0, 0.0, 0.0, JOINT45);
Node model5;

const int FPS = 60;
GLint viewport[4];
GLdouble modelview[16];
GLdouble projection[16];

GLuint pickedObj = -1;
char titleString[150];

int winWidth = 960;
int winHeight = 540;


// Lights & Materials
//GLfloat ambient[] = {0.2, 0.2, 0.2, 1.0};
GLfloat ambient[] = {1.0, 1.0, 1.0, 1.0};
GLfloat position[] = {0.0, 0.0, -10.0, 1.0};
GLfloat mat_diffuse[] = {1.0, 1.0, 1.0, 1.0};
GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat mat_shininess[] = {50.0};

static GLScreenCapturer screenshot("screenshot-%d.ppm");

GLfloat norm(Vector3f v)
{
    return (GLfloat)sqrt (v(0) * v(0) + v(1) * v(1) + v(2) * v(2));
}

MatrixXf getPseudoInverse(MatrixXf J)
{
    MatrixXf JTr = J.transpose();
    MatrixXf JJTr = (J * JTr);
    MatrixXf JJTrPlusLambdaIInv = (JJTr + 0.000001 * MatrixXf::Identity(JJTr.rows(), JJTr.cols())).inverse();
    return JTr * JJTrPlusLambdaIInv;
}


void incrementAnglesRecursively(Node model, MatrixXf dTheta, int level)
{
    model.joint->rotation += degree(dTheta(level));
    for (int i = 0; i < model.children.size(); i++)
        incrementAnglesRecursively(model.children[i], dTheta, level + i + 1);
}

MatrixXf getJacobian(Node model, MatrixXf &J, int col, float prevAngle)
{
    float length = norm(model.joint->endPoint);
    float angle = model.joint->rotation + prevAngle;
    Vector3f tmp(-length * sin(radian(angle)),length * cos(radian(angle)), model.joint->endPoint(2));
    if (model.children.size() > 0)
    {
        Vector3f val = tmp + getJacobian(model.children[0], J, col + 1, angle);
        J(0 , col) = val(0);
        J(1 , col) = val(1);
        J(2 , col) = val(2);
        return (val);
    }
    else
    {
        J(0 , col) = tmp(0);
        J(1 , col) = tmp(1);
        J(2 , col) = tmp(2);
        return tmp;
    }
}

Vector3f getEffectorPos(Node model, float prevAngle)
{
    float length = norm(model.joint->endPoint);
    float angle = model.joint->rotation + prevAngle;
    Vector3f tmp(length * cos(radian(angle)),length * sin(radian(angle)), model.joint->endPoint(2));
    if (model.children.size() > 0)
    {
        Vector3f val = tmp + getEffectorPos(model.children[0], angle);
        return (val);
    }
    else
    {
        return tmp;
    }
}

int getDepth(Node model)
{
    if (model.children.size() == 0)
        return 1;
    return 1 + getDepth(model.children[0]);
}

bool updateMovementAngles(Node model, int effector)
{
    //Vector3f currentEffectorPos = currentPos[effector];

    int joints = getDepth(model);
    MatrixXf jacobian = MatrixXf::Zero(3, joints);
    Vector3f currentEffectorPos = getEffectorPos(model, 0.0);
    getJacobian(model, jacobian, 0, 0.0);
    Vector3f dPos = (targetPos - currentEffectorPos);
    //while (norm(dPos) > 0.1)
        dPos *= 0.1;
    MatrixXf dTheta = getPseudoInverse(jacobian) * dPos;

    incrementAnglesRecursively(model, dTheta, 0);
    if (norm(targetPos - getEffectorPos(model, 0.0)) < 0.001)
        return true;
    if (norm(targetPos - getEffectorPos(model, 0.0)) > norm(targetPos - currentEffectorPos))
    {
        incrementAnglesRecursively(model, -1 * dTheta, 0);
        return true;
    }
    else
        return false;


}

void initModels()
{
    vector<Node> v;
    Node node(&joint4, std::vector<Node>());//, NULL,

    v.push_back(node);
    node = Node(&joint3, v);// NULL,

    v.clear();
    v.push_back(node);
    node = Node(&joint2, v);// NULL,

    v.clear();
    v.push_back(node);
    model1 = Node(&joint1, v);//, NULL

    //CHAIN 2
    v.clear();
    node = Node(&joint13, v);// NULL,

    v.push_back(node);
    node = Node(&joint12, v);// NULL,

    v.clear();
    v.push_back(node);
    model2 = Node(&joint11, v);//, NULL*/

    //CHAIN 3
    v.clear();
    node = Node(&joint24, v);// NULL,

    v.push_back(node);
    node = Node(&joint23, v);// NULL,

    v.clear();
    v.push_back(node);
    node = Node(&joint22, v);// NULL,

    v.clear();
    v.push_back(node);
    model3 = Node(&joint21, v);//, NULL*/

    //CHAIN 4
    v.clear();
    node = Node(&joint33, v);// NULL,

    v.push_back(node);
    node = Node(&joint32, v);// NULL,

    v.clear();
    v.push_back(node);
    model4 = Node(&joint31, v);//, NULL*/

    //CHAIN 5
    v.clear();
    node = Node(&joint45, v);// NULL,

    v.push_back(node);
    node = Node(&joint44, v);// NULL,

    v.clear();
    v.push_back(node);
    node = Node(&joint43, v);// NULL,

    v.clear();
    v.push_back(node);
    node = Node(&joint42, v);// NULL,

    v.clear();
    v.push_back(node);
    model5 = Node(&joint41, v);//, NULL*/
}

void initLights(void)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    //glEnable(GL_BLEND);
    glDisable(GL_BLEND);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, mat_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

}

void reshape( int w, int h )
{
    winWidth = w;
    winHeight = h;
    tbReshape(w, h);

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Set the clipping volume
    gluPerspective(60.0f, (GLfloat)w / (GLfloat)h, near, far);
    //glOrtho(0.0, (GLdouble) winWidth, 0.0, (GLdouble) winHeight, 1.0f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}


void timer(int v) {

    if (currentModel == MODEL1)
    {
        if (!updateMovementAngles(model1, 0))
        {
            glutTimerFunc(1000/FPS, timer, v);
        }
    }
    else if (currIKChain == CHAIN2)
    {
        if (!updateMovementAngles(model2, 0))
        {
            glutTimerFunc(1000/FPS, timer, v);
        }
    }
    else if (currIKChain == CHAIN3)
    {
        if (!updateMovementAngles(model3, 0))
        {
            glutTimerFunc(1000/FPS, timer, v);
        }
    }
    else if (currIKChain == CHAIN4)
    {
        if (!updateMovementAngles(model4, 0))
        {
            glutTimerFunc(1000/FPS, timer, v);
        }
    }
    else if (currIKChain == CHAIN5)
    {
        if (!updateMovementAngles(model5, 0))
        {
            glutTimerFunc(1000/FPS, timer, v);
        }
    }

    glutPostRedisplay();
}

int animatePosGen = 0;

int getClosestChain()
{
    float min = norm(getEffectorPos(model2, 0.0) - targetPos);
    int chain = CHAIN2;
    float tmp = norm(getEffectorPos(model3, 0.0) - targetPos);
    if (tmp < min)
    {
        min = tmp;
        chain = CHAIN3;
    }
    tmp = norm(getEffectorPos(model4, 0.0) - targetPos);
    if (tmp < min)
    {
        min = tmp;
        chain = CHAIN4;
    }
    tmp = norm(getEffectorPos(model5, 0.0) - targetPos);
    if (tmp < min)
    {
        min = tmp;
        chain = CHAIN5;
    }
    return chain;
}


void animatedTimer(int v) {

    if (animating && currMode == INVERSE)
    {
        if (currentModel == MODEL1)
        {
            if (!updateMovementAngles(model1, 0))
            {
                glutTimerFunc(1000/FPS, animatedTimer, v);
            }
            else if (animatePosGen++ <= 10)
            {
                srand (time(NULL));
                targetPos(0) = ((rand() % 2 == 0) ? -1 : 1) * (rand() % 8);
                targetPos(1) = ((rand() % 2 == 0) ? -1 : 1) * (rand() % 8);
                glutTimerFunc(1000/FPS, animatedTimer, v);
            }
            else
                animating = false;
        }
        else
        {
            bool rerun = false;
            if (currIKChain == CHAIN2)
                rerun = !updateMovementAngles(model2, 0);
            if (currIKChain == CHAIN3)
                rerun = !updateMovementAngles(model3, 0);
            if (currIKChain == CHAIN4)
                rerun = !updateMovementAngles(model4, 0);
            if (currIKChain == CHAIN5)
                rerun = !updateMovementAngles(model5, 0);
            if (rerun)
            {
                glutTimerFunc(1000/FPS, animatedTimer, v);
            }
            else if (animatePosGen++ <= 10)
            {
                srand (time(NULL));
                targetPos(0) = ((rand() % 2 == 0) ? -1 : 1) * (rand() % 8);
                targetPos(1) = ((rand() % 2 == 0) ? -1 : 1) * (rand() % 8);
                currIKChain = getClosestChain();
                glutTimerFunc(1000/FPS, animatedTimer, v);
            }
            else
                animating = false;
        }

    }

    glutPostRedisplay();
}


void setupRC()
{
    tbInit(GLUT_RIGHT_BUTTON);
    tbAnimate(GL_TRUE);
    
    // Place Camera
    //    camRotX = 350.0f;
    //    camRotY = 680.0f;
    //    camPosX = 0.0f;
    //    camPosY = 0.0f;
    //    camPosZ = -10.5f;
    
    //glEnable( GL_DEPTH_TEST );

    //glShadeModel(GL_SMOOTH);
    //glClearColor(0.0, 0.0, 0.0, 0.0);
    //glShadeModel (GL_FLAT);
    initLights();
    //initDisplayLists();

}

void setCamera( void )
{
    /*glTranslatef(0, 0, camPosZ);
    glRotatef(camRotX, 1, 0, 0);
    glRotatef(camRotY, 0, 1, 0);*/
    glTranslatef(0.0, 0.0, -1 * camZ);
    //glScalef(0.1, 0.1, 0.1);
    //gluLookAt(0.0, 2.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}


GLfloat selColor[] = {0, 1, 0, 1};

void drawNodeRecursively(Node node, bool pickable, GLfloat jointColor[], GLfloat linkColor[])
{
    glPushMatrix();
    {
        Joint* currJoint = node.joint;
        glPushMatrix();
        {
            if (currentJoint1 == currJoint->selName || currentJoint2 == currJoint->selName)
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, selColor);
            else
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, jointColor);
            if (currentModel == MODEL2 && (currentJoint2 == JOINT11 || currentJoint2 == JOINT21
                            || currentJoint2 == JOINT31 || currentJoint2 == JOINT41))
            {
                if (currJoint->selName == JOINT11 || currJoint->selName == JOINT21 ||
                        currJoint->selName == JOINT31 || currJoint->selName == JOINT41)
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, selColor);
            }
            //glLoadName(currJoint->selName);
//            if (pickable)
//                glColor3ub(currJoint->selName + 1,0,0);
            glutSolidSphere(0.4, 20, 20);
        }
        glPopMatrix();

        //glTranslatef(0.25, 0.0, 0.0);
        glRotatef(currJoint->rotation, currJoint->axis(0), currJoint->axis(1), currJoint->axis(2));


        glPushMatrix();
        {
            glRotatef(90.0, 0.0, 1.0, 0.0);
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, linkColor);
            GLUquadric* cylQuad = gluNewQuadric();
//            glLoadName(currJoint->selName);
//            if (pickable)
//                glColor3ub(currJoint->selName + 1,0,0);
            if (cylQuad != 0)
                gluCylinder(cylQuad, 0.25, 0.25, norm(currJoint->endPoint), 10, 50);
        }
        glPopMatrix();

        glTranslatef(currJoint->endPoint(0), currJoint->endPoint(1), currJoint->endPoint(2));//joint1.axleLength + joint2.radius, 0.0, 0.0);

        for (int i = 0; i < node.children.size(); i++)
            drawNodeRecursively(node.children[i], pickable, jointColor, linkColor);
    }
    glPopMatrix();
}

void drawModel1 (bool pickable)
{
    GLfloat c1[] = {0, 0, 0, 1};
    GLfloat c2[] = {0, 0, 0, 1};
    if (currentModel == MODEL1)
    {
        c1[0] = 1; c1[1] = 0; c1[2] = 0;
        c2[0] = 1; c2[1] = 0; c2[2] = 1;
        drawNodeRecursively(model1, pickable, c1, c2);
    }
    if (currentModel == MODEL2)
    {
        c1[0] = 1; c1[1] = 0; c1[2] = 0;
        c2[0] = 1; c2[1] = 0; c2[2] = 1;
        drawNodeRecursively(model2, pickable, c1, c2);

        c1[0] = 0; c1[1] = 0; c1[2] = 1;
        c2[0] = 0.6; c2[1] = 0.4; c2[2] = 0.2;
        drawNodeRecursively(model3, pickable, c1, c2);

        c1[0] = 1; c1[1] = 1; c1[2] = 0;
        c2[0] = 0.2; c2[1] = 0.4; c2[2] = 0.6;
        drawNodeRecursively(model4, pickable, c1, c2);

        c1[0] = 0; c1[1] = 1; c1[2] = 1;
        c2[0] = 0.7; c2[1] = 0.8; c2[2] = 0.4;
        drawNodeRecursively(model5, pickable, c1, c2);
    }
    // Retrieve current matrice before they popped.
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );        // Retrieve The Modelview Matrix
    glGetDoublev( GL_PROJECTION_MATRIX, projection );    // Retrieve The Projection Matrix
    glGetIntegerv( GL_VIEWPORT, viewport );                // Retrieves The Viewport Values (X, Y, Width, Height)

}
void display(void)
{
    glLoadIdentity ();             /* clear the matrix */
    setCamera();
    tbMatrix();
    GLfloat currentColor[] = {1,1,0,1};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, currentColor);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawModel1(false);
    if (currMode == INVERSE)
    {
        glPushMatrix();
        GLfloat pointColor[] = {1,1,1,1};
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pointColor);
            glTranslatef(targetPos(0), targetPos(1), targetPos(2));
            glutSolidSphere(0.1, 20, 20);
        glPopMatrix();
    }
    glFlush ();
    glutSwapBuffers();
}

void specialKeys( int key, int x, int y )
{
    switch(key)
    {
        case GLUT_KEY_UP:
            if (currMode == FORWARD)
            {
                if (currentModel == MODEL1)
                {
                    if (currentJoint1 == JOINT1) //&& joint1.rotation < 80)
                        joint1.rotation += 1;
                    else if (currentJoint1 == JOINT2) //&& joint2.rotation < 80)
                        joint2.rotation += 1;
                    else if (currentJoint1 == JOINT3) //&& joint3.rotation < 80)
                        joint3.rotation += 1;
                    else if (currentJoint1 == JOINT4) //&& joint4.rotation < 80)
                        joint4.rotation += 1;
                }
                else if (currentModel == MODEL2)
                {
                    if (currentJoint2 == JOINT11) //&& joint11.rotation < 80)
                        joint11.rotation += 1;
                    else if (currentJoint2 == JOINT12) //&& joint12.rotation < 80)
                    {
                        joint12.rotation += 1;
                    }
                    else if (currentJoint2 == JOINT13) //&& joint13.rotation < 80)
                        joint13.rotation += 1;
                    else if (currentJoint2 == JOINT21) //&& joint21.rotation < 80)
                        joint21.rotation += 1;
                    else if (currentJoint2 == JOINT22) //&& joint22.rotation < 80)
                        joint22.rotation += 1;
                    else if (currentJoint2 == JOINT23) //&& joint23.rotation < 80)
                        joint23.rotation += 1;
                    else if (currentJoint2 == JOINT24) //&& joint24.rotation < 80)
                        joint24.rotation += 1;
                    else if (currentJoint2 == JOINT31) //&& joint31.rotation < 80)
                        joint31.rotation += 1;
                    else if (currentJoint2 == JOINT32) //&& joint32.rotation < 80)
                        joint32.rotation += 1;
                    else if (currentJoint2 == JOINT33) //&& joint33.rotation < 80)
                        joint33.rotation += 1;
                    else if (currentJoint2 == JOINT41) //&& joint41.rotation < 80)
                        joint41.rotation += 1;
                    else if (currentJoint2 == JOINT42) //&& joint42.rotation < 80)
                        joint42.rotation += 1;
                    else if (currentJoint2 == JOINT43) //&& joint43.rotation < 80)
                        joint43.rotation += 1;
                    else if (currentJoint2 == JOINT44) //&& joint44.rotation < 80)
                        joint44.rotation += 1;
                    else if (currentJoint2 == JOINT45) //&& joint45.rotation < 80)
                        joint45.rotation += 1;

                }
                glutPostRedisplay();
            }
            break;
        case GLUT_KEY_DOWN:
            if (currMode == FORWARD)
            {
                if (currentModel == MODEL1)
                {
                    if (currentJoint1 == JOINT1) //&& joint1.rotation > -80)
                        joint1.rotation -= 1;
                    else if (currentJoint1 == JOINT2) //&& joint2.rotation > -80)
                        joint2.rotation -= 1;
                    else if (currentJoint1 == JOINT3) //&& joint3.rotation > -80)
                        joint3.rotation -= 1;
                    else if (currentJoint1 == JOINT4) //&& joint4.rotation > -80)
                        joint4.rotation -= 1;
                }
                else if (currentModel == MODEL2)
                {
                    if (currentJoint2 == JOINT11) //&& joint11.rotation > -80)
                        joint11.rotation -= 1;
                    else if (currentJoint2 == JOINT12) //&& joint12.rotation > -80)
                        joint12.rotation -= 1;
                    else if (currentJoint2 == JOINT13) //&& joint13.rotation > -80)
                        joint13.rotation -= 1;
                    else if (currentJoint2 == JOINT21) //&& joint21.rotation > -80)
                        joint21.rotation -= 1;
                    else if (currentJoint2 == JOINT22) //&& joint22.rotation > -80)
                        joint22.rotation -= 1;
                    else if (currentJoint2 == JOINT23) //&& joint23.rotation > -80)
                        joint23.rotation -= 1;
                    else if (currentJoint2 == JOINT24) //&& joint24.rotation > -80)
                        joint24.rotation -= 1;
                    else if (currentJoint2 == JOINT31) //&& joint31.rotation > -80)
                        joint31.rotation -= 1;
                    else if (currentJoint2 == JOINT32) //&& joint32.rotation > -80)
                        joint32.rotation -= 1;
                    else if (currentJoint2 == JOINT33) //&& joint33.rotation > -80)
                        joint33.rotation -= 1;
                    else if (currentJoint2 == JOINT41) //&& joint41.rotation > -80)
                        joint41.rotation -= 1;
                    else if (currentJoint2 == JOINT42) //&& joint42.rotation > -80)
                        joint42.rotation -= 1;
                    else if (currentJoint2 == JOINT43) //&& joint43.rotation > -80)
                        joint43.rotation -= 1;
                    else if (currentJoint2 == JOINT44) //&& joint44.rotation > -80)
                        joint44.rotation -= 1;
                    else if (currentJoint2 == JOINT45) //&& joint45.rotation > -80)
                        joint45.rotation -= 1;

                }
                glutPostRedisplay();
            }
            break;

        //case GLUT_KEY_LEFT:

        //case GLUT_KEY_RIGHT:

    }
}


void keyboard( unsigned char key, int x, int y )
{
    switch(key)
    {
    case 27: // Escape key
        exit(0);
        break;
    case 'r':
        printf("save current screen\n");
        screenshot.capture();
        break;
    case ' ': //Space bar
        if (currMode == FORWARD)
        {
            currMode = INVERSE;
            glutPostRedisplay();
            //Update Current positions currentPos[5]
//            glutTimerFunc(1000/FPS, timer, v);
        }
        else
            currMode = FORWARD;
        break;
    case '1':
        currentModel = MODEL1;
        currentJoint1 = JOINT1;
        glutPostRedisplay();
        break;
    case '2':
        currentModel = MODEL2;
        currentJoint2 = JOINT12;
        glutPostRedisplay();
        break;
    case 'a':
        if (currMode == INVERSE)
        {
            if (animating)
                animating = false;
            else
            {
                animating = true;
                srand (time(NULL));
                targetPos(0) = ((rand() % 2 == 0) ? -1 : 1) * (rand() % 8);
                targetPos(1) = ((rand() % 2 == 0) ? -1 : 1) * (rand() % 8);
                if (currentModel == MODEL2)
                    currIKChain = getClosestChain();
                else
                    currIKChain = CHAIN1;
                glutTimerFunc(1000/FPS, animatedTimer, 0);
            }
            animatePosGen = 0;
        }
        break;
    }
}


// Got this from Nehe Gamedev Tutorials
Vector3f GetOGLPos(int x, int y)

{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;

    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );

    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, int(winY), 1.0, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
    return Vector3f(posX, posY, posZ);
}

int selectedJoint(Node model, float prevAngle, Vector3f prevPos, Vector3f click)
{
    float length = norm(model.joint->endPoint);
    float angle = model.joint->rotation + prevAngle;
    Vector3f tmp(length * cos(radian(angle)),length * sin(radian(angle)), model.joint->endPoint(2));
    tmp += prevPos;
//    if (model.children.size() > 0 && norm (click - tmp) < 0.1)
//        return model.children[0].joint->selName;
    if (norm (click - tmp) < 1.0)
        return model.joint->selName;
    else if (model.children.size() > 0)
    {
        return selectedJoint(model.children[0], angle, tmp, click);
    }
    else
    {
        return -1;
    }
}


void resolveSelection (float x, float y)
{
    Vector3f click(x, y, 0.0);
    int selection = -1;
    if (currentModel == MODEL1)
    {
        //if (norm (click) < 0.1)
        //{
        //    currentJoint1 = JOINT1;
        //}
        //else
        //{
            selection = selectedJoint(model1, 0.0, Vector3f(0.0,0.0,0.0), click);
            if (selection != -1)
                currentJoint1 = selection;
        //}
        return;
    }
    else if (currentModel == MODEL2)
    {

            selection = selectedJoint(model2, 0.0, Vector3f(0.0,0.0,0.0), click);
            if (selection != -1)
            {
                currentJoint2 = selection;
                return;
            }
            selection = selectedJoint(model3, 0.0, Vector3f(0.0,0.0,0.0), click);
            if (selection != -1)
            {
                currentJoint2 = selection;
                return;
            }
            selection = selectedJoint(model4, 0.0, Vector3f(0.0,0.0,0.0), click);
            if (selection != -1)
            {
                currentJoint2 = selection;
                return;
            }
            selection = selectedJoint(model5, 0.0, Vector3f(0.0,0.0,0.0), click);
            if (selection != -1)
            {
                currentJoint2 = selection;
                return;
            }

    }
}

void mouse( int button, int state, int x, int y)
{
    tbMouse(button, state, x, y);
    
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        //processSelection(x, y);
        Vector3f openGLCoord = GetOGLPos(x,y);
        if (currMode == INVERSE && !animating)
        {
            targetPos(0) = (camZ / far) * openGLCoord(0);
            targetPos(1) = (camZ / far) * openGLCoord(1);
            if (currentModel == MODEL2)
                currIKChain = getClosestChain();
            else
                currIKChain = CHAIN1;
            glutTimerFunc(1000/FPS, timer, 0);
        }
        else if (currMode == FORWARD)
        {
            resolveSelection((camZ / far) * openGLCoord(0), (camZ / far) * openGLCoord(1));
        }
    }
    
    if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    {
        pickedObj = -1;
        glutPostRedisplay();
    }
}

void motion(int x, int y)
{
    tbMotion(x, y);
    
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;
    
    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
    glutPostRedisplay();

}

int main (int argc, char *argv[])
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize( winWidth, winHeight );

    glutCreateWindow( "KINEMATICS" );
    initModels();
    setupRC();

    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutSpecialFunc( specialKeys );
    glutMouseFunc( mouse );
    glutMotionFunc( motion );

    glutMainLoop();
}

