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
//#include "trackball.h"
#include "Texture.h"


using namespace std;

#define BUFFER_LENGTH 64
#define drawOneLine(x1,y1,x2,y2)  glBegin(GL_LINES);  \
    glVertex2f ((x1),(y1)); glVertex2f ((x2),(y2)); glEnd();


const float pinDia = 0.8;
//GLfloat camRotX, camRotY, camPosX, camPosY, camPosZ;
enum states {START, ROLLING, OVER};
int currentState = START;
GLfloat currentAngleOfRotation = 0.0;
GLfloat currentDistFromOrigin = 0.0;
int gameOverCounter = 0;
const int FPS = 60;
GLint viewport[4];
GLdouble modelview[16];
GLdouble projection[16];

GLuint pickedObj = -1;
char titleString[150];

bool isTeapot1_selected = false;
bool isTeapot2_selected = false;

int wallsAndFloor = 0;
int pin = 0;
int sphere = 0;
GLfloat alleyWidth = 7.0;
GLfloat alleyLeft = -1 * alleyWidth / 2;
GLfloat alleyLength = 30.0;
GLfloat gutterWidth = 1.2;
int winWidth = 720;
int winHeight = 640;

//xDir = 1 means positive x and -1 means negative. Speed along x is assumed unit speed
float xBallVel = 0.0;
float zBallVel = -12.0;
GLfloat ballSpeed = 30.0;
bool inGutter = false;
int pinPattern = 63;
short remChances = 0;
short score = -1;

// Lights & Materials
//GLfloat ambient[] = {0.2, 0.2, 0.2, 1.0};
GLfloat ambient[] = {1.0, 1.0, 1.0, 1.0};
GLfloat position[] = {0.0, 0.0, -10.0, 1.0};
GLfloat mat_diffuse[] = {1.0, 1.0, 1.0, 1.0};
GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat mat_shininess[] = {50.0};

static GLScreenCapturer screenshot("screenshot-%d.ppm");

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
    //tbReshape(w, h);

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Set the clipping volume
    gluPerspective(60.0f, (GLfloat)w / (GLfloat)h, 0.5f, 100.0f);
    //glOrtho(0.0, (GLdouble) w, 0.0, (GLdouble) h, 1.0f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void renderText(float x, float y, float z, char* string)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (GLdouble) winWidth, 0.0, (GLdouble) winHeight, 1.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
    {
        //glTranslatef(3.0, 3.0, -10.0);
        //glRasterPos3f((float)winWidth / 2.7, (float)winHeight / 2 , -1.0);
        glRasterPos3f((float)winWidth / 2 - 82, (float)winHeight / 2 + 82 - y, -1.0);
        for (char *c = string; *c != '\0'; c++)
        //    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN , *c);
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18 , *c);
    }
    glPopMatrix();
    reshape(winWidth, winHeight);
}

void resetGameStates()
{
    currentState = START;
    currentAngleOfRotation = 0.0;
    currentDistFromOrigin = 0.0;
    xBallVel = 0.0;
    zBallVel = -12.0;
    ballSpeed = 30.0;
    inGutter = false;
    if (remChances == 0)
    {
        pinPattern = 63;
        score = 0;
    }
    else if (pinPattern == 0)
    {
        remChances = 0;
        pinPattern = 63;
        score = 0;
    }
}

void timer(int v) {
    if (currentState != ROLLING)
        return;
    currentAngleOfRotation += 1.0;
    if (abs(zBallVel * ballSpeed * currentDistFromOrigin) < alleyLength)
        currentDistFromOrigin += 0.001;
    else if (gameOverCounter < 20)
    {
        //Naive Collision Detection
        if (gameOverCounter == 0)
        {
            if (!inGutter)
            {
                //float ballXPos = xBallVel * ballSpeed * currentDistFromOrigin;
                float ballXPos = -1 * (xBallVel * alleyLength / zBallVel);
                float ballRadius = 0.5;
                float pinStart = alleyLeft;
                float pinEnd = pinStart + pinDia;

                for (int i = 0; i < 6; i++)
                {
                    if (((ballXPos + ballRadius) >= pinStart)
                            && ((ballXPos - ballRadius) <= pinEnd))
                    {
                        pinPattern &= (63 - (1 << i));
                    }
                    //HardCode
                    pinStart += 1.04;
                    pinEnd = pinStart + pinDia;
                }
            }

        }
        gameOverCounter ++;
    }
    else {
        if (remChances < 3 && pinPattern > 0)
        {
            currentState = OVER;
            remChances = (++remChances) % 4;
        }
        else
        {
            score = 0;
            currentState = OVER;
            remChances = 0;
        }
        gameOverCounter = 0;
        glutPostRedisplay();
        return;
    }


    if (!inGutter && abs(xBallVel * ballSpeed * currentDistFromOrigin) >= alleyWidth/2)
        inGutter = true;


    if (currentAngleOfRotation > 360.0)
            currentAngleOfRotation -= 360.0;
    glutPostRedisplay();
    glutTimerFunc(1000/FPS, timer, v);
}

