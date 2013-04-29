//
//  model.cpp
//  raytracer
//
//  read data in wavefront obj file,
//  see http://en.wikipedia.org/wiki/Wavefront_.obj_file ,
//  referred to Nate Robins' code:
//  nate@pobox.com, http://www.pobox.com/~nate
//
//  Created by ambling on 13-4-24.
//  Copyright (c) 2013年 ambling. All rights reserved.
//

#include "model.h"

#include <sstream>

using namespace std;
using namespace raytracer;

/*
 * constructor, read the obj file and load the data
 *
 * filename - name of the file containing the Wavefront .OBJ format data.
 *
 */

AmModel::AmModel(string filename)
    :mPathname(filename)
{
    /* open the file */
    ifstream ifs(filename);
    assert(ifs);
    
    // make a pass through the file to read in the data
    pass(ifs);
}

/* pass: pass through the Wavefront OBJ file that gets all
 * the data.
 *
 * ifs-- ifstream of the OBJ file
 */
void AmModel::pass(ifstream &ifs)
{
    unsigned int numvertices;        /* number of vertices in model */
    unsigned int numnormals;         /* number of normals in model */
    unsigned int numtexcoords;       /* number of texcoords in model */
    unsigned int numtriangles;       /* number of triangles in model */
    unsigned int material;           /* current material */
    unsigned int group;              /* current group */
    
    // pass through the file, read all the data
    numvertices = numnormals = numtexcoords = 1;//indices begin with 1
    mVertices.push_back(AmVec3f(0,0,0));
    mTexcoords.push_back(AmVec2f(0,0));
    mNormals.push_back(AmVec3f(0,0,0));
    numtriangles = 0;
    material = group = 0;
    
    while(!ifs.eof()) {
        char buf[128];
        ifs.getline(buf, 128);
        istringstream sreader(buf);
        
        string first, remain;
        AmVec3f vec3f(0, 0, 0);
        AmVec2f vec2f(0, 0);
        
        switch(buf[0]) {
            case '#':               /* comment */
                break;
            case 'v':               /* v, vn, vt */
                sreader>>first;
                if(first == "v")
                {/* vertex */
                    sreader>>vec3f.mData[0]
                            >>vec3f.mData[1]
                            >>vec3f.mData[2];
                    mVertices.push_back(vec3f);
                    numvertices++;
                } else if(first == "vn") {
                    /* normal */
                    sreader>>vec3f.mData[0]
                            >>vec3f.mData[1]
                            >>vec3f.mData[2];
                    mNormals.push_back(vec3f);
                    numnormals++;
                } else if(first == "vt"){
                    /* texcoord */
                    sreader>>vec2f.mData[0]
                            >>vec2f.mData[1];
                    mTexcoords.push_back(vec2f);
                    numtexcoords++;
                }
                break;
            case 'm':
                sreader>>first>>mMtllibname;
                readMTL();
                break;
            case 'u':
                sreader>>first>>remain;
                mGroups[group].material = material = findMaterial(remain);
                break;
            case 'g':               /* group */
                sreader>>first>>remain;
                group = findGroup(remain);
                mGroups[group].material = material;
                break;
            case 'f':               /* face */
            {
                int v, n, t;
                v = n = t = 0;
                AmTriangle triangle(group);
                sreader>>first>>remain;
                /* can be one of %d, %d//%d, %d/%d, %d/%d/%d */
                if (sscanf(remain.c_str(), "%d//%d", &v, &n) == 2) {
                    /* v//n */
                    assert(v>0);assert(n>0);//do not accept negative indices
                    triangle.vindices[0] = v;
                    triangle.nindices[0] = n;
                    
                    sreader>>remain;
                    sscanf(remain.c_str(), "%d//%d", &v, &n);
                    assert(v>0);assert(n>0);//do not accept negative indices
                    triangle.vindices[1] = v;
                    triangle.nindices[1] = n;
                    
                    sreader>>remain;
                    sscanf(remain.c_str(), "%d//%d", &v, &n);
                    assert(v>0);assert(n>0);//do not accept negative indices
                    triangle.vindices[2] = v;
                    triangle.nindices[2] = n;
                    
                    mTriangles.push_back(triangle);
                    mGroups[group].triangles.push_back(numtriangles);
                    numtriangles++;
                    assert(mTriangles.size() == numtriangles);//check count
                    while(sreader>>remain) {
                        AmTriangle nextTri(group);
                        sscanf(remain.c_str(), "%d//%d", &v, &n);
                        assert(v>0);assert(n>0);//do not accept negative indices
                        nextTri.vindices[0] = triangle.vindices[0];
                        nextTri.nindices[0] = triangle.nindices[0];
                        nextTri.vindices[1] = triangle.vindices[2];
                        nextTri.nindices[1] = triangle.nindices[2];
                        nextTri.vindices[2] = v;
                        nextTri.nindices[2] = n;
                        
                        mTriangles.push_back(nextTri);
                        mGroups[group].triangles.push_back(numtriangles);
                        numtriangles++;
                        assert(mTriangles.size() == numtriangles);//check count
                    }
                } else if(sscanf(remain.c_str(), "%d/%d/%d", &v, &t, &n) == 3) {
                    /* v/t/n */
                    //do not accept negative indices
                    assert(v>0);assert(t>0);assert(n>0);
                    triangle.vindices[0] = v;
                    triangle.tindices[0] = t;
                    triangle.nindices[0] = n;
                    
                    sreader>>remain;
                    sscanf(remain.c_str(), "%d/%d/%d", &v, &t, &n);
                    //do not accept negative indices
                    assert(v>0);assert(t>0);assert(n>0);
                    triangle.vindices[1] = v;
                    triangle.tindices[1] = t;
                    triangle.nindices[1] = n;
                    
                    sreader>>remain;
                    sscanf(remain.c_str(), "%d/%d/%d", &v, &t, &n);
                    //do not accept negative indices
                    assert(v>0);assert(t>0);assert(n>0);
                    triangle.vindices[2] = v;
                    triangle.tindices[2] = t;
                    triangle.nindices[2] = n;
                    
                    mTriangles.push_back(triangle);
                    mGroups[group].triangles.push_back(numtriangles);
                    numtriangles++;
                    assert(mTriangles.size() == numtriangles);//check count
                    while(sreader>>remain) {
                        AmTriangle nextTri(group);
                        sscanf(remain.c_str(), "%d/%d/%d", &v, &t, &n);
                        //do not accept negative indices
                        assert(v>0);assert(t>0);assert(n>0);
                        
                        nextTri.vindices[0] = triangle.vindices[0];
                        nextTri.tindices[0] = triangle.tindices[0];
                        nextTri.nindices[0] = triangle.nindices[0];
                        nextTri.vindices[1] = triangle.vindices[2];
                        nextTri.tindices[1] = triangle.tindices[2];
                        nextTri.nindices[1] = triangle.nindices[2];
                        nextTri.vindices[2] = v;
                        nextTri.tindices[2] = t;
                        nextTri.nindices[2] = n;
                        
                        mTriangles.push_back(nextTri);
                        mGroups[group].triangles.push_back(numtriangles);
                        numtriangles++;
                        assert(mTriangles.size() == numtriangles);//check count
                    }
                } else if (sscanf(remain.c_str(), "%d/%d", &v, &t) == 2) {
                    /* v/t */
                    assert(v>0);assert(t>0);//do not accept negative indices
                    triangle.vindices[0] = v;
                    triangle.tindices[0] = t;
                    
                    sreader>>remain;
                    sscanf(remain.c_str(), "%d/%d", &v, &t);
                    assert(v>0);assert(t>0);//do not accept negative indices
                    triangle.vindices[1] = v;
                    triangle.tindices[1] = t;
                    
                    sreader>>remain;
                    sscanf(remain.c_str(), "%d/%d", &v, &t);
                    assert(v>0);assert(t>0);//do not accept negative indices
                    triangle.vindices[2] = v;
                    triangle.tindices[2] = t;
                    
                    mTriangles.push_back(triangle);
                    mGroups[group].triangles.push_back(numtriangles);
                    numtriangles++;
                    assert(mTriangles.size() == numtriangles);//check count
                    while(sreader>>remain) {
                        AmTriangle nextTri(group);
                        sscanf(remain.c_str(), "%d/%d", &v, &t);
                        assert(v>0);assert(t>0);//do not accept negative indices
                        nextTri.vindices[0] = triangle.vindices[0];
                        nextTri.tindices[0] = triangle.tindices[0];
                        nextTri.vindices[1] = triangle.vindices[2];
                        nextTri.tindices[1] = triangle.tindices[2];
                        nextTri.vindices[2] = v;
                        nextTri.tindices[2] = t;
                        
                        mTriangles.push_back(nextTri);
                        mGroups[group].triangles.push_back(numtriangles);
                        numtriangles++;
                        assert(mTriangles.size() == numtriangles);//check count
                    }
                    
                } else {
                    /* v */
                    sscanf(remain.c_str(), "%d", &v);
                    assert(v>0);//do not accept negative indices
                    triangle.vindices[0] = v;
                    
                    sreader>>remain;
                    sscanf(remain.c_str(), "%d", &v);
                    assert(v>0);//do not accept negative indices
                    triangle.vindices[1] = v;
                    
                    sreader>>remain;
                    sscanf(remain.c_str(), "%d", &v);
                    assert(v>0);//do not accept negative indices
                    triangle.vindices[2] = v;
                    
                    mTriangles.push_back(triangle);
                    mGroups[group].triangles.push_back(numtriangles);
                    numtriangles++;
                    assert(mTriangles.size() == numtriangles);//check count
                    while(sreader>>remain) {
                        AmTriangle nextTri(group);
                        sscanf(remain.c_str(), "%d", &v);
                        assert(v>0);//do not accept negative indices
                        nextTri.vindices[0] = triangle.vindices[0];
                        nextTri.vindices[1] = triangle.vindices[2];
                        nextTri.vindices[2] = v;
                        
                        mTriangles.push_back(nextTri);
                        mGroups[group].triangles.push_back(numtriangles);
                        numtriangles++;
                        assert(mTriangles.size() == numtriangles);//check count
                    }
                }
            }
                break;
            default:
                break;
        }
    }
}

