#include <stdlib.h>
#include <stdio.h>

#include <GL/glew.h>
#if defined(_WIN32)
#   include <GL/wglew.h>
#endif

#if defined(__APPLE__) || defined(MACOSX)
#   include <GLUT/glut.h>
#else
#   include <GL/glut.h>
#endif

#include "GLScreenCapturer.h"
#include "trackball.h"
#include "shader.h"
#include "GLSLProgram.h"

using namespace std;

#define BUFFER_LENGTH 64

GLfloat camRotX, camRotY, camPosX, camPosY, camPosZ;
GLint viewport[4];
GLdouble modelview[16];
GLdouble projection[16];

GLuint pickedObj = -1;
char titleString[150];

bool isTeapot1_selected = false;
bool isTeapot2_selected = false;

// Lights & Materials
GLfloat ambient[] = {0.2, 0.2, 0.2, 1.0};
GLfloat position[] = {0.0, 0.0, 2.0, 1.0};

GLfloat ambient1[] = {0.2, 0.2, 0.2, 1.0};
GLfloat position1[] = {0.0, 10.0, 2.0, 1.0};

GLfloat ambient2[] = {0.2, 0.2, 0.2, 1.0};
GLfloat position2[] = {-30.0, 1.0, 2.0, 1.0};

GLfloat mat_diffuse[] = {0.6, 0.6, 0.6, 1.0};
GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat mat_shininess[] = {50.0};

static GLScreenCapturer screenshot("screenshot-%d.ppm");
static GLSLProgram*     shaderProg = NULL;
int numLights = 1;

float barycentricArray[] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
GLfloat alpha[] = {1.0f, 0.0f, 0.0f};
GLfloat beta[] = {0.0f, 1.0f, 0.0f};
GLfloat gamma[] = {0.0f, 0.0f, 1.0f};
GLint loc;
GLuint vao;
GLuint tex;

void initLights(int numLights)
{
    shaderProg->enable();
    GLint loc = glGetUniformLocation(shaderProg->prog_, "numLights");
    glUniform1i(loc, numLights);
    shaderProg->disable();
}