void drawSinglePin()
{
    //GLfloat selectedColor[] = {1 , 0, 0, 1};
    glPushMatrix();
    {
        glColor3f(0.0, 0.5, 0.5);
        //glMaterialfv(GL_FRONT, GL_DIFFUSE, selectedColor);
        glTranslatef(0.0 , 0.0, -1 * alleyLength);
        glRotatef(-90.0, 1.0, 0.0, 0.0);
        glutSolidCone(0.4, 2.0, 100.0, 100.0);
        //glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    }
    glPopMatrix();
}

void drawPins()
{
    //pattern is a number between 0 and 2^6 - 1 representing which pins to display
    //for eg 5 = 000101 would mean show pins 4 and 6 and not others
    //63 = 111111 means show all
    glPushMatrix();
    {
        //HardCode
        glTranslatef(-2.6, 0.0, 0.0);
        int pattern = pinPattern;
        while (pattern != 0)
        {
            if ((pattern % 2) == 1)
                glCallList(pin);
            glTranslatef(1.04 , 0.0, 0.0);
            pattern = pattern >> 1;
        }
    }
    glPopMatrix();
}

void drawWallsAndFloor()
{
    GLfloat currentColor[] = {1, 1, 1, 1};
    GLfloat alleyColor[] = {0.3, 0.3, 0.7, 1};
    glEnable( GL_TEXTURE_2D );
    //glBindTexture( GL_TEXTURE_2D, loadTexture("./textures/brick.bmp") );
    loadTexture("./textures/brick.bmp");
    glPushMatrix();
    {
        glTranslatef(0.0, 0.0, -1 * alleyLength);
        //Far Wall
        glBegin( GL_QUADS );
            glTexCoord2d(0.0,0.0); glVertex3f(alleyLeft - gutterWidth, 0.0, 0.0);//alleyLength);
            glTexCoord2d(1.0,0.0); glVertex3f(alleyLeft + alleyWidth + gutterWidth, 0.0, 0.0);//alleyLength);
            glTexCoord2d(1.0,1.0); glVertex3f(alleyLeft + alleyWidth + gutterWidth, 8.0, 0.0);//alleyLength);
            glTexCoord2d(0.0,1.0); glVertex3f(alleyLeft - gutterWidth, 8.0, 0.0);//alleyLength);
        glEnd();

        //LeftWall
        glBegin( GL_QUADS );
        glTexCoord2d(0.0,0.0); glVertex3f(alleyLeft - gutterWidth, 0.0, 0.0);
        glTexCoord2d(1.0,0.0); glVertex3f(alleyLeft - gutterWidth, 0.0, alleyLength);
        glTexCoord2d(1.0,1.0); glVertex3f(alleyLeft - gutterWidth, 8.0, alleyLength);
        glTexCoord2d(0.0,1.0); glVertex3f(alleyLeft - gutterWidth, 8.0, 0.0);
        glEnd();

        //RightWall
        glBegin( GL_QUADS );
        glTexCoord2d(0.0,0.0); glVertex3f(alleyLeft + alleyWidth + gutterWidth, 0.0, 0.0);
        glTexCoord2d(1.0,0.0); glVertex3f(alleyLeft + alleyWidth + gutterWidth, 0.0, alleyLength);
        glTexCoord2d(1.0,1.0); glVertex3f(alleyLeft + alleyWidth + gutterWidth, 8.0, alleyLength);
        glTexCoord2d(0.0,1.0); glVertex3f(alleyLeft + alleyWidth + gutterWidth, 8.0, 0.0);
        glEnd();

        glBegin( GL_QUADS );
        glTexCoord2d(0.0,0.0); glVertex3f(alleyLeft - gutterWidth, 8.0, 0.0);
        glTexCoord2d(1.0,0.0); glVertex3f(alleyLeft + alleyWidth + gutterWidth, 8.0, 0.0);
        glTexCoord2d(1.0,1.0); glVertex3f(alleyLeft + alleyWidth + gutterWidth, 8.0, alleyLength);
        glTexCoord2d(0.0,1.0); glVertex3f(alleyLeft - gutterWidth, 8.0, alleyLength);
        glEnd();
    }
    glPopMatrix();

    loadTexture("./textures/wood.bmp");
    glPushMatrix();
    {
        glBegin( GL_QUADS );
            glTexCoord2d(0.0,0.0); glVertex3f(alleyLeft, 0.0, 0.0);
            glTexCoord2d(1.0,0.0); glVertex3f(alleyLeft + alleyWidth, 0.0, 0.0);
            glTexCoord2d(1.0,1.0); glVertex3f(alleyLeft + alleyWidth, 0.0, -1 * (alleyLength));
            glTexCoord2d(0.0,1.0); glVertex3f(alleyLeft, 0.0, -1 * (alleyLength));
        glEnd();
    }
    glPopMatrix();

    glDisable( GL_TEXTURE_2D );

    //glColor3f (0.3, 0.3, 0.7);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, alleyColor);
    glBegin(GL_POLYGON);
        glVertex3f(alleyLeft - gutterWidth, 0.0, 0.0);
        glVertex3f(alleyLeft, 0.0, 0.0);
        glVertex3f(alleyLeft, 0.0, -1 * (alleyLength));
        glVertex3f(alleyLeft - gutterWidth, 0.0, -1 * (alleyLength));
    glEnd();
    glBegin(GL_POLYGON);
        glVertex3f(alleyLeft + alleyWidth, 0.0, 0.0);
        glVertex3f(alleyLeft + alleyWidth + gutterWidth, 0.0, 0.0);
        glVertex3f(alleyLeft + alleyWidth + gutterWidth, 0.0, -1 * (alleyLength));
        glVertex3f(alleyLeft + alleyWidth, 0.0, -1 * (alleyLength));
    glEnd();
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, currentColor);
}

