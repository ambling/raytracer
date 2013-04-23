//
//  gl.h
//  raytracer
//
//  Created by ambling on 13-4-21.
//  Copyright (c) 2013年 ambling. All rights reserved.
//

#ifndef raytracer_gl_h
#define raytracer_gl_h

#include <iostream>

#if defined(__APPLE__) || defined(MACOSX)
#include <GLUT/GLUT.h>
#else
#ifdef WIN32
#include <glut.h>
#else
#include <GL/glut.h>
#endif
#endif

using namespace std;

namespace raytracer
{
	class MyOpengl
	{
	public:
		MyOpengl(int argc, char** argv);
		MyOpengl(int, char**, int , int );
		~MyOpengl();
        
		void init();
        
	private:
		int mWidth;
		int mHeight;
        int argc;
        char **argv;
        
	};
    
}

#endif
