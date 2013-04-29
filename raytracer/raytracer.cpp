//
//  raytracer.cpp
//  raytracer
//
//  Created by ambling on 13-4-26.
//  Copyright (c) 2013年 ambling. All rights reserved.
//

#include "raytracer.h"
#include "model.h"

using namespace std;
using namespace raytracer;



// constructor of the ray, generate a ray from the camera, at pixel [w,h]
AmRay::AmRay(const AmCameraPtr &camera, int w, int h)
{
    orig = camera->eye;
    dir = camera->base + (camera->vecx * w) + (camera->vecy * h) - orig;
    dir.normalize();
}


// render the model with the camera info, put the result into buffer
void AmRayTracer::render(AmUintPtr &pixels)
{
    int idx = 0;
    for (int h = 0; h < camera->height; h++) {
        for (int w = 0; w < camera->width; w++) {
            AmRay ray(camera, w, h);

            
            AmVec3f color = rayTracing(ray, maxDepth);
            
            color = color * 255;
            pixels.get()[idx] = static_cast<unsigned int>(color.x()) |
                        (static_cast<unsigned int>(color.x()) << 8) |
                        (static_cast<unsigned int>(color.x()) << 16);
            
            idx += 1;
        }
    }
}


// ray tracing and set the value to color
AmVec3f AmRayTracer::rayTracing(const AmRay &ray, const int depth)
{
    AmVec3f color(0, 0, 0);
    if (depth == 0) {
        return color;
    }
    
    AmVec3f addcolor(0,0,0);
    int minMesh = -1;
    
    //get the nearest hit point of the ray and the model
    float hit = getHitPoint(ray, minMesh);
    
    if (hit > EPSILON) {
        /* get the intersection, calculate the color
         * Phong shading:
         * intensity = diffuse * (L.N) + specular * (V.R)^shinniness + ambient
         * (L is the vector from the intersection point to the light source,
         *  N is the plane normal, V is the view direction
         *  and R is L reflected in the surface)
         */
        
        AmTriangle *triangle = & model->mTriangles[minMesh];
        AmGroup *group = & model->mGroups[triangle->group];
        AmMaterial *material = & model->mMaterials[group->material];
        
        // ambient part
        AmVec3f matAmbient(material->ambient[0]*material->ambient[3],
                           material->ambient[1]*material->ambient[3],
                           material->ambient[2]*material->ambient[3]);
        
        for (int i = 0; i < lights.size(); i++) {
            if (lights[i]->type == AmLight::AM_AMBIENT) {
                color = color + ( matAmbient *
                        AmVec3f(lights[i]->value[0] * lights[i]->value[3],
                                 lights[i]->value[1] * lights[i]->value[3],
                                 lights[i]->value[2] * lights[i]->value[3]) );
            }
        }
        
        // check if shadowed,
        //  if not, get the shadow rays into the vector
        vector<AmRay> shadowRays;
        shadowRay(hit, minMesh, ray, shadowRays);
        
        // for each visible shadow ray, get the diffusive and reflective color
        for (int i = 0; i < shadowRays.size(); i++) {
            color = color + getDiffColor(shadowRays[i], minMesh, material);
            color = color + getReflColor(ray, shadowRays[i], minMesh, material);
        }
//
//        // generate tracing ray for reflection and refraction
//        AmVec3f pos = ray.orig + (ray.dir * hit);
//        if (material->illum >= 3 && material->illum <= 7) {
//            AmVec3f refl = getReflRayDir(ray.dir*(-1.0), model->mTriNorms[minMesh]);
//            color = color + rayTracing(AmRay(pos, refl), depth-1);
//        }
//        
//        if (material->transperancy < 1 && material->density != 0) {
//            // need refraction
//            color.setUpper(1.0);
//            color = color * material->transperancy;
//            
//            if (material->illum >= 6 && material->illum <= 7) {
//                // move front a little
//                pos = pos + ray.dir * 2 * EPSILON;
//                AmVec3f refr = getRefrRayDir(ray.dir,
//                                        model->mTriNorms[minMesh], material);
//                color = color + (rayTracing(AmRay(pos, refr), depth-1)
//                                 * (1-material->transperancy));
//            }
//            
//        }
        
        //color clipped to [0, 1.0]
        color.setUpper(1.0);
    } else {
        //not intersection, color is black
        return color;
    }
    
    return color;
    
}


// get the hit point of the ray and the model,
// as well as the index of the mesh, return -1 if there is no intersection
float AmRayTracer::getHitPoint(const AmRay &ray, int &index)
{
    // if not using kd-tree, iterate all the meshes
    float mindis = -1;
    bool hitOrNot = false;
    for(unsigned int i = 0; i < model->mTriangles.size(); i++)
    {
        unsigned int * vindices = model->mTriangles[i].vindices;
        float hit = hitMesh(ray, model->mVertices[vindices[0]],
                            model->mVertices[vindices[1]],
                            model->mVertices[vindices[2]]);

        if(hit > EPSILON)
        {
            if(!hitOrNot || hit < mindis)
            {
                hitOrNot = true;
                mindis = hit;
                index = i;
            }		
        }
    }
    
    return mindis;
}

