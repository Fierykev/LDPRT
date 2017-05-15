#include "LightProb.h"
#include "mat.h"
#include "vec.h"
#include "SphericalHarmonic.h"

float LightProb::intensity = 1.f;

const float speed = .1f;
const float intensityDelta = .1f;

void LightProb::deltaIntensity(float delta)
{
	intensity += delta;
}

float LightProb::getLightIntensity()
{
	return intensity;
}

Vec3f LightProb::getLightDir()
{
	Mat4f rotationX;
	rotationX = Mat4f::createRotation(angleX, 1, 0, 0);

	Mat4f rotationY;
	rotationY = Mat4f::createRotation(angleY, 0, 1, 0);

	Vec3f lightDir = { 0, 0, 1 };

	lightDir = rotationX * lightDir * rotationY;

	lightDir.normalize();

	return lightDir;
}

void LightProb::getHarmonics(Vec4f* lightR, Vec4f* lightG, Vec4f* lightB)
{
	Vec3f lightDir = getLightDir();

	// TODO: color lights LIKE A BOSS
	SphericalHarmonic::evaluateSphericalHarmonic6(lightDir, (float*)lightR);
	SphericalHarmonic::evaluateSphericalHarmonic6(lightDir, (float*)lightG);
	SphericalHarmonic::evaluateSphericalHarmonic6(lightDir, (float*)lightB);
}

void LightProb::getBackLightHarmonics(Vec4f* lightR, Vec4f* lightG, Vec4f* lightB)
{
	Vec3f lightDir = -getLightDir();

	// TODO: color lights LIKE A BOSS
	SphericalHarmonic::evaluateSphericalHarmonic6(lightDir, (float*)lightR);
	SphericalHarmonic::evaluateSphericalHarmonic6(lightDir, (float*)lightG);
	SphericalHarmonic::evaluateSphericalHarmonic6(lightDir, (float*)lightB);
}

void LightProb::draw()
{
	Vec3f lightDir = getLightDir();

	// Render a cube
	glPushMatrix();
	{
		glTranslatef(
			lightDir[0],
			lightDir[1],
			lightDir[2]
		);

		lightDir.normalize();

		glScalef(.1, .1, .1);

		glBegin(GL_QUADS);
		{
			// Top face
			glColor3f(0.0f, 1.0f, 0.0f);  // Green
			glVertex3f(1.0f, 1.0f, -1.0f);  // Top-right of top face
			glVertex3f(-1.0f, 1.0f, -1.0f);  // Top-left of top face
			glVertex3f(-1.0f, 1.0f, 1.0f);  // Bottom-left of top face
			glVertex3f(1.0f, 1.0f, 1.0f);  // Bottom-right of top face

										   // Bottom face
			glColor3f(1.0f, 0.5f, 0.0f); // Orange
			glVertex3f(1.0f, -1.0f, -1.0f); // Top-right of bottom face
			glVertex3f(-1.0f, -1.0f, -1.0f); // Top-left of bottom face
			glVertex3f(-1.0f, -1.0f, 1.0f); // Bottom-left of bottom face
			glVertex3f(1.0f, -1.0f, 1.0f); // Bottom-right of bottom face

										   // Front face
			glColor3f(1.0f, 0.0f, 0.0f);  // Red
			glVertex3f(1.0f, 1.0f, 1.0f);  // Top-Right of front face
			glVertex3f(-1.0f, 1.0f, 1.0f);  // Top-left of front face
			glVertex3f(-1.0f, -1.0f, 1.0f);  // Bottom-left of front face
			glVertex3f(1.0f, -1.0f, 1.0f);  // Bottom-right of front face

											// Back face
			glColor3f(1.0f, 1.0f, 0.0f); // Yellow
			glVertex3f(1.0f, -1.0f, -1.0f); // Bottom-Left of back face
			glVertex3f(-1.0f, -1.0f, -1.0f); // Bottom-Right of back face
			glVertex3f(-1.0f, 1.0f, -1.0f); // Top-Right of back face
			glVertex3f(1.0f, 1.0f, -1.0f); // Top-Left of back face

										   // Left face
			glColor3f(0.0f, 0.0f, 1.0f);  // Blue
			glVertex3f(-1.0f, 1.0f, 1.0f);  // Top-Right of left face
			glVertex3f(-1.0f, 1.0f, -1.0f);  // Top-Left of left face
			glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom-Left of left face
			glVertex3f(-1.0f, -1.0f, 1.0f);  // Bottom-Right of left face

											 // Right face
			glColor3f(1.0f, 0.0f, 1.0f);  // Violet
			glVertex3f(1.0f, 1.0f, 1.0f);  // Top-Right of left face
			glVertex3f(1.0f, 1.0f, -1.0f);  // Top-Left of left face
			glVertex3f(1.0f, -1.0f, -1.0f);  // Bottom-Left of left face
			glVertex3f(1.0f, -1.0f, 1.0f);  // Bottom-Right of left face
		}
		glEnd();
	}
	glPopMatrix();
}

void LightProb::processKeys(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		angleX += speed;
		break;
	case 'a':
		angleY += speed;
		break;
	case 's':
		angleX -= speed;
		break;
	case 'd':
		angleY -= speed;
		break;
	case 'u':
		deltaIntensity(intensityDelta);
		break;
	case 'j':
		deltaIntensity(-intensityDelta);
		break;
	}
}