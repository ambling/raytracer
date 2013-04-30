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
    
    int minMesh = -1;
    
    //get the nearest hit point of the ray and the model
    //float hit = getHitPoint(ray, minMesh);
    float hit = kdtree.search(ray, minMesh);
    
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

        // generate tracing ray for reflection and refraction
        AmVec3f pos = ray.orig + (ray.dir * hit);
        if (material->illum >= 3 && material->illum <= 7) {
            AmVec3f refl = getReflRayDir(ray.dir*(-1.0), model->mTriNorms[minMesh]);
            color = color + rayTracing(AmRay(pos, refl), depth-1);
        }
        
        if (material->transperancy < 1 && material->density != 0) {
            // need refraction
            color.setUpper(1.0);
            color = color * material->transperancy;
            
            if (material->illum >= 6 && material->illum <= 7) {
                // move front a little
                pos = pos + ray.dir * 2 * EPSILON;
                AmVec3f refr = getRefrRayDir(ray.dir,
                                        model->mTriNorms[minMesh], material);
                color = color + (rayTracing(AmRay(pos, refr), depth-1)
                                 * (1-material->transperancy));
            }
            
        }
        
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
        //float hitAgain = getHitPoint(ray, hitMesh);//use kdtree instead
        float hitAgain = kdtree.search(ray, hitMesh);
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


////////////// functions of kd-tree /////////////////////

// init the root node of kd-tree and call to build the whole tree
void AmKDTree::init()
{
    //clear exist data
    nodes.clear();
    
    AmKDTreeNodePtr root(new AmKDTreeNode);
    for (unsigned int i = 1; i < model->mTriangles.size(); i++) {
        // start from index 1
        root->meshes.push_back(i);
    }
    
    AmVec3f vmax(M_MIN, M_MIN, M_MIN);
    AmVec3f vmin(M_MAX, M_MAX, M_MAX);
    for (unsigned int i = 1; i < model->mVertices.size(); i++) {
        if (model->mVertices[i].x() > vmax.x()) {
            vmax.setX(model->mVertices[i].x());
        }
        if (model->mVertices[i].y() > vmax.y()) {
            vmax.setY(model->mVertices[i].y());
        }
        if (model->mVertices[i].z() > vmax.z()) {
            vmax.setZ(model->mVertices[i].z());
        }
        if (model->mVertices[i].x() < vmin.x()) {
            vmin.setX(model->mVertices[i].x());
        }
        if (model->mVertices[i].y() < vmin.y()) {
            vmin.setY(model->mVertices[i].y());
        }
        if (model->mVertices[i].z() < vmin.z()) {
            vmin.setZ(model->mVertices[i].z());
        }
    }
    root->start = vmin;
    root->end = vmax;
    
    nodes.push_back(root);
    buildNode(0);
}

// depth-first search to build the tree
void AmKDTree::buildNode(int index)
{
    while (index != -1) {
        if (terminate(index)) {
            nodes[index]->leaf = true;
        } else {
            splitNode(index);
        }
        
        if (nodes[index]->leftChild != -1) {
            index = nodes[index]->leftChild;
            continue;
        } else if (nodes[index]->sibling != -1) {
            index = nodes[index]->sibling;
            continue;
        } else {
            while (nodes[index]->parent != -1) {
                index = nodes[index]->parent;
                if (nodes[index]->sibling != -1) {
                    index = nodes[index]->sibling;
                    break;
                }
            }
            if (nodes[index]->parent == -1) {
                index = -1;
            }
        }
    }
}

// check if need to terminate the splittion
bool AmKDTree::terminate(int index)
{
    if(nodes[index]->depth == 16)
	{// maximum depth of 16
		return true;
	}
	if(nodes[index]->meshes.size() <= 5)
    {// the number of meshes in the node is small enough
		return true;
	}
    return false;
}


