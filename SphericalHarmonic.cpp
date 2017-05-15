#define _USE_MATH_DEFINES

#include <math.h>

#include "SphericalHarmonic.h"

// TMP
#include <iostream>

using namespace std;

float SphericalHarmonic::c[21][2];

// constants derived from https://en.wikipedia.org/wiki/Table_of_spherical_harmonics#l_.3D_5.5B1.5D
// I did it on several napkins from Kin's market... so I can't exactly attach it
void SphericalHarmonic::genConstants()
{
	// m = 0

	c[0][0] = sqrt(1.f / M_PI) / 2.f;
	c[0][1] = 0.f;

	c[1][0] = sqrt(3.f / M_PI) / 2.f;
	c[1][1] = 0.f;

	c[2][0] = sqrt(5.f / M_PI) * 3.f / 4.f;
	c[2][1] = -sqrt(5.f / M_PI) / 4.f;

	c[3][0] = sqrt(7.f / M_PI) / 4.f * 5.f;
	c[3][1] = -sqrt(7.f / M_PI) / 4.f * 3.f;

	c[4][0] = 4.f * sqrt(M_PI / 7.f) / 5.f * 3.f / 16.f * sqrt(1.f / M_PI) * 35.f;
	c[4][1] = -4.f * sqrt(M_PI / 5.f) * 3.f / 16.f * sqrt(1.f / M_PI) * 3.f;

	c[5][0] = 16.f / 3.f * sqrt(M_PI) / 35.f * 1.f / 16.f * sqrt(11.f / M_PI) * 63.f;
	c[5][1] = -4.f * sqrt(M_PI / 7.f) * 1.f / 16.f * sqrt(11.f / M_PI) * 3.2f;

	// m = 1

	c[6][0] = -c[1][0];
	c[6][1] = 0.f;

	c[7][0] = -1.f / 2.f * sqrt(15.f / M_PI);
	c[7][1] = 0.f;

	c[8][0] = -1.f / 8.f * sqrt(21.f * 2.f / M_PI) * 5.f;
	c[8][1] = 1.f / 8.f * sqrt(21.f * 2.f / M_PI);

	c[9][0] = -3.f / 8.f * sqrt(5.f * 2.f / M_PI) * 7.f;
	c[9][1] = 3.f / 8.f * sqrt(5.f * 2.f / M_PI) * 3.f;

	c[10][0] = 8.f / 3.f * sqrt(M_PI / 10.f) / 7.f * 1.f / 16.f * sqrt(165.f / M_PI) * 21.f;
	c[10][1] = -8.f * sqrt(M_PI / 42.f) * 1.f / 16.f * sqrt(165.f / M_PI);

	// m = 2

	c[11][0] = 1.f / 4.f * sqrt(15.f / M_PI);
	c[11][1] = 0.f;

	c[12][0] = 1.f / 4.f * sqrt(105.f / M_PI);
	c[12][1] = 0.f;

	c[13][0] = 3.f / 8.f * sqrt(5 / M_PI) * 7.f;
	c[13][1] = -3.f / 8.f * sqrt(5 / M_PI);

	c[14][0] = 1.f / 8.f * sqrt(1155.f / M_PI) * 3.f;
	c[14][1] = -1.f / 8.f * sqrt(1155.f / M_PI);

	// m = 3

	c[15][0] = -1.f / 8.f * sqrt(35.f * 2.f / M_PI);
	c[15][1] = 0.f;

	c[16][0] = -3.f / 8.f * sqrt(35.f * 2.f / M_PI);
	c[16][1] = 0.f;

	c[17][0] = -1.f / 32.f * sqrt(385.f * 2.f / M_PI) * 9.f;
	c[17][1] = 1.f / 32.f * sqrt(385.f * 2.f / M_PI);

	// m = 4

	c[18][0] = 3.f / 16.f * sqrt(35.f / M_PI);
	c[18][1] = 0.f;

	c[19][0] = 3.f / 16.f * sqrt(385.f / M_PI);
	c[19][1] = 0.f;

	// m = 5

	c[20][0] = -3.f / 32.f * sqrt(77.f * 2.f / M_PI);
	c[20][1] = 0.f;
}

