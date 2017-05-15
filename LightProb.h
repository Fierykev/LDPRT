#ifndef __LIGHT_PROB_H__
#define __LIGHT_PROB_H__

#include "PRT.h"

#define LIGHT_PROB_GRPS NUM_BANDS * NUM_BANDS >> 2

class LightProb
{
public:
	static void deltaIntensity(float delta);
	static float getLightIntensity();
	static Vec3f getLightDir();
	static void getHarmonics(Vec4f* lightR, Vec4f* lightG, Vec4f* lightB);
	static void getBackLightHarmonics(Vec4f* lightR, Vec4f* lightG, Vec4f* lightB);
	static void draw();
	static void processKeys(unsigned char key, int x, int y);

private:
	static float intensity;
};

#endif