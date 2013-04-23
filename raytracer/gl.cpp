//
//  gl.cpp
//  raytracer
//
//  Created by ambling on 13-4-21.
//  Copyright (c) 2013年 ambling. All rights reserved.
//

#include "gl.h"


using namespace std;



    ////////global variables and functions for OpenGL//////

int window_id;

void idle(void)
{
    glutPostRedisplay();
}

void display(void)
{
    glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    glViewport(0, 0, width, height);
    
    glMatrixMode( GL_PROJECTION);
    glLoadIdentity();
    
		// glOrtho(0, width, 0, height, -10000, 10000);
    glFrustum(-width / 2.0, width / 2.0, -height / 2.0, height / 2.0, -1.0,
              -200.0);
    
    glMatrixMode( GL_MODELVIEW);
    glLoadIdentity();
    
		//viewing matrix
        //		gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0],
        //				up[1], up[2]);
    
		//light info
        //		GLfloat light_pos[] = { 0.0, 0.0, 0.0, 0.0 };
        //		GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };
        //		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
        //		glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
        //		glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
        //		glEnable( GL_LIGHT0);
        //		glEnable( GL_LIGHTING);
    
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == 27 || key == 'q') // the escape key or 'q'
    {
        glutDestroyWindow(window_id);
        exit(0);
    }
    
    if (key == ' ') {
    }
}

namespace raytracer
{
    
	MyOpengl::MyOpengl( int argc,  char** argv) :
        argc(argc), argv(argv), mWidth(800), mHeight(600)
	{}
    
	MyOpengl::MyOpengl( int argc,  char** argv, int w, int h) :
        argc(argc), argv(argv), mWidth(w), mHeight(h)
	{
		cout <<"window size: " << w << " x " << h << endl;
	}
    MyOpengl::~MyOpengl()
    {}
    
	void MyOpengl::init()
	{
		glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
        
		glutInit(&argc, argv);
		glutInitWindowPosition(0, 0);
		glutInitWindowSize(mWidth, mHeight);
        
		window_id = glutCreateWindow("raytracer 0.1 By Ambling");
		glutDisplayFunc(display);
		glutKeyboardFunc(keyboard);
		glutIdleFunc(idle);
        
		glutMainLoop();
	}
    
}