// based on https://en.wikipedia.org/wiki/Table_of_spherical_harmonics#l_.3D_5.5B1.5D
void SphericalHarmonic::evaluateSphericalHarmonic6(Vec3f dir, float* output)
{
	float x = dir[0], y = dir[1], z = dir[2];
	float zSq = z * z;

	/************ l=?, m=0 ************/

	// l=0, m=0
	output[0] = c[0][0];

	// l= 1, m=0
	output[2] = z * c[1][0];
	
	// l=2, m=0
	output[6] = c[2][0] * zSq + c[2][1];
	
	// l=3, m=0
	output[12] = z * (c[3][0] * zSq + c[3][1]);

	// l=4, m=0
	output[20] = c[4][0] * output[12] * z + c[4][1] * output[6];

	// l=5, m=0
	output[30] = c[5][0] * output[20] * z + c[4][1] * output[12];
	
	/************ l=?, m=+/-1 ************/

	// l=1, m=+/-1
	output[1] = y * c[6][0];
	output[3] = x * c[6][0];

	// l=2, m=+/-1
	output[5] = y * c[7][0] * z;
	output[7] = x * c[7][0] * z;

	// l=3, m=+/-1
	float t3_1 = c[8][0] * zSq + c[8][1];
	output[11] = y * t3_1;
	output[13] = x * t3_1;

	// l=4, m=+/-1
	float t4_1 = z * (c[9][0] * zSq + c[9][1]);
	output[19] = y * t4_1;
	output[21] = x * t4_1;

	// l=5, m=+/-1
	float t5_1 = c[10][0] * t4_1 * z + c[10][1] * t3_1;
	output[29] = y * t5_1;
	output[31] = x * t5_1;

	/************ l=?, m=+/-2 ************/

	float in_2_0 = 2.f * x * y;
	float in_2_1 = x * x - y * y;

	// l=2, m=+/-2
	output[4] = in_2_0 * c[11][0];
	output[8] = in_2_1 * c[11][0];

	// l=3, m=+/-2
	output[10] = in_2_0 * c[12][0] * z;
	output[14] = in_2_1 * c[12][0] * z;

	// l=4, m=+/-2
	float t4_2 = c[13][0] * zSq + c[13][1];
	output[18] = in_2_0 * t4_2;
	output[22] = in_2_1 * t4_2;

	// l=5, m=+/-2
	float t5_2 = z * (c[14][0] * zSq + c[14][1]);
	output[28] = in_2_0 * t5_2;
	output[32] = in_2_1 * t5_2;

	/************ l=?, m=+/-3 ************/

	float in_3_0 = x * in_2_0 + y * in_2_1;
	float in_3_1 = x * in_2_1 - y * in_2_0;

	// l=3, m=+/-3
	output[9] = in_3_0 * c[15][0];
	output[15] = in_3_1 * c[15][0];

	// l=4, m=+/-3
	output[17] = in_3_0 * c[16][0] * z;
	output[23] = in_3_1 * c[16][0] * z;

	// l=5, m=+/-3
	float t5_3 = c[17][0] * zSq + c[17][1];
	output[27] = in_3_0 * t5_3;
	output[33] = in_3_1 * t5_3;

	/************ l=?, m=+/-4 ************/

	float in_4_0 = x * in_3_0 + y * in_3_1;
	float in_4_1 = x * in_3_1 - y * in_3_0;

	// l=4, m=+/-4
	output[16] = in_4_0 * c[18][0];
	output[24] = in_4_1 * c[18][0];

	// l=5, m=+/-4
	output[26] = in_4_0 * c[19][0] * z;
	output[34] = in_4_1 * c[19][0] * z;

	/************ l=?, m=+/-5 ************/

	float in_5_0 = x * in_4_0 + y * in_4_1;
	float in_5_1 = x * in_4_1 - y * in_4_0;

	// l=5, m=+/-5
	output[25] = in_5_0 * c[20][0];
	output[35] = in_5_1 * c[20][0];
}

void SphericalHarmonic::evaluateSphericalHarmonicGradient6NUMERIC(Vec3f dir, Vec3f* output)
{
	float e = .0001;

	float tmp[2][36];

	Vec3f tmpDir;

	for (int i = 0; i < 3; i++)
	{
		tmpDir = dir;
		tmpDir[i] -= e;

		tmpDir.normalize();

		evaluateSphericalHarmonic6(dir, tmp[0]);

		evaluateSphericalHarmonic6(tmpDir, tmp[1]);

		for (int j = 0; j < 36; j++)
		{
			output[j][i] = (tmp[0][j] - tmp[1][j]) / e;
		}
	}
}

