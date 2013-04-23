//
//  main.cpp
//  raytracer
//
//  Created by ambling on 13-4-21.
//  Copyright (c) 2013年 ambling. All rights reserved.
//

#include <iostream>
#include "gl.h"

using namespace raytracer;

int main(int argc, char * argv[])
{

    MyOpengl mygl(argc, argv);
	mygl.init();
    return 0;
}

