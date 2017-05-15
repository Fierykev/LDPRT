#ifndef __LDPRT_H__
#define __LDPRT_H__

#include <lbfgs.h>
#include "PRT.h"
#include "vec.h"
#include "ObjectFileLoader.h"

class LDPRT
{
public:
	LDPRT();
	~LDPRT();

	void load(ObjLoader* geo);
	void draw();

	const Vec3f** getCoeffs()
	{
		return (const Vec3f**)coeffs;
	}

	const GLuint* getCubeMaps()
	{
		return (const GLuint*)harmonicTex;
	}

	static bool toggle;

private:

	void genSphericalHarmonicTextures6(size_t size);
	void calcLDPRT(PRT* prt);
	void calcLDPRTBFGS(PRT* prt);

	Vec3f** coeffs;

	// textures
	GLuint harmonicTex[9];
	PRT prt;
	ObjLoader* obj;
};

class EvalLDPRT
{
public:
	void set(const Vec3f* prtCoeffs)
	{
		this->prtCoeffs = prtCoeffs;
	}

	static lbfgsfloatval_t evaluate(
		void *instance,
		const lbfgsfloatval_t *x,
		lbfgsfloatval_t *g,
		const int n,
		const lbfgsfloatval_t step);

	static int progress(
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
	);

	lbfgsfloatval_t evaluateLS(
		const lbfgsfloatval_t *x,
		lbfgsfloatval_t *g,
		const int n,
		const lbfgsfloatval_t step
	);

private:
	const Vec3f* prtCoeffs;
};

#endif