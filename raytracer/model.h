//
//  model.h
//  raytracer
//
//  read data in wavefront obj file
//  see http://en.wikipedia.org/wiki/Wavefront_.obj_file ,
//  referred to Nate Robins' code:
//  nate@pobox.com, http://www.pobox.com/~nate
//
//  Created by ambling on 13-4-23.
//  Copyright (c) 2013年 ambling. All rights reserved.
//

#ifndef raytracer_model_h
#define raytracer_model_h

#include "utils.h"

#include <fstream>


namespace raytracer {
    
    /*
     * AmMaterial: class that defines a material in a model.
     *  See:
     *  http://www.mathworks.com/matlabcentral/fileexchange/
            27982-wavefront-obj-toolbox/content/
            help%20file%20format/MTL_format.html
     *  http://en.wikipedia.org/wiki/Wavefront_.obj_file
     */
    class AmMaterial {
        
    public:
        string name;
        float diffuse[4];     
        float ambient[4];     
        float specular[4];    
        float emmissive[4];   
        float shininess;// the specular exponent for the current material. --Ns
        float transperancy; //                                       --Tr or d
        float density;// the optical density for the surface,
                      //    also known as index of refraction,
                      //    works when illum is 6 or 7                    --Ni
        
        /* illumination models
         0. Color on and Ambient off
         1. Color on and Ambient on
         2. Highlight on
         3. Reflection on and Ray trace on
         4. Transparency: Glass on, Reflection: Ray trace on
         5. Reflection: Fresnel on and Ray trace on
         6. Transparency: Refraction on, Reflection: Fresnel off, Ray trace on
         7. Transparency: Refraction on, Reflection: Fresnel on, Ray trace on
         8. Reflection on and Ray trace off
         9. Transparency: Glass on, Reflection: Ray trace off
         10. Casts shadows onto invisible surfaces
         */
        int   illum;          // illumination models
        
        
        AmMaterial()
            :diffuse{0.8, 0.8, 0.8, 1.0},
            ambient{0.2, 0.2, 0.2, 1.0},
            specular{0.0, 0.0, 0.0, 1.0},
            emmissive{0.0, 0.0, 0.0, 0.0},
            shininess(65.0),
            transperancy(1),
            density(1.0),
            name("__AM_FIRST_BLANK_MATERIAL__")
        {}
        
        AmMaterial(string n)
            :diffuse{0.8, 0.8, 0.8, 1.0},
            ambient{0.2, 0.2, 0.2, 1.0},
            specular{0.0, 0.0, 0.0, 1.0},
            emmissive{0.0, 0.0, 0.0, 0.0},
            shininess(65.0),
            transperancy(1),
            density(1.0),
            name(n)
        {}
    };
    
    /*
     * AmTriangle: class that defines a triangle in a model.
     */
    class AmTriangle {
        
    public:
        unsigned int vindices[3];      // array of triangle vertex indices 
        unsigned int nindices[3];      // array of triangle normal indices 
        unsigned int tindices[3];      // array of triangle texcoord indices
        unsigned int group;            // index of triangle group
        
        AmTriangle(unsigned int g)
            :vindices{0,0,0}, nindices{0,0,0}, tindices{0,0,0}, group(g)
        {}
        
    };
    
    /* 
     * AmGroup: class that defines a group in a model.
     */
    class AmGroup{
        
    public:
        string          name;               /* name of this group */
        vector<unsigned int>    triangles;  /* array of triangle indices */
        unsigned int    material;           /* index to material for group */
        
        
        AmGroup(string name)
            :name(name), material(0)
        {}
    };
    
    
    /*
     * AmModel: class that stores all info of the scene model
     */
    class AmModel {
        
        string mPathname;
        string mMtllibname;                 // name of the material library
    
    public:
        vector<AmTriangle>  mTriangles;     // triangles of the scene
        vector<AmVec3f>     mVertices;
        vector<AmVec3f>     mNormals;       // normals of vertex
        vector<AmVec3f>     mTriNorms;      // normals of triangles
        vector<AmVec2f>     mTexcoords;
        vector<AmMaterial>  mMaterials;
        vector<AmGroup>     mGroups;
        
    public:
        AmModel(string pathname);
        
        void utilize();

    private:
        void readOBJ(string filename);
        void pass(ifstream &ifs);
        unsigned int findMaterial(string name);
        unsigned int findGroup(string name);
        void readMTL();
        
    };
    
    
    /*
     * camera info
     */
    class AmCamera
    {
    public:
        AmVec3f eye;
        AmVec3f center;
        AmVec3f up;
        int width;
        int height;
        float angle;
        
        AmVec3f dir;    // normalized direction vector
        AmVec3f base;   // position of (0, 0) in world coordinate
        AmVec3f vecx;   // normalized x direction vector
        AmVec3f vecy;   // normalized y direction vector
        
        AmCamera(int w, int h, const AmVec3f &e,
                 const AmVec3f &c, const AmVec3f &u)
            :width(w), height(h), angle(45), eye(e), center(c), up(u)
        {
            update();
        }
        AmCamera(int w, int h, float an, const AmVec3f &e,
                 const AmVec3f &c, const AmVec3f &u)
        :width(w), height(h), angle(an), eye(e), center(c), up(u)
        {
            update();
        }
        
        void update();
    };
    
    /*
     * light info
     */
    class AmLight
    {
    public:
        enum GL_TYPE
        {
            AM_POSITION =   0x1203,
            AM_AMBIENT  =   0x1200,
            AM_DIFFUSE  =   0x1201,
            AM_SPECULAR =   0x1202,
        };
        
        enum GL_NAME
        {
            AM_LIGHT0   =   0x4000,
            AM_LIGHT1   =   0x4001,
            AM_LIGHT2   =   0x4002,
            AM_LIGHT3   =   0x4003,
            AM_LIGHT4   =   0x4004,
            AM_LIGHT5   =   0x4005,
            AM_LIGHT6   =   0x4006,
            AM_LIGHT7   =   0x4007,
        };
        
        GL_TYPE type;
        GL_NAME name;
        float value[4];
        
        AmLight(GL_TYPE t, GL_NAME n, float * v)
            :type(t), name(n), value{v[0], v[1], v[2], v[3]}
        {}
    };
    
}



#endif
