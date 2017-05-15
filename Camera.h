#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "mat.h"
#include "vec.h"

class Camera
{
public:
	Camera();

	void apply();
	void reset();

	static Vec3f getCameraLoc();

	void processSpecialKeys(int key, int x, int y);

	void processKeys(int key, int x, int y);

	void setEye(Vec3f eye)
	{
		this->eye = eye;
	}

	void setAt(Vec3f at)
	{
		this->at = at;
	}

private:
	const float step = .1f;
	const float camStep = .1f;

	Mat4f world, view, projection;
	Vec3f eye, at;
};

#endif