/*
 * find material index from its name
 */
unsigned int AmModel::findMaterial(string name)
{
    for (int i = 0; i < mMaterials.size(); i++) {
        if (mMaterials[i].name == name) {
            return i;
        }
    }
    
    cerr<<"can't find material: "<<name<<endl;
    return 0;
}

/*
 * find group index from its name, if not exist, insert a new one with the name
 */
unsigned int AmModel::findGroup(string name)
{
    for (int i = 0; i < mGroups.size(); i++) {
        if (mGroups[i].name == name) {
            return i;
        }
    }
    
    mGroups.push_back(AmGroup(name));
    return static_cast<unsigned int>(mGroups.size()-1);
}


/* readMTL: read a wavefront material library file
 *
 */

void AmModel::readMTL()
{
//    FILE* file;
//    char* dir;
//    char* filename;
//    char buf[128];
//    GLuint nummaterials, i;
    
    string dir = CommonFuncs::getDirName(mPathname);
    string filename = dir + mMtllibname;
    
    // open the file
    ifstream ifs(filename);
    assert(ifs);// the file must be opened
    
    
    /* now, read in the data */
    if (mMaterials.size() == 0) {
        mMaterials.push_back(AmMaterial());//first one of blank material
    }
    unsigned long material = mMaterials.size()-1; //current material index
    while(!ifs.eof()) {
        char buf[128];
        ifs.getline(buf, 128);
        istringstream sreader(buf);
        string first, remain;
        switch(buf[0]) {
            case '#':               /* comment */
                break;
            case 'n':               /* newmtl */
                sreader>>first>>remain;
                mMaterials.push_back(AmMaterial(remain));
                material++;
                break;
            case 'd':
                // transperancy
                sreader>>first>>mMaterials[material].transperancy;
                break;
            case 'T':
                // transperancy
                sreader>>first>>mMaterials[material].transperancy;
                break;
            case 'N':
            {
                switch (buf[1]) {
                    case 's':
                        // shininess
                        sreader>>first>>mMaterials[material].shininess;
                        // wavefront shininess is from [0, 1000], so scale for OpenGL */
                        mMaterials[material].shininess *= 128.0 / 1000.0;
                        break;
                    case 'i':
                        // density
                        sreader>>first>>mMaterials[material].density;
                        break;
                    default:
                        break;
                }
                
                break;
            }
            case 'i':               /* illum */
                sreader>>first>>mMaterials[material].illum;
                break;
            case 'K':
                switch(buf[1]) {
                    case 'd':
                        sreader>>first
                                >>mMaterials[material].diffuse[0]
                                >>mMaterials[material].diffuse[1]
                                >>mMaterials[material].diffuse[2];
                        break;
                    case 's':
                        sreader>>first
                                >>mMaterials[material].specular[0]
                                >>mMaterials[material].specular[1]
                                >>mMaterials[material].specular[2];
                        break;
                    case 'a':
                        sreader>>first
                                >>mMaterials[material].ambient[0]
                                >>mMaterials[material].ambient[1]
                                >>mMaterials[material].ambient[2];
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

}

// utilize the model:
//  scale and translate the model to the unit cube around the origin,
//  and calculate the normal direction vector of each triangle.
//  Assumes a counter-clockwise winding.
void AmModel::utilize()
{
    AmVec3f vmax(M_MIN, M_MIN, M_MIN);
    AmVec3f vmin(M_MAX, M_MAX, M_MAX);
    for (unsigned int i = 1; i < mVertices.size(); i++) {
        if (mVertices[i].x() > vmax.x()) {
            vmax.setX(mVertices[i].x());
        }
        if (mVertices[i].y() > vmax.y()) {
            vmax.setY(mVertices[i].y());
        }
        if (mVertices[i].z() > vmax.z()) {
            vmax.setZ(mVertices[i].z());
        }
        if (mVertices[i].x() < vmin.x()) {
            vmin.setX(mVertices[i].x());
        }
        if (mVertices[i].y() < vmin.y()) {
            vmin.setY(mVertices[i].y());
        }
        if (mVertices[i].z() < vmin.z()) {
            vmin.setZ(mVertices[i].z());
        }
    }
    
    // the width, height and depth of the model
    float w = abs(vmax.x()) + abs(vmin.x());
    float h = abs(vmax.y()) + abs(vmin.y());
    float d = abs(vmax.z()) + abs(vmin.z());
    
    // the center of the model
    float cx = (vmax.x() + vmin.x()) / 2;
    float cy = (vmax.y() + vmin.y()) / 2;
    float cz = (vmax.z() + vmin.z()) / 2;
    
    // scale and translate the model
    float scale = 2.0 / max(max(w, h), d);
    for (unsigned int i = 0; i < mVertices.size(); i++) {
        mVertices[i].mData[0] -= cx;
        mVertices[i].mData[1] -= cy;
        mVertices[i].mData[2] -= cz;
        mVertices[i].mData[0] *= scale;
        mVertices[i].mData[1] *= scale;
        mVertices[i].mData[2] *= scale;
    }
    
    // get the normals of triangles
    mTriNorms.clear();
    for (unsigned int i = 0; i < mTriangles.size(); i++) {
        
        AmVec3f u = mVertices[mTriangles[i].vindices[1]]
                    - mVertices[mTriangles[i].vindices[0]];
        AmVec3f v = mVertices[mTriangles[i].vindices[2]]
                    - mVertices[mTriangles[i].vindices[1]];
        mTriNorms.push_back(u.cross(v));
        mTriNorms[i].normalize();
    }
}


//////Camera Class///////////
void AmCamera::update()
{
    dir = center - eye;
    float dis = sqrt(dir.dot(dir)); //distance
    dir.normalize();
    
    float scale = tan(angle / 2.0) * dis * 2.0 / width;
    vecx = dir.cross(up);
    vecx.normalize();
    vecx = vecx * scale;
    vecy = vecx.cross(dir);
    vecy.normalize();
    vecy = vecy * scale;
    
    // from center to the base of the coordinate
    base = center - (vecx * width / 2) - (vecy * height / 2);
}



