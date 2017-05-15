#ifndef HELPER_H
#define HELPER_H

#include "vec.h"

inline bool operator== (const Vec2f &a, const Vec2f &b)
{
	return a[0] == b[0] && a[1] == b[1];
}

inline bool operator== (const Vec3f &a, const Vec3f &b)
{
	return a[0] == b[0] && a[1] == b[1] && a[2] == a[2];
}

inline bool operator== (const Vec4f &a, const Vec4f &b)
{
	return a[0] == b[0] && a[1] == b[1] && a[2] == a[2] && a[3] == b[3];
}

struct Vec3Compare
{
	bool operator() (const Vec3f &a, const Vec3f &b)
	{
		if (a[0] < b[0])
			return true;
		else if (a[0] > b[0])
			return false;
		// must be equal check y value

		if (a[1] < b[1])
			return true;
		else if (a[1] > b[1])
			return false;
		// must be equal check z value
		if (a[2] < b[2])
			return true;

		return false;
	}
};

#endif