// diffuse * (L.N)
AmVec3f AmRayTracer::getDiffColor(const AmRay &shadowRay,
                                  const int index,
                                  const AmMaterial *material)
{
    AmVec3f color(material->diffuse[0] * material->diffuse[3],
                  material->diffuse[1] * material->diffuse[3],
                  material->diffuse[2] * material->diffuse[3]);
    
    float ln = shadowRay.dir.dot(model->mTriNorms[index]);
    ln = max(ln, float(0)); // if the direction is negative, set it to black
    color = color * ln;
    return color;
}

// specular * (V.R)^shinniness
AmVec3f AmRayTracer::getReflColor(const AmRay &ray,
                                  const AmRay &shadowRay,
                                  const int index,
                                  const AmMaterial *material)
{
    AmVec3f color(material->specular[0] * material->specular[3],
                  material->specular[1] * material->specular[3],
                  material->specular[2] * material->specular[3]);
    
    AmVec3f refl = getReflRayDir(shadowRay.dir, model->mTriNorms[index]);
    
    // (V.R)^shinniness
    float vr = refl.dot(ray.dir * (-1.0));
    vr = max(vr, float(0));
    vr = pow(vr, material->shininess);
    
    return color * vr;
}

AmVec3f AmRayTracer::getReflRayDir(const AmVec3f &D, const AmVec3f &N)
{
    // R: reflected ray's direction
    // R = (D.N)N - (D-(D.N)N)
    AmVec3f norm = N * (D.dot(N));
    AmVec3f refl = norm*2 - D;
    refl.normalize();
    return refl;
}

AmVec3f AmRayTracer::getRefrRayDir(const AmVec3f &D, const AmVec3f &N,
                                   const AmMaterial *material)
{
    assert(material->density != 0);//should have non-zero density
    float density = material->density;
    AmVec3f n(N);
    
    if (D.dot(N) < 0) {
        // from in to out
        n = n * (-1.0);
        density = 1.0 / density;
    }
    
    float cosIn = D.dot(n);
    float cos2Out = 1.0 - (1.0 - cosIn * cosIn) * (density * density);
    if(cos2Out < 0) cos2Out = 0; // full reflection
    
    float cosOut = sqrt(cos2Out);
    float sinOut = sqrt(1.0-cos2Out);
    
    // D-(D.N)N
    AmVec3f DmN = D - n * (n.dot(D));
    DmN.normalize();
    return n*cosOut + DmN*sinOut;
    
}

// for each light, check if it can reach the mesh,
//  the shadow ray's direction is from the mesh to the light
void AmRayTracer::shadowRay(const float hit, const int index,
                            const AmRay &ray, vector<AmRay> &shadowRays)
{
    for (int i = 0; i < lights.size(); i++) {
        if (lights[i]->type != AmLight::AM_POSITION) {
            continue;
        }
        AmLightPtr light = lights[i];
        AmVec3f pos = ray.orig + (ray.dir * hit);//hit position
        AmVec3f dir = AmVec3f(light->value[0], light->value[1], light->value[2])
                        - pos;
        
        float dis = sqrt(dir.dot(dir)); //distance
        dir.normalize();
        AmRay ray(pos, dir);
        
        int hitMesh = -1;
        float hitAgain = getHitPoint(ray, hitMesh);
        if (hitMesh != index && hitAgain > EPSILON && hitAgain < dis) {
            // hit another mesh
            continue;
        }
        shadowRays.push_back(ray);
    }
}

// get the hit point of the ray and the triangle (a, b, c),
//  using Intersection with Barycentric Triangle:
//  http://groups.csail.mit.edu/graphics/classes/
//        6.837/F04/lectures/02_RayCasting_II.pdf
// return -1 if there is no intersection
float AmRayTracer::hitMesh(const AmRay &ray, const AmVec3f &a,
                const AmVec3f &b, const AmVec3f &c)
{
	AmVec3f t1 = a - b;
	AmVec3f t2 = a - c;
	AmVec3f t3 = a - ray.orig;
    
    float tmpA = t1.det(t2, ray.dir);
	float beta = t3.det(t2, ray.dir) / tmpA;
	float gama = t1.det(t3, ray.dir) / tmpA;
    
    if(beta + gama <= 1 && beta >= 0 && gama >= 0)
	{//intersected
		float t = t1.det(t2, t3) / tmpA;
		return t;
	}
    
	return -1;		//no intersection
}


