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
     */
    class AmMaterial {
        
    public:
        string name;
        float diffuse[4];     
        float ambient[4];     
        float specular[4];    
        float emmissive[4];   
        float shininess;
        
        /* illumination models
         0. Color on and Ambient off
         1. Color on and Ambient on
         2. Highlight on
         3. Reflection on and Ray trace on
         4. Transparency: Glass on, Reflection: Ray trace on
         5. Reflection: Fresnel on and Ray trace on
         6. Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
         7. Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
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
            name("default")
        {}
        
        AmMaterial(string n)
            :diffuse{0.8, 0.8, 0.8, 1.0},
            ambient{0.2, 0.2, 0.2, 1.0},
            specular{0.0, 0.0, 0.0, 1.0},
            emmissive{0.0, 0.0, 0.0, 0.0},
            shininess(65.0),
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
        unsigned int findex;           // index of triangle facet normal
        
        AmTriangle()
            :vindices{0, 0, 0}, nindices{0,0,0}, tindices{0,0,0},findex(0)
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
        vector<AmVec3f>     mNormals;       // normals of each triangle
        vector<AmVec2f>     mTexcoords;
        vector<AmMaterial>  mMaterials;
        vector<AmGroup>     mGroups;
        
        AmVec3f             mCenter;        // position of the model center
        
    public:
        AmModel(string pathname);

    private:
        void readOBJ(string filename);
        void pass(ifstream &ifs);
        unsigned int findMaterial(string name);
        unsigned int findGroup(string name);
        void readMTL();
        
        void calc();
    };
    
}



#endif
