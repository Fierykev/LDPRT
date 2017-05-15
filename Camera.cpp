#include "Camera.h"
#include "ShaderHelper.h"

#include <GL/glut.h>
#include <iostream>

Camera::Camera()
{
	reset();
}

void Camera::reset()
{
	world = Mat4f();
	view = Mat4f();
	projection = Mat4f();
}

void Camera::apply()
{
	gluLookAt(
		eye[0], eye[1], eye[2],
		at[0], at[1], at[2],
		0, 1, 0
	);

	Mat4f viewTmp = view;
	viewTmp.transpose();
	glMultMatrixf(&viewTmp[0][0]);
}

Vec3f Camera::getCameraLoc()
{
	Mat4f modelView =
		glGetMatrix(GL_MODELVIEW_MATRIX);

	// plane normals
	Vec3f norms[3];
	norms[0] = Vec3f(modelView[0][0], modelView[0][1], modelView[0][2]);
	norms[1] = Vec3f(modelView[1][0], modelView[1][1], modelView[1][2]);
	norms[2] = Vec3f(modelView[2][0], modelView[2][1], modelView[2][2]);

	// intersections
	Vec3f inters[3];
	inters[0] = norms[1] ^ norms[2];
	inters[1] = norms[2] ^ norms[0];
	inters[2] = norms[0] ^ norms[1];

	// distance
	Vec3f distance = Vec3f(
		modelView[0][3],
		modelView[1][3],
		modelView[2][3]
	);

	// top
	Vec3f top = inters[0] * distance[0]
		+ inters[1] * distance[1]
		+ inters[2] * distance[2];

	return top / -(norms[0] * inters[0]);
}

void Camera::processSpecialKeys(int key, int x, int y)
{
	Mat4f rot;

	switch (key)
	{
	case GLUT_KEY_DOWN:
		rot = Mat4f::createRotation(step, 1, 0, 0);

		break;
	case GLUT_KEY_UP:
		rot = Mat4f::createRotation(-step, 1, 0, 0);

		break;
	case GLUT_KEY_LEFT:
		rot = Mat4f::createRotation(-step, 0, 1, 0);

		break;
	case GLUT_KEY_RIGHT:
		rot = Mat4f::createRotation(step, 0, 1, 0);
		break;
	}

	view = view * rot;
}

void Camera::processKeys(int key, int x, int y)
{
	switch (key)
	{
	case 'i':
		eye[2] -= camStep;

		break;
	case 'k':
		eye[2] += camStep;

		break;
	}
}