// split the node
void AmKDTree::splitNode(int index)
{
    //first choose the plane to split
    findPlane(index);
    
    //get the index of the children
    nodes[index]->leftChild = static_cast<int>(nodes.size());
    nodes.push_back(AmKDTreeNodePtr(new AmKDTreeNode));
    nodes[index]->rightChild = static_cast<int>(nodes.size());
    nodes.push_back(AmKDTreeNodePtr(new AmKDTreeNode));
    
    AmKDTreeNodePtr thenode = nodes[index];
    AmKDTreeNodePtr left = nodes[thenode->leftChild];
    AmKDTreeNodePtr right = nodes[thenode->rightChild];
    
    left->sibling = thenode->rightChild;
    left->parent = right->parent = index;
    left->depth = right->depth = thenode->depth + 1;
	
	left->start = thenode->start;
    left->end = thenode->end;
    right->start = thenode->start;
    right->end = thenode->end;
	if(thenode->plane.axis == AmPlane::AM_X)
	{
        left->end.mData[0] = right->start.mData[0] = thenode->plane.value;
	} else if(thenode->plane.axis == AmPlane::AM_Y)
	{
        left->end.mData[1] = right->start.mData[1] = thenode->plane.value;
	} else {
        left->end.mData[2] = right->start.mData[2] = thenode->plane.value;
	}
    
	// split the triangles
	for(int i = 0; i < thenode->meshes.size(); i++)
	{// check the triangles in parent node
		int parentMesh = thenode->meshes[i];
        
		int position = meshInNode(parentMesh, thenode);
		if(position <= 0)
		{// in left child
			left->meshes.push_back(parentMesh);
		}
        if (position >= 0)
        {// in right child
			right->meshes.push_back(parentMesh);
		}
	}
}

// find the plane to split the node
//  first check if blank space is too large in one axis,
//  if not, choose the axis of the largest span to split
void AmKDTree::findPlane(int index)
{
    AmVec3f vmax(M_MIN, M_MIN, M_MIN);
    AmVec3f vmin(M_MAX, M_MAX, M_MAX);
    for (unsigned int i = 0; i < nodes[index]->meshes.size(); i++) {
        AmVec3f start = model->mTriangles[nodes[index]->meshes[i]].start;
        AmVec3f end = model->mTriangles[nodes[index]->meshes[i]].end;
        if (end.x() > vmax.x()) {
            vmax.setX(end.x());
        }
        if (end.y() > vmax.y()) {
            vmax.setY(end.y());
        }
        if (end.z() > vmax.z()) {
            vmax.setZ(end.z());
        }
        if (start.x() < vmin.x()) {
            vmin.setX(start.x());
        }
        if (start.y() < vmin.y()) {
            vmin.setY(start.y());
        }
        if (start.z() < vmin.z()) {
            vmin.setZ(start.z());
        }
    }
    float xspan = nodes[index]->end.x() - nodes[index]->start.x();
	if((vmin.x() - nodes[index]->start.x()) / xspan > 0.25)
	{// blank space of x axis from start is too much
		nodes[index]->plane.axis = AmPlane::AM_X;
		nodes[index]->plane.value = vmin.x();
		return;
	} else if((nodes[index]->end.x() - vmax.x()) / xspan > 0.25)
	{// blank space of x axis from end is too much
		nodes[index]->plane.axis = AmPlane::AM_X;
		nodes[index]->plane.value = vmax.x();
		return;
	}
    
	float yspan = nodes[index]->end.y() - nodes[index]->start.y();
	if((vmin.y() - nodes[index]->start.y()) / xspan > 0.25)
	{// blank space of y axis from start is too much
		nodes[index]->plane.axis = AmPlane::AM_Y;
		nodes[index]->plane.value = vmin.y();
		return;
	} else if((nodes[index]->end.y() - vmax.y()) / yspan > 0.25)
	{// blank space of y axis from end is too much
		nodes[index]->plane.axis = AmPlane::AM_Y;
		nodes[index]->plane.value = vmax.y();
		return;
	}
    
	float zspan = nodes[index]->end.z() - nodes[index]->start.z();
	if((vmin.z() - nodes[index]->start.z()) / zspan > 0.25)
	{// blank space of z axis from start is too much
		nodes[index]->plane.axis = AmPlane::AM_Z;
		nodes[index]->plane.value = vmin.z();
		return;
	} else if((nodes[index]->end.z() - vmax.z()) / zspan > 0.25)
	{// blank space of z axis from end is too much
		nodes[index]->plane.axis = AmPlane::AM_Z;
		nodes[index]->plane.value = vmax.z();
		return;
	}
    
	// find the spacial median of the longest axis
	if(xspan > yspan)
	{
		if(xspan > zspan)
			nodes[index]->plane.axis = AmPlane::AM_X;
		else
			nodes[index]->plane.axis = AmPlane::AM_Z;
	} else {
		if(yspan > zspan)
			nodes[index]->plane.axis = AmPlane::AM_Y;
		else
			nodes[index]->plane.axis = AmPlane::AM_Z;
	}
    
	if(nodes[index]->plane.axis == AmPlane::AM_X)
		nodes[index]->plane.value =
            (nodes[index]->start.x() + nodes[index]->end.x()) * 0.5;
	else if(nodes[index]->plane.axis == AmPlane::AM_Y)
		nodes[index]->plane.value =
            (nodes[index]->start.y() + nodes[index]->end.y()) * 0.5;
	else
		nodes[index]->plane.value =
            (nodes[index]->start.z() + nodes[index]->end.z()) * 0.5;
}

