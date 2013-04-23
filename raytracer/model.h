//
//  model.h
//  raytracer
//
//  Created by ambling on 13-4-23.
//  Copyright (c) 2013年 ambling. All rights reserved.
//

#ifndef raytracer_model_h
#define raytracer_model_h

#include "utils.h"


namespace raytracer {
    
    /*
     * AmMaterial: class that defines a material in a model.
     */
    class AmMaterial {
        
    public:
        char*   name;              
        float diffuse[4];     
        float ambient[4];     
        float specular[4];    
        float emmissive[4];   
        float shininess;      
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
    };
    
    
    /*
     * AmModel: class that stores all info of the scene model
     */
    class AmModel {
        string pathname;
        
        vector<AmTriangle> mTriangles; //triangles of the scene
        vector<AmVec3f> mVertices;
        vector<AmVec3f> mNormals;       // normals of each triangle
        
        vector<AmMaterial> mMaterials;
        
        
    public:
        AmModel(string pathname);
    };
}



#endif