void initLights(void)
{
    glEnable(GL_LIGHTING);
    	glEnable(GL_LIGHT0);
    	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    	glLightfv(GL_LIGHT0, GL_POSITION, position);
        glEnable(GL_LIGHT1);
        glLightfv(GL_LIGHT1, GL_AMBIENT, ambient1);
        glLightfv(GL_LIGHT1, GL_POSITION, position1);
        glEnable(GL_LIGHT2);
        glLightfv(GL_LIGHT2, GL_AMBIENT, ambient2);
        glLightfv(GL_LIGHT2, GL_POSITION, position2);
    shaderProg->enable();
    GLint loc = glGetUniformLocation(shaderProg->prog_, "numLights");
    glUniform1i(loc, 1);
    shaderProg->disable();
    
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void setupRC()
{
    tbInit(GLUT_RIGHT_BUTTON);
    tbAnimate(GL_TRUE);
    
    // Place Camera
    camRotX = 350.0f;
    camRotY = 680.0f;
    camPosX = 0.0f;
    camPosY = 0.0f;
    camPosZ = -10.5f;
    
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    initLights();
}

void setCamera( void )
{
    glTranslatef(0, 0, camPosZ);
    glRotatef(camRotX, 1, 0, 0);
    glRotatef(camRotY, 0, 1, 0);
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
    
    shaderProg->enable();
    //glBindVertexArray(vao);
    //shaderProg->set_uniform_3f("lightDir", 2.f, -1.f, -0.3f);
    // Draw two teapots next to each other in z axis
    glPushMatrix();
    {
//	glBegin(GL_TRIANGLES);//start drawing triangles
//	  glVertexAttrib3fv(loc,alpha); glVertex3f(-1.0f,-0.25f,0.0f);//triangle one first vertex
//      	  glVertexAttrib3fv(loc,beta); glVertex3f(-0.5f,-0.25f,0.0f);//triangle one second vertex
//	  glVertexAttrib3fv(loc,gamma); glVertex3f(-0.75f,0.25f,0.0f);//triangle one third vertex
      //drawing a new triangle
//          glVertexAttrib3fv(loc,alpha); glVertex3f(0.5f,-0.25f,0.0f);//triangle two first vertex
//          glVertexAttrib3fv(loc,beta); glVertex3f(1.0f,-0.25f,0.0f);//triangle two second vertex
//          glVertexAttrib3fv(loc,gamma); glVertex3f(0.75f,0.25f,0.0f);//triangle two third vertex
//        glEnd();//end drawing of triangles
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
    shaderProg->disable();
    
    glColor4fv(currentColor);

}

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glPushMatrix();
    {
    
        setCamera();
        tbMatrix();
        
        drawSelectableTeapots();
        
        // Retrieve current matrice before they popped.
        glGetDoublev( GL_MODELVIEW_MATRIX, modelview );        // Retrieve The Modelview Matrix
        glGetDoublev( GL_PROJECTION_MATRIX, projection );    // Retrieve The Projection Matrix
        glGetIntegerv( GL_VIEWPORT, viewport );                // Retrieves The Viewport Values (X, Y, Width, Height)
    }
    glPopMatrix();

    glFlush();
    // End Drawing calls
    glutSwapBuffers();
}

void reshape( int w, int h )
{
    tbReshape(w, h);

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Set the clipping volume
    gluPerspective(45.0f, (GLfloat)w / (GLfloat)h, 1.0f, 100.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

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
    case '1':
	if (shaderProg != NULL)
	    delete shaderProg;
	shaderProg = new GLSLProgram(gouraudVS, gouraudFS);
	initLights(numLights);
    glutSetWindowTitle("Gouraud with Phong");
        glutPostRedisplay();
        break;
    case '2':
	if (shaderProg != NULL)
            delete shaderProg;
        shaderProg = new GLSLProgram(blinnPhongVS, blinnPhongFS);
	initLights(numLights);
    glutSetWindowTitle("Blinn-Phong");
        glutPostRedisplay();
        break;
    case '3':
	if (shaderProg != NULL)
            delete shaderProg;
        shaderProg = new GLSLProgram(checkerVS, checkerFS);
	initLights(numLights);
    glutSetWindowTitle("Checkerboard");
        glutPostRedisplay();
        break;
    case 'q':
        numLights = 1;
	initLights(numLights);
        glutPostRedisplay();
        break;
    case 'w':
	numLights = 2;
	initLights(numLights);
        glutPostRedisplay();
        break;
    case 'e':
	numLights = 3;
	initLights(numLights);
        glutPostRedisplay();
        break;
    }
}

void processSelection(int xPos, int yPos)
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

}

static void setupShaders()
{
    printf("MSG: Initialize GLSL Shaders ...\n");

    glewInit();
    if ( !glewIsSupported("GL_VERSION_2_0 GL_ARB_multitexture GL_EXT_framebuffer_object") ) 
    {
        fprintf(stderr, "Required OpenGL extensions missing\n");
        exit(2);
    }
    shaderProg = new GLSLProgram(gouraudVS, gouraudFS);
    //shaderProg = new GLSLProgram(blinnPhongVS, gouraudFS);
    //shaderProg = new GLSLProgram(checkerVS, checkerFS);
    //glGenTextures(1, &tex);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //float pixels[] = { 0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
//		       1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f };
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels);
    //shaderProg->bind_texture("tex", tex, GL_TEXTURE_2D, 0);
    //GLuint vao;
    //glGenVertexArrays(1, &vao);
    //GLuint buffer;
    //GLint attribLoc;
    //glGenBuffers(1, &buffer);
    //loc = glGetAttribLocation(shaderProg->prog_, "vertexBCCoord");
    
    //glBindBuffer(GL_ARRAY_BUFFER, buffer);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(barycentricArray), barycentricArray, GL_STATIC_DRAW);
    //glEnableClientState(GL_VERTEX_ARRAY);
 
    //glEnableVertexAttribArray(loc);
 
    //glVertexAttribPointer(loc , 3, GL_FLOAT, 0, 0, barycentricArray);
    //glBindVertexArray(vao);
}

int main (int argc, char *argv[])
{
    int win_width = 512;
    int win_height = 512;

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize( win_width, win_height );

    glutCreateWindow( "Opengl demo" );
    setupShaders();
    setupRC();

    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutMouseFunc( mouse );
    glutMotionFunc( motion );

    glutMainLoop();
    delete shaderProg;
}
