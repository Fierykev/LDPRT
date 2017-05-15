#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include "vec.h"

struct Ray
{
	Vec3f origin;
	Vec3f direction;

	Vec3f invDirection;
};

struct isect
{
	double t;
	Vec2f uvCoordinates;
	Vec3f bary;
	Vec3f normal;
	size_t index[3];
	Vec3f color;
};

class Geometry
{
public:
	virtual Vec3f getBoundingBoxMin() = 0;
	virtual Vec3f getBoundingBoxMax() = 0;

	virtual Vec3f getMinBB(size_t index) {
		return getBoundingBoxMin();
	}

	virtual Vec3f getMaxBB(size_t index) {
		return getBoundingBoxMax();
	}

	virtual Vec3f getCentroid(size_t index) {
		return (getMinBB(index) + getMaxBB(index)) / 2.f;
	}

	virtual size_t numberOfObjects() = 0;

	virtual size_t numberofVerts() = 0;

	virtual bool intersectLocal(Ray ray, isect& isect, size_t index) = 0;

	virtual Vec3f getNormal(size_t vertID) = 0;

	virtual Vec3f getPosition(size_t vertID) = 0;

	virtual Vec3f getColor(size_t vertID) = 0;
};

#endif