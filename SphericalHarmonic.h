#ifndef __SPHERICAL_HARMONIC_H__
#define __SPHERICAL_HARMONIC_H__

#include "vec.h"
#include "mat.h"

class SphericalHarmonic
{
public:
	static void evaluateSphericalHarmonic6(Vec3f dir, float* output);
	static void genConstants();

	static void evaluateSphericalHarmonicGradient6NUMERIC(Vec3f dir, Vec3f* output);
	static void evaluateSphericalHarmonicGradient6(Vec3f dir, Vec3f* output);

	static void createJacobian(Vec3f coord, Mat3f* output);

private:

	// constants for spherical harmonic 6
	static float c[21][2];
};

#endif