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
#include "raytracer.h"

#include <ctime> //timer

using namespace std;

namespace raytracer
{
    ////////global variables for OpenGL//////
    int window_id, width, height;
    bool myDraw;
    AmModelPtr model;
    AmCameraPtr camera;
    vector<AmLightPtr> lights;
    AmRayTracerPtr rayTracer;
    AmUintPtr pixels;
    
    void idle(void);
    void display(void);
    void keyboard(unsigned char key, int x, int y);
    void reshape(int w, int h);
    
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
        
        width = mWidth;
        height = mHeight;
        myDraw = true;
        
        AmVec3f eye(0,0,2);
        AmVec3f center(0,0,0);
        AmVec3f up(0,1,0);
        camera = AmCameraPtr(new AmCamera(mWidth, mHeight, eye, center, up));
        
        
        float light_position[] = { 1.0, 0.0, 0.0, 0.0 };
        float light_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
        lights.push_back(AmLightPtr(new AmLight(
                            AmLight::AM_POSITION,
                            AmLight::AM_LIGHT0, light_position)));
        lights.push_back(AmLightPtr(new AmLight(
                            AmLight::AM_AMBIENT,
                            AmLight::AM_LIGHT1, light_ambient)));
        
        rayTracer = AmRayTracerPtr(new AmRayTracer());
        rayTracer->setCamera(camera);
        rayTracer->setLight(lights);
        
        pixels = AmUintPtr(new unsigned int[width * height]);
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
        glutReshapeFunc(reshape);
        
		glutMainLoop();
	}
    
    
    void MyOpengl::setModel(const AmModelPtr &m){
        model = m;
        rayTracer->setModel(m);
    }
    
    void PrintString(void *font, const string str)
    {
        for (int i = 0; i < str.size(); i++)
            glutBitmapCharacter(font, str[i]);
    }
    
    void raytracerDraw()
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        rayTracer->render(pixels);
        
        glWindowPos2i(0, 0);
        glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());
        
    }
    
    void openglDraw()
    {
        // set the camera
        gluLookAt(camera->eye.x(), camera->eye.y(), camera->eye.z(),
                  camera->center.x(), camera->center.y(), camera->center.z(),
                  camera->up.x(), camera->up.y(), camera->up.z());
        
        // enable the light
        glEnable(GL_LIGHTING);
        for (int i = 0; i < lights.size(); i++) {
            glLightfv(lights[i]->name, lights[i]->type, lights[i]->value);
            glEnable(lights[i]->name);
        }
        
        glEnable(GL_DEPTH_TEST);
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
    
    
    /////// OpenGL reliable functions /////////////
    
    void idle(void)
    {
        glutPostRedisplay();
    }
    
    void reshape(int w, int h)
    {
        width = w;
        height = h;
        camera->width = w;
        camera->height = h;
        camera->update();
        
        pixels = AmUintPtr(new unsigned int[w*h]);
        
    }
    
    void display(void)
    {
        glClearColor(0.2, 0.2, 0.2, 1.0); //black is not comfortable
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glViewport(0, 0, (GLsizei)width, (GLsizei)height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if (width <= height)
            glOrtho(-1, 1, -1 * (GLfloat)height/(GLfloat)width,
                    1 * (GLfloat)height/(GLfloat)width, -10.0, 10.0);
        else
            glOrtho(-1*(GLfloat)width/(GLfloat)height,
                    1*(GLfloat)width/(GLfloat)height,
                    -1, 1, -10.0, 10.0);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // start the timer
        clock_t startTime = clock();
        
        if (myDraw) {
            // use the raytracer functions of the model
            raytracerDraw();
        } else {
            // call the openGL functions to draw the scene
            openglDraw();
        }
        
        // stop the timer
        clock_t endTime = clock();
        //show the fps
        double fps = 1.0 * CLOCKS_PER_SEC / (endTime - startTime);
        ostringstream oss;
        oss.precision(10);
        oss<<"fps: "<<fps;
        glDisable(GL_COLOR_MATERIAL);
        glColor3f(1.f, 1.f, 1.f);
        if (width <= height)
            glRasterPos2f(-0.9, -0.9*(GLfloat)height/(GLfloat)width);
        else
            glRasterPos2f(-0.9*(GLfloat)width/(GLfloat)height, -0.9);
        PrintString(GLUT_BITMAP_HELVETICA_18, oss.str());
        
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
            glutPostRedisplay();
        }
        
        float step = 0.1;
//        float angle = 20.0 * M_PI / 180.0;
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

