//
//  raytracer.h
//  raytracer
//
//  Created by ambling on 13-4-26.
//  Copyright (c) 2013年 ambling. All rights reserved.
//

#ifndef raytracer_raytracer_h
#define raytracer_raytracer_h

#include "utils.h"

using namespace std;

namespace raytracer {
    
    
    /*
     * ray info
     */
    class AmRay
    {
    public:
        AmVec3f orig;
        AmVec3f dir;
        
        AmRay(const AmCameraPtr &camera, int w, int h);
        
        bool operator == (AmRay &rhs)
        {
            return (orig == rhs.orig && dir == rhs.dir);
        }
        
    };
    
    
    /*
     * the class that implements ray tracing algorithm
     */
    class AmRayTracer
    {
        AmModelPtr model;
        AmCameraPtr camera;
        vector<AmLightPtr> lights;
        int maxDepth;
        
    public:
        AmRayTracer()
            :maxDepth(5)
        {}
     
        AmRayTracer(const AmModelPtr &m)
            :maxDepth(5), model(m)
        {}
        
        void setModel(const AmModelPtr &m)
        {
            model = m;
        }
     
        void setCamera(const AmCameraPtr &c)
        {
            camera = c;
        }
        
        void setLight(const vector<AmLightPtr> l)
        {
            lights = vector<AmLightPtr>(l);
        }
        
        // render the model with the camera, put the result into buffer
        void render(AmUintPtr &pixels);
        
    private:
        AmVec3f rayTracing(AmRay &ray, int depth);
        float   getHitPoint(const AmRay &ray, int &index);
        AmRay   setColor(const AmRay &ray, AmVec3f &color);
        
        float   hitMesh(const AmRay &ray, const AmVec3f &a,
                        const AmVec3f &b, const AmVec3f &c);
        void    shadowRay(const float hit, const int index,
                          vector<AmRay> &shadowRays);
    };


}


#endif