void initDisplayLists()
{
    wallsAndFloor = glGenLists (3);
    glNewList(wallsAndFloor, GL_COMPILE);
        drawWallsAndFloor();
    glEndList();
    pin = wallsAndFloor + 1;
    glNewList(pin, GL_COMPILE);
        drawSinglePin();
    glEndList();
    sphere = wallsAndFloor + 2;
    glNewList(sphere, GL_COMPILE);
        glutSolidSphere(0.5, 20, 20);
    glEndList();

}

void setupRC()
{
    //tbInit(GLUT_RIGHT_BUTTON);
    //tbAnimate(GL_TRUE);
    
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
    initDisplayLists();

}

void setCamera( void )
{
    /*glTranslatef(0, 0, camPosZ);
    glRotatef(camRotX, 1, 0, 0);
    glRotatef(camRotY, 0, 1, 0);*/
    glTranslatef(0.0, -2.0, -1.0);
    //gluLookAt(0.0, 2.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void drawSelectableTeapots( void )
{
    float currentColor[4];
    glGetFloatv(GL_CURRENT_COLOR, currentColor);
    GLfloat selectedColor[] = {0, 1, 0, 1};
    GLfloat unselectedColor[] = {1, 0, 0, 1};
    // Initialize the name stack
    glInitNames();
    glPushName(0);
    
    // Draw two teapots next to each other in z axis
    glPushMatrix();
    {

        if( isTeapot1_selected )
            glMaterialfv(GL_FRONT, GL_DIFFUSE, selectedColor);
        else
            glMaterialfv(GL_FRONT, GL_DIFFUSE, unselectedColor);
        glLoadName(0);
        glutSolidTeapot(2.5);

        if( isTeapot2_selected )
            glMaterialfv(GL_FRONT, GL_DIFFUSE, selectedColor);
        else
            glMaterialfv(GL_FRONT, GL_DIFFUSE, unselectedColor);
        glLoadName(1);
        glTranslatef(0,0,5);
        glutSolidTeapot(1.5);
    }
    glPopMatrix();
    glColor4fv(currentColor);
}

void display(void)
{
    glLoadIdentity ();             /* clear the matrix */
    setCamera();
    GLfloat ballColor[] = {1, 0, 1, 0.5};
    GLfloat pinColor[] = {0.5, 0.5, 0.5, 1};
    GLfloat arrowColor[] = {1, 0, 0, 1};
    GLfloat currentColor[] = {1,1,1,1};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, currentColor);

    //TODO : LIGHTS
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glClear(GL_COLOR_BUFFER_BIT);
    glCallList(wallsAndFloor);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pinColor);
    drawPins();
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, currentColor);

    if (currentState == START)
    {
        //glEnable(GL_LIGHTING);
        //glColor3f (1.0, 0.0, 0.0);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, arrowColor);
        glLineWidth (5.0);
        GLfloat tipX = xBallVel * ballSpeed / 30;
        GLfloat tipZ = zBallVel * ballSpeed / 30;
        GLfloat tipXDx = ballSpeed / 100.0;
        GLfloat tipZDz = 0;
        if (tipX > 0)
            tipZDz = 0.2;
        else if (tipX < 0)
            tipZDz = -0.2;
        glBegin(GL_LINE_STRIP);
            glVertex3f(0.0, 0.0, 0.0);
            glVertex3f(tipX, 0.0, tipZ);
        glEnd();
        glBegin(GL_POLYGON);
            glVertex3f(tipX - tipXDx, 0.0, tipZ + 1);
            glVertex3f(tipX + tipXDx, 0.0, tipZ + 1 + tipZDz);
            glVertex3f(tipX, 0.0, tipZ);
        glEnd();
        //glColor3f (1.0, 1.0, 1.0);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, currentColor);
        renderText(0.0, 0.0, 0.0, "Use Arrow Keys for");
        renderText(0.0, 30.0, 0.0, "speed and direction");
        renderText(0.0, 70.0, 0.0, "Hit SPACE to roll");

    }
    else if (currentState == ROLLING)
    //if (currentState == ROLLING)
    {
        //glGetFloatv(GL_CURRENT_COLOR, currentColor);
        //glPushAttrib(GL_CURRENT_COLOR);
        //glDisable(GL_LIGHTING);
        glPushMatrix();
        {
            glColor3f (1.0, 1.0, 0.0);
            if (!inGutter)
                glTranslatef(xBallVel * ballSpeed * currentDistFromOrigin, 0.0,
                             zBallVel * ballSpeed * (currentDistFromOrigin));
            else if (xBallVel > 0)
                glTranslatef(alleyLeft + alleyWidth + gutterWidth/2, 0.0,
                             zBallVel * ballSpeed * (currentDistFromOrigin));
            else
                glTranslatef(alleyLeft - gutterWidth/2, 0.0,
                             zBallVel * ballSpeed * (currentDistFromOrigin));
            glRotatef(currentAngleOfRotation * 5, 1.0, 0.0, 0.0);
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ballColor);
            //glutSolidSphere(0.5, 20, 20);
            glCallList(sphere);
        }
        glPopMatrix();
        //glColor3f (1.0, 1.0, 1.0);
        //glPopAttrib();
        //glColor4fv(currentColor);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, currentColor);
        if (inGutter)
            renderText(0.0, 0.0, 0.0, "IN GUTTER!!");
    }
    else
    {
        //glEnable(GL_LIGHTING);
        glColor3f (1.0, 1.0, 1.0);
        if (remChances > 0)
        {
            char chances = 52 - remChances;
            std::string text = "Chances Left: ";
            text.append(1, chances);
            renderText(250.0, 0.0, 0.0, &text[0]);
            renderText(250.0, 30.0, 0.0, "Hit SPACE to resume");
        }
        else if (score >= 0)
        {
            score = (score < 0) ? 0 : score;
            for (short tmp = pinPattern; tmp != 0; tmp = tmp >> 1)
                if (tmp % 2 == 1)
                    score -= 20;
            score += 120;
            renderText(255.0, 0.0, 0.0, "GAME OVER!!!");
            std::string text = "YOUR SCORE: ";
            std::ostringstream str;
            str<<score;
            text += str.str();
            renderText(0.0, 30.0, 0.0, &text[0]);
            renderText(0.0, 60.0, 0.0, "Hit SPACE to restart");
        }
    }
    //glColor4fv(currentColor);
    glFlush ();
    glutSwapBuffers();
}