// return gradient of spherical harmonic function
void SphericalHarmonic::evaluateSphericalHarmonicGradient6(Vec3f dir, Vec3f* output)
{
	float x = dir[0], y = dir[1], z = dir[2];

	if (x == 0 && y == 0 || x == 0 && z == 0 || y == 0 && z == 0)
	{
		Vec3f eVec = { x, y, z };
		eVec += Vec3f(.0001, .0001, .0001);
		printf("CORRECTION\n");
		eVec.normalize();

		x = eVec[0];
		y = eVec[1];
		z = eVec[2];
	}

	float zSq = z * z, ySq = y * y, xSq = x * x;
	float zCu = z * z * z;

	/************ l=?, m=0. ************/

	// l=0, m=0
	output[0] = Vec3f(0, 0, 0);
	
	// l= 1, m=0
	output[2] = Vec3f(
		-(sqrt(3.f / M_PI)*x*z) / (2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 1.5f)),
		-(sqrt(3.f / M_PI)*y*z) / (2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 1.5f)),
		(sqrt(3.f / M_PI)*(pow(x, 2) + pow(y, 2))) /
		(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 1.5f))
	);

	// l= 2, m=0
	output[6] = Vec3f(
		(-3.f * sqrt(5.f / M_PI)*x*pow(z, 2)) /
		(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2)),
		(-3.f * sqrt(5.f / M_PI)*y*pow(z, 2)) /
		(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2)),
		(3.f * sqrt(5.f / M_PI)*(pow(x, 2) + pow(y, 2))*z) /
		(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2))
	);

	// l= 3, m=0
	output[12] = Vec3f(
		(3.f * sqrt(7.f / M_PI)*x*z*(pow(x, 2) + pow(y, 2) - 4.f * pow(z, 2))) /
		(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5f)),
		(3.f * sqrt(7.f / M_PI)*y*z*(pow(x, 2) + pow(y, 2) - 4.f * pow(z, 2))) /
		(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5f)),
		(-3.f * sqrt(7.f / M_PI)*(pow(x, 2) + pow(y, 2))*
		(pow(x, 2) + pow(y, 2) - 4.f * pow(z, 2))) /
			(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5f))
	);

	// l= 4, m=0
	output[20] = Vec3f(
		(15.f * x*pow(z, 2)*(3.f * (pow(x, 2) + pow(y, 2)) - 4.f * pow(z, 2))) /
		(4.f*sqrt(M_PI)*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
		(15.f * y*pow(z, 2)*(3.f * (pow(x, 2) + pow(y, 2)) - 4.f * pow(z, 2))) /
		(4.f*sqrt(M_PI)*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
		(15.f * (pow(x, 2) + pow(y, 2))*z*
		(-3.f * (pow(x, 2) + pow(y, 2)) + 4.f * pow(z, 2))) /
			(4.f*sqrt(M_PI)*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3))
	);

	// l= 5, m=0
	output[30] = Vec3f(
		(-15.f * sqrt(11.f / M_PI)*x*z*(pow(pow(x, 2) + pow(y, 2), 2) -
			12 * (pow(x, 2) + pow(y, 2))*pow(z, 2) + 8.f * pow(z, 4))) /
			(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5f)),
		(-15.f * sqrt(11.f / M_PI)*y*z*(pow(pow(x, 2) + pow(y, 2), 2) -
			12 * (pow(x, 2) + pow(y, 2))*pow(z, 2) + 8.f * pow(z, 4))) /
			(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5f)),
		(15.f * sqrt(11.f / M_PI)*(pow(x, 2) + pow(y, 2))*
		(pow(pow(x, 2) + pow(y, 2), 2) -
			12 * (pow(x, 2) + pow(y, 2))*pow(z, 2) + 8.f * pow(z, 4))) /
			(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5f))
	);

	/************ l=?, m=+/-1.f ************/

	// l=1, m=+/-1
	output[1] = Vec3f(
		(sqrt(3.f / M_PI)*x*y*pow((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)), 1.5)) /
			(2.f*pow(pow(x, 2) + pow(y, 2), 1.5)),
		-(sqrt(3.f / M_PI)*(pow(x, 2) + pow(z, 2))*
			pow((pow(x, 2) + pow(y, 2)) /
			(pow(x, 2) + pow(y, 2) + pow(z, 2)), 1.5)) /
				(2.f*pow(pow(x, 2) + pow(y, 2), 1.5)),
		(sqrt(3.f / M_PI)*y*z*pow((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)), 1.5)) /
			(2.f*pow(pow(x, 2) + pow(y, 2), 1.5))
	);
	output[3] = Vec3f(
		-(sqrt(3.f / M_PI)*(pow(y, 2) + pow(z, 2))*
			pow((pow(x, 2) + pow(y, 2)) /
			(pow(x, 2) + pow(y, 2) + pow(z, 2)), 1.5f)) /
				(2.f*pow(pow(x, 2) + pow(y, 2), 1.5f)),
		(sqrt(3.f / M_PI)*x*y*pow((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)), 1.5f)) /
			(2.f*pow(pow(x, 2) + pow(y, 2), 1.5f)),
		(sqrt(3.f / M_PI)*x*z*pow((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)), 1.5f)) /
			(2.f*pow(pow(x, 2) + pow(y, 2), 1.5f))
	);

	// l=2, m=+/-1
	output[5] = Vec3f(
		(sqrt(15.f / M_PI)*x*y*sqrt(pow(x, 2) + pow(y, 2))*z) /
		(sqrt((pow(x, 2) + pow(y, 2)) / (pow(x, 2) + pow(y, 2) + pow(z, 2)))*
			pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5)),
		-(sqrt(15.f / M_PI)*sqrt(pow(x, 2) + pow(y, 2))*z*
		(pow(x, 2) - pow(y, 2) + pow(z, 2))) /
			(2.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5)),
		-(sqrt(15.f / M_PI)*y*sqrt(pow(x, 2) + pow(y, 2))*
		(pow(x, 2) + pow(y, 2) - pow(z, 2))) /
			(2.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5))
	);
	output[7] = Vec3f(
		-(sqrt(15.f / M_PI)*sqrt(pow(x, 2) + pow(y, 2))*z*
		(-pow(x, 2) + pow(y, 2) + pow(z, 2))) /
			(2.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5f)),
				(sqrt(15.f / M_PI)*x*y*sqrt(pow(x, 2) + pow(y, 2))*z) /
		(sqrt((pow(x, 2) + pow(y, 2)) / (pow(x, 2) + pow(y, 2) + pow(z, 2)))*
			pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5f)),
		-(sqrt(15.f / M_PI)*x*sqrt(pow(x, 2) + pow(y, 2))*
		(pow(x, 2) + pow(y, 2) - pow(z, 2))) /
			(2.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5f))
	);

	// l=3, m=+/-1
	output[11] = Vec3f(
		-(sqrt(21.f / (2.*M_PI))*x*y*sqrt(pow(x, 2) + pow(y, 2))*
		(pow(x, 2) + pow(y, 2) - 14.f * pow(z, 2))) /
			(4.*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
				(sqrt(21.f / (2.*M_PI))*sqrt(pow(x, 2) + pow(y, 2))*
		(pow(x, 4) + 11.f * pow(y, 2)*pow(z, 2) - 4.f * pow(z, 4) +
			pow(x, 2)*(pow(y, 2) - 3 * pow(z, 2)))) /
			(4.*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
				(sqrt(21.f / (2.*M_PI))*y*sqrt(pow(x, 2) + pow(y, 2))*z*
		(-11.f * (pow(x, 2) + pow(y, 2)) + 4.f * pow(z, 2))) /
					(4.*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
						pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3))
	);
	output[13] = Vec3f(
		(sqrt(21.f / (2.f*M_PI))*sqrt(pow(x, 2) + pow(y, 2))*
		(pow(y, 4) - 3.f * pow(y, 2)*pow(z, 2) - 4.f * pow(z, 4) +
			pow(x, 2)*(pow(y, 2) + 11.f * pow(z, 2)))) /
			(4.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
		-(sqrt(21.f / (2.f*M_PI))*x*y*sqrt(pow(x, 2) + pow(y, 2))*
		(pow(x, 2) + pow(y, 2) - 14.f * pow(z, 2))) /
			(4.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
				(sqrt(21.f / (2.f*M_PI))*x*sqrt(pow(x, 2) + pow(y, 2))*z*
		(-11.f * (pow(x, 2) + pow(y, 2)) + 4.f * pow(z, 2))) /
					(4.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
						pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3))
	);

	// l=4, m=+/-1
	output[19] = Vec3f(
		(-3.f * sqrt(5.f / (2.f*M_PI))*x*y*sqrt(pow(x, 2) + pow(y, 2))*z*
		(3.f * (pow(x, 2) + pow(y, 2)) - 11 * pow(z, 2))) /
			(2.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5)),
				(-3.f * sqrt(5.f / (2.f*M_PI))*sqrt(pow(x, 2) + pow(y, 2))*z*
		(-3.f * pow(x, 4) + 3.f * pow(y, 4) +
					(pow(x, 2) - 21 * pow(y, 2))*pow(z, 2) + 4.f * pow(z, 4))) /
			(4.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5)),
				(3.f * sqrt(5.f / (2.f*M_PI))*y*sqrt(pow(x, 2) + pow(y, 2))*
		(3.f * pow(pow(x, 2) + pow(y, 2), 2) -
			21 * (pow(x, 2) + pow(y, 2))*pow(z, 2) + 4.f * pow(z, 4))) /
			(4.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5))
	);
	output[21] = Vec3f(
		(-3.f * sqrt(5.f / (2.f*M_PI))*sqrt(pow(x, 2) + pow(y, 2))*z*
		(3.f * pow(x, 4) - 3.f * pow(y, 4) - 21.f * pow(x, 2)*pow(z, 2) +
			pow(y, 2)*pow(z, 2) + 4.f * pow(z, 4))) /
			(4.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5f)),
				(-3.f * sqrt(5.f / (2.f*M_PI))*x*y*sqrt(pow(x, 2) + pow(y, 2))*z*
		(3.f * (pow(x, 2) + pow(y, 2)) - 11.f * pow(z, 2))) /
					(2.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
						pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5f)),
						(3.f * sqrt(5.f / (2.f*M_PI))*x*sqrt(pow(x, 2) + pow(y, 2))*
		(3.f * pow(pow(x, 2) + pow(y, 2), 2) -
			21.f * (pow(x, 2) + pow(y, 2))*pow(z, 2) + 4.f * pow(z, 4))) /
			(4.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5f))
	);

	// l=5, m=+/-1
	output[29] = Vec3f(
		(sqrt(165.f / M_PI)*x*y*sqrt(pow(x, 2) + pow(y, 2))*
		(pow(pow(x, 2) + pow(y, 2), 2) -
			40.f * (pow(x, 2) + pow(y, 2))*pow(z, 2) + 64.f * pow(z, 4))) /
			(16.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 4)),
				(sqrt(165.f / M_PI)*sqrt(pow(x, 2) + pow(y, 2))*
		(-(pow(x, 2)*pow(pow(x, 2) + pow(y, 2), 2)) +
					(11.f * pow(x, 2) - 29.f * pow(y, 2))*(pow(x, 2) + pow(y, 2))*
			pow(z, 2) + 4 * (pow(x, 2) + 17 * pow(y, 2))*pow(z, 4) -
			8.f * pow(z, 6))) /
			(16.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 4)),
				(sqrt(165.f / M_PI)*y*sqrt(pow(x, 2) + pow(y, 2))*z*
		(29.f * pow(pow(x, 2) + pow(y, 2), 2) -
			68.f * (pow(x, 2) + pow(y, 2))*pow(z, 2) + 8.f * pow(z, 4))) /
			(16.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 4))
	);
	output[31] = Vec3f(
		-(sqrt(165.f / M_PI)*sqrt(pow(x, 2) + pow(y, 2))*
		(pow(y, 2)*pow(pow(x, 2) + pow(y, 2), 2) +
			(29.f * pow(x, 2) - 11.f * pow(y, 2))*(pow(x, 2) + pow(y, 2))*
			pow(z, 2) - 4.f * (17.f * pow(x, 2) + pow(y, 2))*pow(z, 4) +
			8.f * pow(z, 6))) /
			(16.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 4)),
				(sqrt(165.f / M_PI)*x*y*sqrt(pow(x, 2) + pow(y, 2))*
		(pow(pow(x, 2) + pow(y, 2), 2) -
			40.f * (pow(x, 2) + pow(y, 2))*pow(z, 2) + 64.f * pow(z, 4))) /
			(16.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 4)),
				(sqrt(165.f / M_PI)*x*sqrt(pow(x, 2) + pow(y, 2))*z*
		(29.f * pow(pow(x, 2) + pow(y, 2), 2) -
			68.f * (pow(x, 2) + pow(y, 2))*pow(z, 2) + 8.f * pow(z, 4))) /
			(16.f*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 4))
	);

	/************ l=?, m=+/-2 ************/

	// l=2, m=+/-2
	output[4] = Vec3f(
		(sqrt(15.f / M_PI)*y*(-pow(x, 2) + pow(y, 2) + pow(z, 2))) /
		(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2)),
		(sqrt(15.f / M_PI)*x*(pow(x, 2) - pow(y, 2) + pow(z, 2))) /
		(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2)),
		-((sqrt(15.f / M_PI)*x*y*z) / pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2))
	);
	output[8] = Vec3f(
		(sqrt(15.f / M_PI)*x*(2 * pow(y, 2) + pow(z, 2))) /
		(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2)),
		-(sqrt(15.f / M_PI)*y*(2 * pow(x, 2) + pow(z, 2))) /
		(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2)),
		(sqrt(15.f / M_PI)*(-pow(x, 2) + pow(y, 2))*z) /
		(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2))
	);

	// l=3, m=+/-2
	output[10] = Vec3f(
		(sqrt(105.f / M_PI)*y*z*(-2.f*pow(x, 2) + pow(y, 2) + pow(z, 2))) /
		(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5)),
		(sqrt(105.f / M_PI)*x*z*(pow(x, 2) - 2.f*pow(y, 2) + pow(z, 2))) /
		(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5)),
		(sqrt(105.f / M_PI)*x*y*(pow(x, 2) + pow(y, 2) - 2.f*pow(z, 2))) /
		(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5))
	);
	output[14] = Vec3f(
		-(sqrt(105.f / M_PI)*x*z*(pow(x, 2) - 5.f * pow(y, 2) - 2 * pow(z, 2))) /
		(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5f)),
		(sqrt(105.f / M_PI)*y*z*(-5.f * pow(x, 2) + pow(y, 2) - 2 * pow(z, 2))) /
		(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5f)),
		(sqrt(105.f / M_PI)*(x - y)*(x + y)*(pow(x, 2) + pow(y, 2) - 2 * pow(z, 2))) /
		(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5f))
	);

	// l=4, m=+/-2
	output[18] = Vec3f(
		(3.f*sqrt(5.f / M_PI)*y*(pow(x, 4) - pow(y, 4) - 21.f * pow(x, 2)*pow(z, 2) +
			5.f*pow(y, 2)*pow(z, 2) + 6.f * pow(z, 4))) /
			(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
		(3.f*sqrt(5.f / M_PI)*x*(-pow(x, 4) + pow(y, 4) +
		(5.f*pow(x, 2) - 21.f * pow(y, 2))*pow(z, 2) + 6.f * pow(z, 4))) /
			(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
		(3.f*sqrt(5.f / M_PI)*x*y*z*(4.f * (pow(x, 2) + pow(y, 2)) - 3.f*pow(z, 2))) /
		pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)
	);
	output[22] = Vec3f(
		(-3.f * sqrt(5.f / M_PI)*x*(pow(y, 4) - 9.f * pow(y, 2)*pow(z, 2) -
			3.f * pow(z, 4) + pow(x, 2)*(pow(y, 2) + 4.f * pow(z, 2)))) /
			(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
		(3.f * sqrt(5.f / M_PI)*y*(pow(x, 4) + 4.f * pow(y, 2)*pow(z, 2) - 3.f * pow(z, 4) +
			pow(x, 2)*(pow(y, 2) - 9.f * pow(z, 2)))) /
			(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
		(3.f * sqrt(5.f / M_PI)*(x - y)*(x + y)*z*
		(4.f * (pow(x, 2) + pow(y, 2)) - 3.f * pow(z, 2))) /
			(2.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3))
	);

	// l=5, m=+/-2
	output[28] = Vec3f(
		(sqrt(1155.f / M_PI)*y*z*(2.f*pow(x, 4) - pow(y, 4) +
			pow(y, 2)*pow(z, 2) + 2.f*pow(z, 4) +
			pow(x, 2)*(pow(y, 2) - 11 * pow(z, 2)))) /
			(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5)),
		(sqrt(1155.f / M_PI)*x*z*(-pow(x, 4) + 2.f*pow(y, 4) -
			11 * pow(y, 2)*pow(z, 2) + 2.f*pow(z, 4) +
			pow(x, 2)*(pow(y, 2) + pow(z, 2)))) /
			(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5)),
		-(sqrt(1155.f / M_PI)*x*y*(pow(pow(x, 2) + pow(y, 2), 2) -
			10.f*(pow(x, 2) + pow(y, 2))*pow(z, 2) + 4.f*pow(z, 4))) /
			(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5))
	);
	output[32] = Vec3f(
		(sqrt(1155.f / M_PI)*x*z*(pow(x, 4) - 5.f * pow(y, 4) +
			14.f * pow(y, 2)*pow(z, 2) + 4.f * pow(z, 4) -
			2 * pow(x, 2)*(2 * pow(y, 2) + 5.f * pow(z, 2)))) /
			(8.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5f)),
		-(sqrt(1155.f / M_PI)*y*z*(-5.f * pow(x, 4) - 4.f * pow(x, 2)*pow(y, 2) +
			pow(y, 4) + 2 * (7.f * pow(x, 2) - 5.f * pow(y, 2))*pow(z, 2) +
			4.f * pow(z, 4))) / (8.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5f))
		, -(sqrt(1155.f / M_PI)*(x - y)*(x + y)*
		(pow(pow(x, 2) + pow(y, 2), 2) -
			10.f * (pow(x, 2) + pow(y, 2))*pow(z, 2) + 4.f * pow(z, 4))) /
			(8.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5f))
	);

	/************ l=?, m=+/-3.f ************/
	// l=3, m=+/-3
	output[9] = Vec3f(
		(3.f*sqrt(35.f / (2.f*M_PI))*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
			(y*(pow(x, 2) + pow(y, 2) + pow(z, 2))*cos(3.f*atan2(y, x)) -
				x*pow(z, 2)*sin(3.f*atan2(y, x)))) /
				(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2)),
		(-3.f*sqrt(35.f / (2.f*M_PI))*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
			(x*(pow(x, 2) + pow(y, 2) + pow(z, 2))*cos(3.f*atan2(y, x)) +
				y*pow(z, 2)*sin(3.f*atan2(y, x)))) /
				(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2)),
		(3.f*sqrt(35.f / (2.f*M_PI))*z*pow((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)), 2.5)*sin(3.f*atan2(y, x))) /
			(4.f*(pow(x, 2) + pow(y, 2)))
	);
	output[15] = Vec3f(
		(-3.f * sqrt(35.f / (2.f*M_PI))*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
			(x*pow(z, 2)*cos(3.f * atan2(y, x)) +
				y*(pow(x, 2) + pow(y, 2) + pow(z, 2))*sin(3.f * atan2(y, x)))) /
				(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2)),
		(3.f * sqrt(35.f / (2.f*M_PI))*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
			(-(y*pow(z, 2)*cos(3.f * atan2(y, x))) +
				x*(pow(x, 2) + pow(y, 2) + pow(z, 2))*sin(3.f * atan2(y, x)))) /
				(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2)),
		(3.f * sqrt(35.f / (2.f*M_PI))*z*pow((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)), 2.5f)*cos(3.f * atan2(y, x))) /
			(4.f*(pow(x, 2) + pow(y, 2)))
	);

	// l=4, m=+/-3
	output[17] = Vec3f(
		(3.f*sqrt(35.f / (2.*M_PI))*x*y*z*(3.f*pow(x, 2) - 5.f*pow(y, 2) - 3.f*pow(z, 2))*
			sqrt((pow(x, 2) + pow(y, 2)) / (pow(x, 2) + pow(y, 2) + pow(z, 2)))
			) / (2.*sqrt(pow(x, 2) + pow(y, 2))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5)),
				(3.f*sqrt(35.f / (2.*M_PI))*z*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
					(-3.f*x*(pow(x, 2) + pow(y, 2) + pow(z, 2))*cos(3.f*atan2(y, x)) +
						y*(pow(x, 2) + pow(y, 2) - 3.f*pow(z, 2))*sin(3.f*atan2(y, x)))) /
						(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5)),
		(-3.f*sqrt(35.f / (2.*M_PI))*(pow(x, 2) + pow(y, 2) - 3.f*pow(z, 2))*
			pow((pow(x, 2) + pow(y, 2)) /
			(pow(x, 2) + pow(y, 2) + pow(z, 2)), 1.5)*sin(3.f*atan2(y, x))) /
				(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 1.5))
	);
	output[23] = Vec3f(
		(3.f * sqrt(35.f / (2.f*M_PI))*z*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
			(x*(pow(x, 2) + pow(y, 2) - 3.f * pow(z, 2))*cos(3.f * atan2(y, x)) -
				3.f * y*(pow(x, 2) + pow(y, 2) + pow(z, 2))*sin(3.f * atan2(y, x)))) /
				(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5f)),
		(3.f * sqrt(35.f / (2.f*M_PI))*x*y*z*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
			(5.f * pow(x, 2) - 3.f * pow(y, 2) + 3.f * pow(z, 2))) /
			(2.f*sqrt(pow(x, 2) + pow(y, 2))*
				pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2.5f)),
				(-3.f * sqrt(35.f / (2.f*M_PI))*(pow(x, 2) + pow(y, 2) - 3.f * pow(z, 2))*
					pow((pow(x, 2) + pow(y, 2)) /
					(pow(x, 2) + pow(y, 2) + pow(z, 2)), 1.5f)*cos(3.f * atan2(y, x))) /
						(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 1.5f))
	);

	// l=5, m=+/-3
	output[27] = Vec3f(
		(-3.f*sqrt(385.f / (2.f*M_PI))*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
			(y*(pow(x, 2) + pow(y, 2) - 8.f*pow(z, 2))*
			(pow(x, 2) + pow(y, 2) + pow(z, 2))*cos(3.f*atan2(y, x)) +
				x*pow(z, 2)*(-7.f*(pow(x, 2) + pow(y, 2)) + 8.f*pow(z, 2))*
				sin(3.f*atan2(y, x)))) /
				(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
		(3.f*sqrt(385.f / (2.f*M_PI))*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
			(x*(pow(x, 2) + pow(y, 2) - 8.f*pow(z, 2))*
			(pow(x, 2) + pow(y, 2) + pow(z, 2))*cos(3.f*atan2(y, x)) +
				y*pow(z, 2)*(7.f*(pow(x, 2) + pow(y, 2)) - 8.f*pow(z, 2))*
				sin(3.f*atan2(y, x)))) /
				(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
		(-3.f*sqrt(385.f / (2.f*M_PI))*z*(7.f*(pow(x, 2) + pow(y, 2)) - 8.f*pow(z, 2))*
			pow((pow(x, 2) + pow(y, 2)) /
			(pow(x, 2) + pow(y, 2) + pow(z, 2)), 1.5)*sin(3.f*atan2(y, x))) /
				(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2))
	);
	output[33] = Vec3f(
		(3.f*sqrt(3.85f / (2.*M_PI))*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
			(x*pow(z, 2)*(7 * (pow(x, 2) + pow(y, 2)) - 8.f*pow(z, 2))*
				cos(3.f*atan2(y, x)) + y*(pow(x, 2) + pow(y, 2) - 8.f*pow(z, 2))*
				(pow(x, 2) + pow(y, 2) + pow(z, 2))*sin(3.f*atan2(y, x)))) /
				(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.f)),
		(-3.f*sqrt(3.85f / (2.*M_PI))*sqrt((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)))*
			(y*pow(z, 2)*(-7 * (pow(x, 2) + pow(y, 2)) + 8.f*pow(z, 2))*
				cos(3.f*atan2(y, x)) + x*(pow(x, 2) + pow(y, 2) - 8.f*pow(z, 2))*
				(pow(x, 2) + pow(y, 2) + pow(z, 2))*sin(3.f*atan2(y, x)))) /
				(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.f)),
		(-3.f*sqrt(3.85f / (2.*M_PI))*z*(7 * (pow(x, 2) + pow(y, 2)) - 8.f*pow(z, 2))*
			pow((pow(x, 2) + pow(y, 2)) /
			(pow(x, 2) + pow(y, 2) + pow(z, 2)), 1.5)*cos(3.f*atan2(y, x))) /
				(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2))
	);

	/************ l=?, m=+/-4.f ************/

	// l=4, m=+/-4
	output[16] = Vec3f(
		(-3.f*sqrt(35.f / M_PI)*y*(pow(x, 4) + pow(y, 2)*(pow(y, 2) + pow(z, 2)) -
			3.f*pow(x, 2)*(2 * pow(y, 2) + pow(z, 2)))) /
			(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
		(3.f*sqrt(35.f / M_PI)*x*(pow(x, 4) + pow(y, 4) - 3.f*pow(y, 2)*pow(z, 2) +
			pow(x, 2)*(-6 * pow(y, 2) + pow(z, 2)))) /
			(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
		(3.f*sqrt(35.f / M_PI)*x*y*(-pow(x, 2) + pow(y, 2))*z) /
		pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)
	);
	output[24] = Vec3f(
		(3.f * sqrt(35.f / M_PI)*x*(-4.f * pow(y, 4) - 3.f * pow(y, 2)*pow(z, 2) +
			pow(x, 2)*(4.f * pow(y, 2) + pow(z, 2)))) /
			(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
		(3.f * sqrt(35.f / M_PI)*y*(-4.f * pow(x, 4) + pow(y, 2)*pow(z, 2) +
			pow(x, 2)*(4.f * pow(y, 2) - 3.f * pow(z, 2)))) /
			(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3)),
		(-3.f * sqrt(35.f / M_PI)*pow(pow(x, 2) + pow(y, 2), 2)*z*cos(4.f * atan2(y, x))) /
		(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3))
	);

	// l=5, m=+/-4
	output[26] = Vec3f(
		(-3.f*sqrt(385 / M_PI)*y*z*(2.f*pow(x, 4) +
			pow(y, 2)*(pow(y, 2) + pow(z, 2)) -
			pow(x, 2)*(7 * pow(y, 2) + 3.f*pow(z, 2)))) /
			(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5)),
		(3.f*sqrt(385 / M_PI)*x*z*(pow(x, 4) + 2.f*pow(y, 4) -
			3.f*pow(y, 2)*pow(z, 2) + pow(x, 2)*(-7.f*pow(y, 2) + pow(z, 2)))) /
			(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5)),
		(3.f*sqrt(385 / M_PI)*x*(x - y)*y*(x + y)*
		(pow(x, 2) + pow(y, 2) - 4.f*pow(z, 2))) /
			(4.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5))
	);
	output[34] = Vec3f(
		(-3.f * sqrt(385.f / M_PI)*x*z*(pow(x, 4) - 22 * pow(x, 2)*pow(y, 2) +
			17.f * pow(y, 4) - 4.f * (pow(x, 2) - 3.f * pow(y, 2))*pow(z, 2))) /
			(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5f)),
		(-3.f * sqrt(385.f / M_PI)*y*z*(17.f * pow(x, 4) + pow(y, 4) -
			4.f * pow(y, 2)*pow(z, 2) +
			pow(x, 2)*(-22 * pow(y, 2) + 12 * pow(z, 2)))) /
			(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5f)),
		(3.f * sqrt(385.f / M_PI)*pow(pow(x, 2) + pow(y, 2), 2)*
		(pow(x, 2) + pow(y, 2) - 4.f * pow(z, 2))*cos(4.f * atan2(y, x))) /
			(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 3.5f))
	);

	/************ l=?, m=+/-5.f ************/

	// l=5, m=+/-5
	output[25] = Vec3f( // PROBLEM CHILD
		(15.f*sqrt(77.f / (2.f*M_PI))*pow((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)), 1.5)*
			(y*(pow(x, 2) + pow(y, 2) + pow(z, 2))*cos(5.f*atan2(y, x)) -
				x*pow(z, 2)*sin(5.f*atan2(y, x)))) /
				(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2)),
		(-15.f*sqrt(77.f / (2.f*M_PI))*pow((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)), 1.5)*
			(x*(pow(x, 2) + pow(y, 2) + pow(z, 2))*cos(5.f*atan2(y, x)) +
				y*pow(z, 2)*sin(5.f*atan2(y, x)))) /
				(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2)),
		(15.f*sqrt(77.f / (2.f*M_PI))*z*pow((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)), 3.5)*sin(5.f*atan2(y, x))) /
			(16.f*(pow(x, 2) + pow(y, 2)))
	);
	output[35] = Vec3f(
		(-15.f * sqrt(77.f / (2.f*M_PI))*pow((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)), 1.5f)*
			(x*pow(z, 2)*cos(5.f * atan2(y, x)) +
				y*(pow(x, 2) + pow(y, 2) + pow(z, 2))*sin(5.f * atan2(y, x)))) /
				(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2)),
		(15.f * sqrt(77.f / (2.f*M_PI))*pow((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)), 1.5f)*
			(-(y*pow(z, 2)*cos(5.f * atan2(y, x))) +
				x*(pow(x, 2) + pow(y, 2) + pow(z, 2))*sin(5.f * atan2(y, x)))) /
				(16.f*pow(pow(x, 2) + pow(y, 2) + pow(z, 2), 2)),
		(15.f * sqrt(77.f / (2.f*M_PI))*z*pow((pow(x, 2) + pow(y, 2)) /
		(pow(x, 2) + pow(y, 2) + pow(z, 2)), 3.5f)*cos(5.f * atan2(y, x))) /
			(16.f*(pow(x, 2) + pow(y, 2)))
	);
}

void SphericalHarmonic::createJacobian(Vec3f coord, Mat3f* output)
{
	float x = coord[0], y = coord[1], z = coord[2];

	*output = Mat3f(
		(pow(y,2) + pow(z,2)) / pow(pow(x,2) + pow(y,2) + pow(z,2),1.5),
		-((x*y) / pow(pow(x,2) + pow(y,2) + pow(z,2),1.5)),
		-((x*z) / pow(pow(x,2) + pow(y,2) + pow(z,2),1.5)),

		-((x*y) / pow(pow(x,2) + pow(y,2) + pow(z,2),1.5)),
		(pow(x,2) + pow(z,2)) / pow(pow(x,2) + pow(y,2) + pow(z,2),1.5),
		-((y*z) / pow(pow(x,2) + pow(y,2) + pow(z,2),1.5)),

		-((x*z) / pow(pow(x,2) + pow(y,2) + pow(z,2),1.5)),
		-((y*z) / pow(pow(x,2) + pow(y,2) + pow(z,2),1.5)),
		(pow(x,2) + pow(y,2)) / pow(pow(x,2) + pow(y,2) + pow(z,2),1.5)
	);
}