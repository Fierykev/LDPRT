#ifndef __PRT_H__
#define __PRT_H__

#define PRT_THREADS 6
#define NUM_BANDS 6
#define NUM_BOUNCES 8
#define SUBSCAT_SAMPLES 1
#define SUBSCAT_BOUNCES 6
#define SIGMA_A 1.0
#define SIGMA_S 4.0
#define SIGMA_T (SIGMA_A + SIGMA_S)
#define CORRECTION 10.0
#define MAX_R 5

#include <array>
#include <atomic>
#include <thread>

#include <unordered_map>
#include <unordered_set>

#include "vec.h"
#include "mat.h"
#include "ObjectFileLoader.h"
#include "BVH.h"
#include "Image.h"

class atomFloat
{
public:
	std::atomic<float> val;

	atomFloat()
		:val()
	{}

	atomFloat(const std::atomic<float> &inVal)
		:val(inVal.load())
	{}

	atomFloat(const atomFloat &inVal)
		:val(inVal.val.load())
	{}

	atomFloat &operator=(const atomFloat &inVal)
	{
		val.store(inVal.val.load());

		return *this;
	}

	atomFloat &operator=(const float &inVal)
	{
		val.store(inVal);

		return *this;
	}

	float load()
	{
		return val.load();
	}


private:
};

struct SphereCoords
{
	float theta, phi;
};

struct PRTSample
{
	SphereCoords sphereCoord;
	Vec3f dir;
	float harmonic[NUM_BANDS * NUM_BANDS];
};

class LightingModel
{
public:
	virtual Vec3f getColor(PRTSample& sample,
		unsigned int lightNum, isect* colIn = nullptr) = 0;
};

class PRT
{
public:
	PRT();
	~PRT();

	static void init();

	void calculate(
		const char* name, BVH* scene, size_t radNumSamples);

	const Vec3f** getCoeffs(){ return (const Vec3f**)vertCoeffs; }

	void reset();

	bool load(const char* fileName);

	void dump(const char* fileName);

	void renderPRT(ObjLoader* obj);

	void runSubsurfaceMonteCarlo(
		size_t ID, size_t loadFactor,
		BVH* bvh,
		vector<vector<array<atomFloat, 3>>> subsurfaceCoeffs,
		Vec3f** diffuseCoeff,
		PRTSample* samples,
		size_t numSamples,
		bool** hasEnergy);

	void runInterref(
		size_t loadFactor,
		size_t ID, BVH* bvh,
		Vec3f*** interrefCoeffs,
		PRTSample* samples,
		size_t numSamples,
		bool** hitMarks,
		size_t bounce
	);

	void runShadow(size_t ID,
		size_t loadFactor,
		BVH* bvh,
		Vec3f** shadowCoeff,
		PRTSample* samples,
		size_t numSamples,
		bool** hitMarks,
		bool** hasEnergy);

private:
	size_t vertNum;
	Vec3f** vertCoeffs = nullptr;
	void runLighting(LightingModel& model,
		Vec3f* coeffs, PRTSample* samples, size_t numSamples);
	PRTSample* genSamples(size_t numSamples);
	void calculateSHForSamples(PRTSample* samples, size_t numSamples);
	float evalCubic(float r, float rMax);
	void cubicSample(float& r, float& h, float xi);
	void addSubsurfaceMonteCarlo(BVH* bvh, PRTSample* samples,
		size_t numSamples, Vec3f** diffuseCoeff, bool** hasEnergy);
	void addSubsurfaceScattering(BVH* bvh, PRTSample* samples,
		size_t numSamples, Vec3f** diffuseCoeff, bool** hasEnergy);
	void addInterreflection(BVH* bvh, PRTSample* samples,
		size_t numSamples, Vec3f** diffuseCoeff,
		bool** hitMarks);
	Vec3f** calcShadow(BVH* bvh, PRTSample* samples,
		size_t numSamples, bool** hitMarks, bool** hasEnergy);
	void dumpShadow(BVH* bvh, PRTSample* samples,
		size_t numSamples, bool** hitMarks, bool** hasEnergy, const char* fileName);
	bool loadShadow(PRTSample*& samples,
		size_t* numSamples, bool**& hitMarks, bool**& hasEnergy,const char* fileName);

	atomic<size_t> completion;
};

#endif