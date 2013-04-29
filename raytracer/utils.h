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
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>

#ifdef _LIBCPP_VERSION
#include <memory>
#else
#include <tr1/memory>
#endif

using namespace std;

namespace raytracer {

    static const float M_MIN = -10000000.0;
    static const float M_MAX = 10000000.0;
    static const float EPSILON = 0.0001;
    
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
        
        float x() const
        {
            return mData[0];
        }
        
        float y() const
        {
            return mData[1];
        }
        
        float z() const
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
        
        bool orVal(const float rhs) const
        {
            return (mData[0] == rhs || mData[1] == rhs || mData[2] == rhs);
        }
        
        bool andVal(const float rhs) const
        {
            return (mData[0] == rhs && mData[1] == rhs && mData[2] == rhs);
        }
        
        AmVec3f operator+ (const AmVec3f &rhs) const
        {
            AmVec3f re(rhs);
            re.mData[0] += mData[0];
            re.mData[1] += mData[1];
            re.mData[2] += mData[2];
            return re;
        }
        
        AmVec3f operator- (const AmVec3f &rhs) const
        {
            AmVec3f re(*this);
            re.mData[0] -= rhs.mData[0];
            re.mData[1] -= rhs.mData[1];
            re.mData[2] -= rhs.mData[2];
            return re;
        }
        
        AmVec3f operator/ (const float rhs) const
        {
            assert(rhs != 0);
            AmVec3f re(*this);
            re.mData[0] /= rhs;
            re.mData[1] /= rhs;
            re.mData[2] /= rhs;
            return re;
        }
        
        AmVec3f operator* (const float rhs) const
        {
            AmVec3f re(*this);
            re.mData[0] *= rhs;
            re.mData[1] *= rhs;
            re.mData[2] *= rhs;
            return re;
        }
        
        AmVec3f operator/ (const AmVec3f &rhs) const
        {
            assert(! rhs.orVal(0) );
            AmVec3f re(*this);
            re.mData[0] /= rhs.mData[0];
            re.mData[1] /= rhs.mData[1];
            re.mData[2] /= rhs.mData[2];
            return re;
        }
        
        AmVec3f operator* (const AmVec3f &rhs) const
        {
            AmVec3f re(*this);
            re.mData[0] *= rhs.mData[0];
            re.mData[1] *= rhs.mData[1];
            re.mData[2] *= rhs.mData[2];
            return re;
        }
        
        float dot(const AmVec3f &rhs) const
        {
            return (mData[0]*rhs.mData[0]
                    +mData[1]*rhs.mData[1]
                    +mData[2]*rhs.mData[2]);
        }
        
        AmVec3f cross(const AmVec3f &rhs) const
        {
            return AmVec3f(mData[1]*rhs.mData[2]-rhs.mData[1]*mData[2],
                           mData[2]*rhs.mData[0]-rhs.mData[2]*mData[0],
                           mData[0]*rhs.mData[1]-rhs.mData[0]*mData[1]);
        }
        
        float det(const AmVec3f &b, const AmVec3f &c) const
        {
            float re = x()* b.y()* c.z();
            re += b.x() * c.y() * z();
            re += c.x() * y() * b.z();
            re -= c.x() * b.y() * z();
            re -= x() * c.y() * b.z();
            re -= b.x() * y() * c.z();
            return re;
        }
        
        bool operator == (const AmVec3f &rhs) const
        {
            return (mData[0] == rhs.mData[0] && mData[1] == rhs.mData[1]
                    && mData[2] == rhs.mData[2]);
        }
        
        void normalize()
        {
            float s = sqrt(mData[0]*mData[0]
                           + mData[1]*mData[1]
                           + mData[2]*mData[2]);
            if (s == 0) {
                return;
            }
            mData[0] /= s;
            mData[1] /= s;
            mData[2] /= s;
        }
        
        void setUpper(float upper)
        {
            if (mData[0] > upper) {
                mData[0] = upper;
            }
            if (mData[1] > upper) {
                mData[1] = upper;
            }
            if (mData[2] > upper) {
                mData[2] = upper;
            }
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
    
    
    class AmRayTracer;
    class AmModel;
    class AmCamera;
    class AmLight;
    class AmTriangle;
    class AmMaterial;
    typedef shared_ptr<AmModel> AmModelPtr;
    typedef shared_ptr<AmCamera> AmCameraPtr;
    typedef shared_ptr<AmLight> AmLightPtr;
    typedef shared_ptr<AmRayTracer> AmRayTracerPtr;
    
    typedef shared_ptr<float> AmFloatPtr;
    typedef shared_ptr<unsigned int> AmUintPtr;
}


#endif
