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
#include <cmath>
#include <cassert>

#ifdef _LIBCPP_VERSION
#include <memory>
#else
#include <tr1/memory>
#endif

using namespace std;

namespace raytracer {

    static const float M_MIN = -10000000.0f;
    static const float M_MAX = 10000000.0f;
    
    /*
     * MyVec3f: 3 floats vector
     */
    class AmVec3f {
        
        
    public:
        float mData[3];
        AmVec3f()
            :mData{0,0,0}
        {}
        
        AmVec3f(float x, float y, float z):
            mData{x, y, z}
        {
        }
        
        float* data()
        {
            return mData;
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
        
        AmVec3f operator+ (const AmVec3f &rhs)
        {
            AmVec3f re(rhs);
            re.mData[0] += mData[0];
            re.mData[1] += mData[1];
            re.mData[2] += mData[2];
            return re;
        }
        
        AmVec3f operator- (const AmVec3f &rhs)
        {
            AmVec3f re(*this);
            re.mData[0] -= rhs.mData[0];
            re.mData[1] -= rhs.mData[1];
            re.mData[2] -= rhs.mData[2];
            return re;
        }
        
        AmVec3f operator/ (const float rhs)
        {
            AmVec3f re(*this);
            re.mData[0] /= rhs;
            re.mData[1] /= rhs;
            re.mData[2] /= rhs;
            return re;
        }
        
        AmVec3f operator* (const float rhs)
        {
            AmVec3f re(*this);
            re.mData[0] *= rhs;
            re.mData[1] *= rhs;
            re.mData[2] *= rhs;
            return re;
        }
        
        void normalize()
        {
            float s = sqrt(mData[0]*mData[0]
                           + mData[1]*mData[1]
                           + mData[2]*mData[2]);
            mData[0] /= s;
            mData[1] /= s;
            mData[2] /= s;
        }
    };
    
    /*
     * 2 floats vector
     */
    class AmVec2f
    {
    public:
        float mData[2];
        AmVec2f()
            :mData{0,0}
        {}
        
        AmVec2f(float u, float v):
            mData{u, v}
        {}
        
        float* data()
        {
            return mData;
        }
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
    
    class AmCamera
    {
    public:
        AmVec3f eye;
        AmVec3f center;
        AmVec3f up;
        
        AmVec3f dir;
        
        AmCamera(const AmVec3f &e, const AmVec3f &c, const AmVec3f &u)
            :eye(e), center(c), up(u)
        {
            update();
        }
        
        void update()
        {
            dir = center - eye;
            dir.normalize();
        }
    };
    
    class AmModel;
    typedef shared_ptr<AmModel> AmModelPtr;
    typedef shared_ptr<AmCamera> AmCameraPtr;
}


#endif
