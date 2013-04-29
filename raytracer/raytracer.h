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
        
        AmRay(const AmVec3f &o, const AmVec3f &d)
            :orig(o), dir(d)
        {}
        
        AmRay(const AmCameraPtr &camera, int w, int h);
        
        bool operator == (AmRay &rhs)
        {
            return (orig == rhs.orig && dir == rhs.dir);
        }
        
    };
    
    /*
     * space plane for kd-tree
     */
    class AmPlane
    {
    public:
        enum AmAxis{AM_X, AM_Y, AM_Z};
        
        AmAxis  axis;
        float   value;
        int     index;
        
        AmPlane()
        :axis(AM_X), value(0), index(0)
        {}
    };
    
    
    /*
     * kd-tree node
     */
    class AmKDTreeNode
    {
    public:
        int     parent;         // index of the parent in tree node vector
        int     leftChild;      // index of left child
        int     rightChild;     // index of right child
        int     sibling;        // index of sibling
        
        bool    leaf;           // is leaf or not
        int     depth;
        AmVec3f start;          // start of bounding box
        AmVec3f end;            // end of bounding box
        
        AmPlane plane;
        vector<int> meshes;     //the indices of meshes in this node
        
        AmKDTreeNode()
        :start(0,0,0), end(0,0,0)
        {
            parent = leftChild = rightChild = sibling = -1;
            leaf = false;
            depth = 0;
        }
    };
    typedef shared_ptr<AmKDTreeNode> AmKDTreeNodePtr;
    
    
    /*
     * kd-tree to accelerate the tracing
     */
    class AmKDTree
    {
    private:
        AmModelPtr  model;
        
    public:
        vector<AmKDTreeNodePtr>    nodes;
        
        AmKDTree()
        {}
        
        AmKDTree(const AmModelPtr &m)
        :model(m)
        {}
        
        void setModel(const AmModelPtr &m)
        {
            model = m;
        }
        
        void    init();               // build the kdtree from the model;
        float   search(const AmRay &ray, int &index); //search for intersection
        
    private:
        void buildNode(int index); // build from the node
        bool terminate(int index);
        void splitNode(int index);
        void findPlane(int index);
        int  meshInNode(int mesh, const AmKDTreeNodePtr &node);
        
        bool hitBox(int index, const AmRay &ray, float &tmin, float &tmax);
        bool searchLeaf(int node, const AmRay &ray, int &index, float &hit);
        
    };
    
    
    /*
     * the class that implements ray tracing algorithm
     */
    class AmRayTracer
    {
        AmModelPtr      model;
        AmCameraPtr     camera;
        vector<AmLightPtr> lights;
        int             maxDepth;
        
        AmKDTree        kdtree;
        
    public:
        AmRayTracer()
            :maxDepth(3)
        {}
     
        AmRayTracer(const AmModelPtr &m)
            :maxDepth(3), model(m), kdtree(m)
        {
            kdtree.init();
        }
        
        void setModel(const AmModelPtr &m)
        {
            model = m;
            kdtree.setModel(m);
            kdtree.init();
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
        
        // static function to get the intersection of a ray and mesh
        static float hitMesh(const AmRay &ray, const AmVec3f &a,
                             const AmVec3f &b, const AmVec3f &c);
        
    private:
        AmVec3f rayTracing(const AmRay &ray, const int depth);
        float   getHitPoint(const AmRay &ray, int &index);
        
        void    shadowRay(const float hit, const int index,
                          const AmRay &ray, vector<AmRay> &shadowRays);
        
        AmVec3f getDiffColor(const AmRay &shadowRay,
                             const int index,
                             const AmMaterial *material);
        
        AmVec3f getReflRayDir(const AmVec3f &D, const AmVec3f &N);
        AmVec3f getReflColor(const AmRay &ray,
                             const AmRay &shadowRay,
                             const int index,
                             const AmMaterial *material);
        
        AmVec3f getRefrRayDir(const AmVec3f &D, const AmVec3f &N,
                              const AmMaterial *material);
    };


}


#endif
