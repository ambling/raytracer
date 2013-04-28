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
            
            pixels.get()[idx] = (int)(255*color.x()) |
                    ((int)(255*color.y()) << 8) |
                    ((int)(255*color.z()) << 16);
            
            idx ++;
        }
    }
}


// ray tracing and set the value to color
AmVec3f AmRayTracer::rayTracing(AmRay &ray, int depth)
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
        shadowRay(hit, minMesh, shadowRays);
        
        // for each visible shadow ray, get the diffusive and reflective color
        for (int i = 0; i < shadowRays.size(); i++) {
            
        }
        
        // generate tracing ray for reflection and refraction 
        
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


/*
 * get the color of this ray and generate the subray
 */
AmRay AmRayTracer::setColor(const AmRay &ray, AmVec3f &color)
{
//    //vAssign((*color), materials[meshes[obMesh].ma].color);
//    if(materials[meshes[obMesh].ma].ref == 0)
//    {//diffuse
//        vAssign((*color), materials[meshes[obMesh].ma].emi);
//        vAdd((*color), (*color), ambient);
//        
//        vec3f norm, pos, ab, bc;
//        vMul(pos, ray.dirc, t);
//        vAdd(pos, ray.orig, pos);		//position = R + tD
//        vSub(ab, vertices[meshes[obMesh].b], vertices[meshes[obMesh].a]);	//pa = a - p
//        vSub(bc, vertices[meshes[obMesh].c], vertices[meshes[obMesh].b]);	//pa = a - p
//        vCross(norm, ab, bc);		//norm == ab x bc, to determine the right direction
//#ifndef GPU_KERNEL
//        //printf("norm: ");vPrint(norm);
//        //printf("ray.dirc: ");vPrint(ray.dirc);
//#endif
//        //if(vDot(norm, ray.dirc) > EPSILON) vMul(norm, norm, -1.0); //set the direction of norm
//        vNorm(norm);
//        
//        int i;
//        for(i = 0; i < sphereNum; i++)
//        {//find the light source
//            if(spheres[i].emi.x == 0 && spheres[i].emi.y == 0
//               && spheres[i].emi.z == 0)
//                continue;
//            
//            vec3f light;
//            vSub(light, pos, spheres[i].pos);	//light = position - pi
//            float distance = sqrt(vDot(light, light));	//for dot light source
//            vNorm(light);
//            float d = vDot(norm, light);		//d = L x N
//            if(d > 0) continue;
//			
//            if(isShadow(root, i, -1, obMesh, distance, sphereNum, vertexNum, meshNum, pos,
//                        light, spheres, vertices, meshes)) continue;
//            
//            vec3f addcolor;
//            vMul(addcolor, materials[meshes[obMesh].ma].color, (-d));
//            vvMul(addcolor, addcolor, spheres[i].emi);
//            vAdd((*color), (*color), addcolor);
//        }
//        
//        return ray;
//    }
//    else if(materials[meshes[obMesh].ma].ref == 1)
//    {//specular
//        Ray newray;
//        
//        vec3f norm, pos, ab, bc;
//        vMul(pos, ray.dirc, t);
//        vAdd(pos, ray.orig, pos);		//position = R + tD
//        vSub(ab, vertices[meshes[obMesh].b], vertices[meshes[obMesh].a]);	//pa = a - p
//        vSub(bc, vertices[meshes[obMesh].c], vertices[meshes[obMesh].b]);	//pa = a - p
//        vCross(norm, ab, bc);		//norm == ab x bc, to determine the right direction
//#ifndef GPU_KERNEL
//        //printf("norm: ");vPrint(norm);
//        //printf("ray.dirc: ");vPrint(ray.dirc);
//#endif
//        //if(vDot(norm, ray.dirc) > EPSILON) vMul(norm, norm, -1.0); //set the direction of norm
//        vNorm(norm);
//        
//        vAssign(newray.orig, pos);
//        
//        vec3f tmp;
//        vMul(tmp, norm, 2*vDot(ray.dirc, norm));
//        vSub(newray.dirc, ray.dirc, tmp);	//newD = D - 2(D.N)N
//        
//        return newray;
//        
//    }
//    else if(materials[meshes[obMesh].ma].ref == 2)
//    {//refraction
//        Ray newray;
//        
//        vec3f norm, pos, ab, bc;
//        vMul(pos, ray.dirc, t);
//        vAdd(pos, ray.orig, pos);		//position = R + tD
//        vSub(ab, vertices[meshes[obMesh].b], vertices[meshes[obMesh].a]);	//pa = a - p
//        vSub(bc, vertices[meshes[obMesh].c], vertices[meshes[obMesh].b]);	//pa = a - p
//        vCross(norm, ab, bc);		//norm == ab x bc, to determine the right direction
//#ifndef GPU_KERNEL
//        //printf("norm: ");vPrint(norm);
//        //printf("ray.dirc: ");vPrint(ray.dirc);
//#endif
//        //if(vDot(norm, ray.dirc) > EPSILON) vMul(norm, norm, -1.0); //set the direction of norm
//        vNorm(norm);
//        
//        float n = materials[meshes[obMesh].ma].outrfr / materials[meshes[obMesh].ma].inrfr;	//default: n = n1 / n2
//        float cosI = - vDot(ray.dirc, norm);
//        if(cosI < 0)
//        {//the ray is inside the mesh box
//            cosI = - cosI;
//            n = 1 / n;
//            vMul(norm, norm, -1);
//        }
//        float cos2O = 1.0f - n * n * (1.0f - cosI * cosI);		//the square of cosO, may be negative
//        if(cos2O > 0)
//        {//has refraction ray
//            vec3f displace;
//            vMul(displace, ray.dirc, 0.01);
//            vAdd(newray.orig, pos, displace);
//            
//            vec3f tmp1, tmp2;
//            vMul(tmp1, ray.dirc, n);
//            float tmp = n * cosI - sqrt(cos2O);
//            vMul(tmp2, norm, tmp);
//            vAdd(newray.dirc, tmp1, tmp2);			//newD = (n * ray.dirc) + (n * cosI - sqrt( cos2O )) * N;
//            
//            return newray;
//        }
//        
//        return ray;
//    }
    return ray;
}

// for each light, check if it can reach the mesh
void AmRayTracer::shadowRay(const float hit, const int index,
                            vector<AmRay> &shadowRays)
{
    
}

// get the hit point of the ray and the triangle (a, b, c),
//  using Intersection with Barycentric Triangle:
//  http://groups.csail.mit.edu/graphics/classes/6.837/F04/lectures/02_RayCasting_II.pdf
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


