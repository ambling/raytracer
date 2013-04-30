//
//  main.cpp
//  raytracer
//
//  Created by ambling on 13-4-21.
//  Copyright (c) 2013年 ambling. All rights reserved.
//

#include <iostream>
#include "utils.h"
#include "gl.h"
#include "model.h"

using namespace raytracer;

#define AM_RELEASE     // if not debugging, comment this out

int main(int argc, char * argv[])
{
    string path;
    if (argc >=2 ) {
        path = argv[1];
    } else {
        path = string("/Users/ambling/Documents/Dropbox/_Code/raytracer/")
                +"raytracer/obj/drawing_desk.obj";
//        path = string("/Users/ambling/Documents/Dropbox/_Code/raytracer/")
//                +"raytracer/obj/light_collection/fot01.obj";
    }
    
    
    MyOpengl mygl(argc, argv, 100, 100);
    
    AmModelPtr model(new AmModel(path));
    model->utilize();
    mygl.setModel(model);
	mygl.init();
    return 0;
}

