#define _USE_MATH_DEFINES

#include <math.h>

#include <GL/glew.h>
#include "LDPRT.h"
#include "BVH.h"
#include "Image.h"
#include "ObjectFileLoader.h"
#include "SphericalHarmonic.h"

LDPRT::LDPRT()
{

}

LDPRT::~LDPRT()
{

}

Vec3f getSHTex(float x, float y, int it)
{
	switch (it)
	{
	case 0:
		return{ 1.f, x, y };
		break;
	case 1:
		return{ -1.f, x, y };
		break;
	case 2:
		return{ y, 1.f, x };
		break;
	case 3:
		return{ y, -1.f, x };
		break;
	case 4:
		return{ y, x, 1.f };
		break;
	default:
		return{ y, x, -1.f };
	}
}

void LDPRT::genSphericalHarmonicTextures6(size_t size)
{
	// allocated already
	if (harmonicTex[0] != 0)
		return;

	const float xSign[6] = {
		1.f, 1.f, -1.f, 1.f, 1.f, 1.f
	};

	const float ySign[6] = {
		1.f, -1.f, -1.f, -1.f, -1.f, 1.f
	};

	Vec4f* pixels = new Vec4f[size * size];

	glEnable(GL_TEXTURE_CUBE_MAP);

	// for harmonic 6th order
	for (int i = 0; i < 9; i++)
	{
		// create cubemap
		glGenTextures(1, &harmonicTex[i]);
		glBindTexture(GL_TEXTURE_CUBE_MAP, harmonicTex[i]);

		float step = 2.f / (float)size;
		float start = 1.f - step / 2.f;

		for (int k = 0; k < 6; k++)
		{
			Vec4f* pixelPtr = pixels;

			for (float x = start * xSign[k];
				-1.f < x * xSign[k];
				x -= step * xSign[k])
			{
				for (float y = start * ySign[k];
					-1.f < y * ySign[k];
					y -= step * ySign[k])
				{
					Vec3f dir = getSHTex(x, y, k);
					dir.normalize();

					float out[NUM_BANDS * NUM_BANDS];
					SphericalHarmonic::evaluateSphericalHarmonic6(dir, out);

					for (int j = 0; j < 4; j++)
						(*pixelPtr)[j] = out[(i << 2) + j];

					pixelPtr++;
				}
			}

			// bind resource
			glTexParameteri(GL_TEXTURE_CUBE_MAP,
				GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP,
				GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + k,
				0, GL_RGBA32F,
				size, size,
				0, GL_RGBA, GL_FLOAT, pixels);
		}
	}

	glDisable(GL_TEXTURE_CUBE_MAP);

	delete[] pixels;
}

void LDPRT::calcLDPRT(PRT* prt)
{
	const Vec3f** prtCoeffs = prt->getCoeffs();

	float sh[NUM_BANDS * NUM_BANDS];
	const Vertex* verts = obj->getVertices();

	if (coeffs != nullptr)
		delete[] coeffs;

	coeffs = new Vec3f*[obj->getNumVertices()];

	for (size_t i = 0; i < obj->getNumVertices(); i++)
		coeffs[i] = new Vec3f[NUM_BANDS];

	// attempted least square approx.
	for (size_t i = 0; i < obj->getNumVertices(); i++)
	{
		// get harmonic in normal direction
		Vec3f norm = verts[i].normal;
		norm.normalize();

		// get harmonic in norm dir
		SphericalHarmonic::evaluateSphericalHarmonic6(
			norm, sh);

		size_t index = 0;

		for (int l = 0; l < NUM_BANDS; l++)
		{
			coeffs[i][l] = { 0,0,0 };

			for (int m = -l; m <= l; m++)
			{
				coeffs[i][l] +=
					sh[index] * prtCoeffs[i][index];

				index++;
			}

			coeffs[i][l] /= (2.f * (float)l + 1.f) / (4.f * M_PI);
		}
	}
}

int EvalLDPRT::progress(
	void *instance,
	const lbfgsfloatval_t *x,
	const lbfgsfloatval_t *g,
	const lbfgsfloatval_t fx,
	const lbfgsfloatval_t xnorm,
	const lbfgsfloatval_t gnorm,
	const lbfgsfloatval_t step,
	int n,
	int k,
	int ls
	)
{
	printf("Iteration %d:\n", k);
	printf("  fx = %f, x[0] = %f, x[1] = %f, x[2] = %f\n", fx, x[0], x[1], x[2]);
	printf("  xnorm = %f, gnorm = %f, step = %f\n", xnorm, gnorm, step);
	printf("\n");

	return 0;
}

lbfgsfloatval_t EvalLDPRT::evaluate(
    void *instance,
    const lbfgsfloatval_t *x,
    lbfgsfloatval_t *g,
    const int n,
    const lbfgsfloatval_t step
    )
{
	return reinterpret_cast<EvalLDPRT*>(instance)->evaluateLS(x, g, n, step);
}

