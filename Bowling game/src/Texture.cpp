#include <iostream>
#include <cstdio>
#include <stdio.h>
#include "Texture.h"

using namespace std;

GLuint loadTexture(const char * imagepath)
{

    unsigned char header[54];
    unsigned int dataPos;
    unsigned int width, height;
    unsigned int imageSize;
    unsigned char * data;

    FILE * file = fopen(imagepath,"rb");
    if (!file)
    {
        cout<<"Image could not be opened\n";
        return -1;
    }
    if ( fread(header, 1, 54, file)!=54 )
    {
        cout<<"Not a correct BMP file\n";
        return -1;
    }
    if ( header[0]!='B' || header[1]!='M' )
    {
        cout<<"Not a correct BMP file\n";
        return -1;
    }
    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    width      = *(int*)&(header[0x12]);
    height     = *(int*)&(header[0x16]);
    if (imageSize==0)
        imageSize=width*height*3;
    if (dataPos==0)
        dataPos=54;
    // Create a buffer
    data = new unsigned char [imageSize];

    // Read the actual data from the file into the buffer
    fread(data,1,imageSize,file);

    //Everything is in memory now, the file can be closed
    fclose(file);

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);


    // select modulate to mix texture with color for shading
    //glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );


    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the original
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    // the texture wraps over at the edges (repeat)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height,
                       GL_BGR, GL_UNSIGNED_BYTE, data );
    // Give the image to OpenGL
    //glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    delete[] data;
    return textureID;
}