void specialKeys( int key, int x, int y )
{
    switch(key)
    {
        case GLUT_KEY_UP:
            if (currentState == START)
            {
                ballSpeed += 0.5;
                glutPostRedisplay();
            }
            break;
        case GLUT_KEY_DOWN:
            if (currentState == START)
            {
                ballSpeed -= 0.5;
                glutPostRedisplay();
            }
            break;
        case GLUT_KEY_LEFT:
            if (currentState == START)
            {
                xBallVel -= 0.2;
                glutPostRedisplay();
            }
            break;
        case GLUT_KEY_RIGHT:
            if (currentState == START)
            {
                xBallVel += 0.2;
                glutPostRedisplay();
            }
            break;
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
        if (currentState == START)
        {
            currentState = ROLLING;
            glutTimerFunc(1000/FPS, timer, 0);
        }
        else if (currentState == OVER)
        {
            resetGameStates();
            glutPostRedisplay();
        }
        break;
    }
}

/*void processSelection(int xPos, int yPos)
{
    GLfloat fAspect;
    
    // Space for selection buffer
    static GLuint selectBuff[BUFFER_LENGTH];
    
    // Hit counter and viewport storage
    GLint hits, viewport[4];
    
    // Setup selection buffer
    glSelectBuffer(BUFFER_LENGTH, selectBuff);
    
    // Get the viewport
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    // Switch to projection and save the matrix
    glMatrixMode(GL_PROJECTION);
    
    glPushMatrix();
    {
        // Change render mode
        glRenderMode(GL_SELECT);
        
        // Establish new clipping volume to be unit cube around
        // mouse cursor point (xPos, yPos) and extending two pixels
        // in the vertical and horizontal direction
        glLoadIdentity();
        gluPickMatrix(xPos, viewport[3] - yPos + viewport[1], 0.1,0.1, viewport);
        
        // Apply perspective matrix
        fAspect = (float)viewport[2] / (float)viewport[3];
        gluPerspective(45.0f, fAspect, 1.0, 425.0);
        
        
        // Render only those needed for selection
        glPushMatrix();
        {
            setCamera();
            tbMatrixForSelection();
            
            drawSelectableTeapots();
        }
        glPopMatrix();
        
        
        // Collect the hits
        hits = glRenderMode(GL_RENDER);
        
        isTeapot1_selected = false;
        isTeapot2_selected = false;
        
        // If hit(s) occurred, display the info.
        if(hits != 0)
        {

            // Save current picked object.
            // Take only the nearest selection
            pickedObj = selectBuff[3];
            
            sprintf (titleString, "You clicked on %d", pickedObj);
            glutSetWindowTitle(titleString);
            
            if (pickedObj == 0) {
                isTeapot1_selected = true;
            }
            
            if (pickedObj == 1) {
                isTeapot2_selected = true;
            }
            
        }
        else
            glutSetWindowTitle("Nothing was clicked on!");
        
        
        // Restore the projection matrix
        glMatrixMode(GL_PROJECTION);
    }
    glPopMatrix();
    
    // Go back to modelview for normal rendering
    glMatrixMode(GL_MODELVIEW);
    
    glutPostRedisplay();
}

void mouse( int button, int state, int x, int y)
{
    tbMouse(button, state, x, y);
    
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        processSelection(x, y);
    
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

}*/

int main (int argc, char *argv[])
{
    int win_width = 720;
    int win_height = 640;

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize( win_width, win_height );

    glutCreateWindow( "Bowling Alley" );
    setupRC();

    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutSpecialFunc( specialKeys );
    //glutMouseFunc( mouse );
    //glutMotionFunc( motion );

    glutMainLoop();
}