lbfgsfloatval_t EvalLDPRT::evaluateLS(
	const lbfgsfloatval_t *x,
	lbfgsfloatval_t *g,
	const int n,
	const lbfgsfloatval_t step
	)
{
	// calculate gradients

	// s*
	Vec3f* gradientS = &((Vec3f*)g)[0];
	*gradientS = Vec3f(0, 0, 0);

	Vec3f norm = ((Vec3f*)x)[0], origNorm = ((Vec3f*)x)[0];
	norm.normalize();

	// gl*
	
	Vec3f* gradientGL = &((Vec3f*)g)[1];
	
	for (size_t i = 0; i < NUM_BANDS; i++)
		gradientGL[i] = Vec3f(0, 0, 0);
	
	Vec3f* data = &((Vec3f*)x)[1];
	
	// ylm(s*)
	float sh[NUM_BANDS * NUM_BANDS];

	SphericalHarmonic::evaluateSphericalHarmonic6(
		norm, sh);
	
	// calculate gradient for s*
	size_t index = 0;

	// construct h(s*)
	Vec3f h = norm;

	// construct 3 x 3 Jacobian

	Vec3f grad[NUM_BANDS * NUM_BANDS];
	
	SphericalHarmonic::evaluateSphericalHarmonicGradient6(
		norm,
		grad
	);

	Mat3f jacobT;
	
	SphericalHarmonic::createJacobian(origNorm, &jacobT);
	jacobT.transpose();

	for (int l = 0; l < NUM_BANDS; l++)
	{
		for (int m = -l; m <= l; m++)
		{
			Vec3f tApprox = sh[index] * data[l];

			Vec3f tmp = jacobT * grad[index] * data[l] * (tApprox - prtCoeffs[index]);

			*gradientS += grad[index] * data[l] * (tApprox - prtCoeffs[index]);
			index++;
		}
	}
	//cout << gradientS[0][0] << " " << gradientS[0][1] << " " << gradientS[0][2] << endl;
	*gradientS *= 2.f;
	
	// calculate gradient for all gl*
	index = 0;
	Vec3f glSum;
	
	for (int l = 0; l < NUM_BANDS; l++)
	{
		glSum = { 0, 0, 0 };

		for (int m = -l; m <= l; m++)
		{
			Vec3f tApprox = sh[index] * data[l];

			glSum += sh[index] * (tApprox - prtCoeffs[index]);

			index++;
		}

		// store gradient
		gradientGL[l] = glSum * 2.f;
	}
	
	index = 0;
	float errorSum = 0;
	
	for (int l = 0; l < NUM_BANDS; l++)
	{
		for (int m = -l; m <= l; m++)
		{
			const Vec3f tlm = sh[index] * data[l];

			const Vec3f diff = prtCoeffs[index] - tlm;
			errorSum += diff.length2();

			index++;
		}
	}

	return errorSum;
}

void LDPRT::calcLDPRTBFGS(PRT* prt)
{
	lbfgsfloatval_t fx;

	const Vertex* verts = obj->getVertices();
	Vec3f optAxis;

	EvalLDPRT eval;
	lbfgsfloatval_t* vars = lbfgs_malloc(sizeof(Vec3f) * (1 + NUM_BANDS));

	// get approx of gl*
	calcLDPRT(prt);

	// optimize estimate
	// best least square approx.
	for (size_t i = 0; i < obj->getNumVertices(); i++)
	{
		cout << "INDEX " << i << endl;
		optAxis = verts[i].normal;
		optAxis.normalize();

		// copy axis to vars (normal axis)
		((Vec3f*)vars)[0] = obj->getVertices()[i].normal;
		
		// copy over gl* vars
		for (size_t j = 1; j < NUM_BANDS + 1; j++)
			((Vec3f*)vars)[j] = coeffs[i][j - 1];

		// create LDPRT eval
		eval.set(prt->getCoeffs()[i]);

		int ret = lbfgs((1 + NUM_BANDS) * 3,
			vars, &fx, eval.evaluate, eval.progress, &eval, NULL);

		printf("NORM %f %f %f\n",
			optAxis[0],
			optAxis[1],
			optAxis[2]);

		// set normal
		//((Vertex*)obj->getVertices())[i].normal = ((Vec3f*)vars)[0];

		// set coeffs
		for (size_t j = 1; j < NUM_BANDS + 1; j++)
			coeffs[i][j - 1] = ((Vec3f*)vars)[j];

		optAxis = ((Vec3f*)vars)[0];

		/* Report the result. */
		printf("L-BFGS optimization terminated with status code = %d\n", ret);
		printf("  fx = %f, x[0] = %f, x[1] = %f, x[2] = %f\n", fx, optAxis[0], optAxis[1], optAxis[2]);
	}

	lbfgs_free(vars);
}

void LDPRT::load(const char* name, ObjLoader* geo)
{
	// store the object
	obj = geo;

	string file = "Output/LDPRT/";
	file += name;
	file += ".prt";

	// reset prt
	prt.reset();

	// run PRT if needed
	if (!prt.load(file.c_str()))
	{
		// create bvh from obj file
		BVH bvh;
		bvh.createBVH(geo);

		prt.calculate(name, &bvh, 32); // 32^2 samples
		prt.dump(file.c_str());
	}

	// create harmonic textures
	genSphericalHarmonicTextures6(NUM_BANDS * NUM_BANDS);
	
	// convert prt to ldprt
	//calcLDPRTBFGS(&prt);
	calcLDPRT(&prt);
}

void LDPRT::draw()
{
	obj->drawLDPRT((const Vec3f**)coeffs);
}

void LDPRT::prtDraw()
{
	prt.renderPRT(obj);
}