// check what node the mesh is in
// return -1 means left, 1 means right and 0 means both
int AmKDTree::meshInNode(int mesh, const AmKDTreeNodePtr &node)
{
    float value = node->plane.value;
    AmTriangle* m = &model->mTriangles[mesh];
	if(node->plane.axis == AmPlane::AM_X)
    {
		if(m->end.x() < value-EPSILON)
			return -1;
		else if(m->start.x() > value+EPSILON)
			return 1;
		else
			return 0;
	} else if(node->plane.axis == AmPlane::AM_Y)
	{
		if(m->end.y() < value-EPSILON)
			return -1;
		else if(m->start.y() > value+EPSILON)
			return 1;
		else
			return 0;
	} else {
		if(m->end.z() < value-EPSILON)
			return -1;
		else if(m->start.z() > value+EPSILON)
			return 1;
		else
			return 0;
	}
}



///// kd-tree traversal ///////

// search for intersection
float AmKDTree::search(const AmRay &ray, int &index)
{
    float hit = -1;
    index = -1;
    
    // check the box intersection
	float tmin, tmax;
	if(!hitBox(0, ray, tmin, tmax))
		return hit;
    
    
	vector<int> stack;
    vector<float> tstack;
    int node = 0;
	while(1)
	{
		if(nodes[node]->leaf)
		{// hit the leaf
			if(searchLeaf(node, ray, index, hit))
				return hit;
			else if(stack.size() > 0)
			{//push node from stack
				node = stack.back();stack.pop_back();
                tmax = tstack.back();tstack.pop_back();
                tmin = tstack.back();tstack.pop_back();
			}
			else
				return hit;
		} else {
			float tHit;
            
			int first, second;  // the order of hit children
            AmPlane::AmAxis axis = nodes[node]->plane.axis;
            
			if(axis == AmPlane::AM_X)
			{
				tHit = (nodes[node]->plane.value - ray.orig.x()) / ray.dir.x();
				if(ray.dir.x() > 0)
				{
					first = nodes[node]->leftChild;
					second = nodes[node]->rightChild;
				} else {
					second = nodes[node]->leftChild;
					first = nodes[node]->rightChild;
				}
			} else if(axis == AmPlane::AM_Y)
			{
				tHit = (nodes[node]->plane.value - ray.orig.y()) / ray.dir.y();
				if(ray.dir.y() > 0)
				{
					first = nodes[node]->leftChild;
					second = nodes[node]->rightChild;
				} else {
					second = nodes[node]->leftChild;
					first = nodes[node]->rightChild;
				}
			} else {
				tHit = (nodes[node]->plane.value - ray.orig.z()) / ray.dir.z();
				if(ray.dir.z() > 0)
				{
					first = nodes[node]->leftChild;
					second = nodes[node]->rightChild;
				}
				else
				{
					second = nodes[node]->leftChild;
					first = nodes[node]->rightChild;
				}
			}
            
			if(tHit > tmax+EPSILON)
				node = first;
			else if(tHit < tmin-EPSILON) //including the condition that tHit<0
				node = second;
			else
			{ // through both children, push the second to stack
                stack.push_back(second);
                tstack.push_back(tHit);
                tstack.push_back(tmax);
				tmax = tHit;
				node = first;
			}
		}
	}
	return hit;
}


