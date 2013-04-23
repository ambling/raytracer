//
//  utils.h
//  raytracer
//
//  Created by ambling on 13-4-23.
//  Copyright (c) 2013年 ambling. All rights reserved.
//

#ifndef raytracer_utils_h
#define raytracer_utils_h

#include <string>
#include <vector>

using namespace std;

namespace raytracer {

    static const float M_PI = 3.1415926f;
    
    /*
     * MyVec3f: 3 floats vector
     */
    class AmVec3f {
        
        float mData[3];
        
    public:
        AmVec3f(float x, float y, float z):
            mData({x, y, z})
        {}
        
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
}


#endif
