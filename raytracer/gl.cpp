//
//  gl.cpp
//  raytracer
//
//  Created by ambling on 13-4-21.
//  Copyright (c) 2013年 ambling. All rights reserved.
//

#include "gl.h"
#include "utils.h"
#include "model.h"

using namespace std;

namespace raytracer
{
    ////////global variables for OpenGL//////
    int window_id;
    bool myDraw;
    AmModelPtr model;
    AmCameraPtr camera;
    
    void idle(void);
    void display(void);
    void keyboard(unsigned char key, int x, int y);
    
	MyOpengl::MyOpengl( int argc,  char** argv) :
        mWidth(800), mHeight(600)
	{
        setVal(argc, argv);
    }
    
	MyOpengl::MyOpengl( int argc,  char** argv, int w, int h) :
        mWidth(w), mHeight(h)
	{
        setVal(argc, argv);
		cout <<"window size: " << w << " x " << h << endl;
	}
    MyOpengl::~MyOpengl()
    {}
    
    void MyOpengl::setVal(int argc,  char** argv)
    {
        this->argc = argc;
        this->argv = argv;
        myDraw = false;
        
        AmVec3f eye(0,0,0);
        AmVec3f center(0,0,-1);
        AmVec3f up(0,1,0);
        camera = AmCameraPtr(new AmCamera(eye, center, up));
    }
    
	void MyOpengl::init()
	{
		glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
        
		glutInit(&argc, argv);
		glutInitWindowPosition(0, 0);
		glutInitWindowSize(mWidth, mHeight);
        
		window_id = glutCreateWindow("raytracer By Ambling");
		glutDisplayFunc(display);
		glutKeyboardFunc(keyboard);
		glutIdleFunc(idle);
        
		glutMainLoop();
	}
    
    
    void MyOpengl::setModel(const AmModelPtr &m){
        model = m;
    }
    
    
    /////// OpenGL reliable functions /////////////
    
    void openglDraw()
    {
        glEnable(GL_COLOR_MATERIAL);
        
        for (int group = 0; group < model->mGroups.size(); group++) {
            int material = model->mGroups[group].material;
            if (material != 0) {
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,
                             model->mMaterials[material].ambient);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,
                             model->mMaterials[material].diffuse);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,
                             model->mMaterials[material].specular);
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS,
                            model->mMaterials[material].shininess);
                //glColor3fv(model->mMaterials[material].diffuse);
            }
            
            glBegin(GL_TRIANGLES);
            for (int i = 0; i < model->mGroups[group].triangles.size(); i++) {
                AmTriangle* triangle
                    = &model->mTriangles[model->mGroups[group].triangles[i]];
                
                glNormal3fv(model->mNormals[triangle->nindices[0]].data());
                glTexCoord2fv(model->mTexcoords[triangle->tindices[0]].data());
                glVertex3fv(model->mVertices[triangle->vindices[0]].data());
                
                glNormal3fv(model->mNormals[triangle->nindices[1]].data());
                glTexCoord2fv(model->mTexcoords[triangle->tindices[1]].data());
                glVertex3fv(model->mVertices[triangle->vindices[1]].data());
                
                glNormal3fv(model->mNormals[triangle->nindices[2]].data());
                glTexCoord2fv(model->mTexcoords[triangle->tindices[2]].data());
                glVertex3fv(model->mVertices[triangle->vindices[2]].data());
            }
            glEnd();
        }
    }
    
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
        
        //glOrtho(-width / 2.0, width / 2.0, -height / 2.0, height / 2.0, -10, 10000);
        glFrustum(-width / 2.0, width / 2.0, -height / 2.0, height / 2.0, 10.0,
                  -200.0);
        
        // set the camera
        gluLookAt(camera->eye.x(), camera->eye.y(), camera->eye.z(),
                  camera->center.x(), camera->center.y(), camera->center.z(),
                  camera->up.x(), camera->up.y(), camera->up.z());
        
        // enable the light
        //glEnable ( GL_LIGHTING ) ;
        
        if (myDraw) {
            // use the raytracer functions of the model
        } else {
            // call the openGL functions to draw the scene
            
            openglDraw();
        }
        
        //glMatrixMode( GL_MODELVIEW);
        glLoadIdentity();
        
        glutSwapBuffers();
    }
    
    void keyboard(unsigned char key, int x, int y)
    {
        if (key == 27 || key == 'q') 
        {// the escape key or 'q' to quit
            glutDestroyWindow(window_id);
            exit(0);
        }
        
        if (key == ' ')
        {// space key to switch the render function (raytracer or OpenGL)
            myDraw = !myDraw;
        }
        
        float step = 1;
        float angle = 20.0 * M_PI / 180.0;
        if (key == 'w')
        {// move forward
            AmVec3f move = camera->dir * step;
            camera->eye = camera->eye + move;
            camera->update();
        }
        
        if (key == 's')
        {// move backward
            AmVec3f move = camera->dir * step;
            camera->eye = camera->eye - move;
            camera->update();
        }
        
        if (key == 'a')
        {// move left
        }
        
        if (key == 'd')
        {// move right
        }
    }
    
}