// check if the ray hit the node
bool AmKDTree::hitBox(int index, const AmRay &ray, float &tmin, float &tmax)
{
    AmKDTreeNodePtr thenode = nodes[index];
    int init = 0;  //indicate whether tmin and tmax has been initialized
    
	// begin to check x axis
	if(abs(ray.dir.x()) < EPSILON)
	{// parallel to x axis
		if(ray.orig.x() < thenode->start.x()
           || ray.orig.x() > thenode->end.x())
			return false;		//no intersection
	} else {
		float t1 = (thenode->start.x() - ray.orig.x()) / ray.dir.x();
		float t2 = (thenode->end.x() - ray.orig.x()) / ray.dir.x();
		
		if(t1 > t2)
		{// swap
			float t = t1;
			t1 = t2;
			t2 = t;
		}
		
		if(t2 < 0)
			return false;	// the box is behind
		
		tmin = t1;
		tmax = t2;
		init = 1;
	}
	
	// begin to check y axis
	if(abs(ray.dir.y()) < EPSILON)
	{// parallel to y axis
		if(ray.orig.y() < thenode->start.y()
           || ray.orig.y() > thenode->end.y())
			return false;		//no intersection
	} else {
		float t1 = (thenode->start.y() - ray.orig.y()) / ray.dir.y();
		float t2 = (thenode->end.y() - ray.orig.y()) / ray.dir.y();
		
		if(t1 > t2)
		{// swap
			float t = t1;
			t1 = t2;
			t2 = t;
		}
        
		if(t2 < 0)
			return false;	// the box is behind
		
		if(init == 1)
		{
			if(tmin < t1) tmin = t1;
			if(tmax > t2) tmax = t2;
			if(tmax - tmin < -EPSILON) return false;
		} else {
			tmin = t1;
			tmax = t2;
			init = 1;
		}
	}
    
	// begin to check z axis
	if(abs(ray.dir.z()) < EPSILON)
	{// parallel to y axis
		if(ray.orig.z() < thenode->start.z()
           || ray.orig.z() > thenode->end.z())
			return false;		//no intersection
	} else {
		float t1 = (thenode->start.z() - ray.orig.z()) / ray.dir.z();
		float t2 = (thenode->end.z() - ray.orig.z()) / ray.dir.z();
        
		if(t1 > t2)
		{// swap
			float t = t1;
			t1 = t2;
			t2 = t;
		}
        
		if(t2 < 0)
			return false;	// the box is behind
        
		if(init == 1)
		{
			if(tmin < t1) tmin = t1;
			if(tmax > t2) tmax = t2;
			if(tmax - tmin < -EPSILON) return false;
		}
		else
		{
			tmin = t1;
			tmax = t2;
		}	
	}
	
	return true;
}

bool AmKDTree::searchLeaf(int node, const AmRay &ray, int &index, float &minHit)
{
    index = -1;
    minHit = -1;
	bool hitOrNot = false;
	for(int i = 0; i < nodes[node]->meshes.size(); i++)
	{
		int j = nodes[node]->meshes[i];
		unsigned int *vindices = model->mTriangles[j].vindices;
		float hit = AmRayTracer::hitMesh(ray, model->mVertices[vindices[0]],
								model->mVertices[vindices[1]],
								model->mVertices[vindices[2]]);
        
		if(hit > EPSILON)
		{
			if(!hitOrNot)
			{
				minHit = hit;
				index = j;
				hitOrNot = true;
			}
			else
			{
				if(hit < minHit)
				{
					minHit = hit;
					index = j;
				}
			}
		}
	}
    return hitOrNot;
}
