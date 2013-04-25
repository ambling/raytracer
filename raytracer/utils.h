//
//  utils.h
//  raytracer
//
//  Created by ambling on 13-4-23.
//  Copyright (c) 2013年 ambling. All rights reserved.
//

#ifndef raytracer_utils_h
#define raytracer_utils_h

#include <iostream>
#include <string>
#include <vector>
#include <cassert>

#ifdef _LIBCPP_VERSION
#include <memory>
#else
#include <tr1/memory>
#endif

using namespace std;

namespace raytracer {

    static const float M_PI = 3.1415926f;
    
    /*
     * MyVec3f: 3 floats vector
     */
    class AmVec3f {
        
        
    public:
        float mData[3];
        AmVec3f(float x, float y, float z):
            mData{x, y, z}
        {
        }
        
        float x()
        {
            return mData[0];
        }
        
        float y()
        {
            return mData[1];
        }
        
        float z()
        {
            return mData[2];
        }
        
        void setX(float x)
        {
            mData[0] = x;
        }
        
        void setY(float y)
        {
            mData[1] = y;
        }
        
        void setZ(float z)
        {
            mData[2] = z;
        }
    };
    
    /*
     * 2 floats vector
     */
    class AmVec2f
    {
    public:
        float mData[2];
        AmVec2f(float u, float v):
            mData{u, v}
        {}
    };
    
    class CommonFuncs
    {
    public:
        static string getDirName(string path)
        {
            string::size_type pos = path.rfind('/');
            if (pos != string::npos) {
                //including the '/'
                return path.substr(0, pos+1);
            } else {
                // local dir
                return "";
            }
        }
    };
}


#endif
