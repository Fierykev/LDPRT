#define _USE_MATH_DEFINES

#include <math.h>

#include "PRT.h"
#include "ShaderHelper.h"
#include "SphericalHarmonic.h"

void subsurfaceMonteCarloThread(PRT* prt,
	size_t loadFactor,
	size_t ID, BVH* bvh,
	vector<vector<array<atomFloat, 3>>> subsurfaceCoeffs,
	Vec3f** diffuseCoeff,
	PRTSample* samples,
	size_t numSamples,
	bool** hasEnergy);

void interreflectionThread(PRT* prt,
	size_t loadFactor,
	size_t ID, BVH* bvh,
	Vec3f*** interrefCoeffs,
	PRTSample* samples,
	size_t numSamples,
	bool** hitMarks,
	size_t bounce);

void shadowThread(PRT* prt,
	size_t loadFactor,
	size_t ID, BVH* bvh,
	Vec3f** shadowCoeff,
	PRTSample* samples,
	size_t numSamples,
	bool** hitMarks,
	bool** hasEnergy);

class ShadowModel : public LightingModel
{
public:

	ShadowModel(BVH* scene, size_t vertIndex, Vec3f color,
		bool** hitMarks = nullptr, bool** hasEnergy = nullptr)
		: scene(scene), vertIndex(vertIndex), color(color),
		hitMarks(hitMarks), hasEnergy(hasEnergy),
		LightingModel() {}

	Vec3f getColor(PRTSample& sample, unsigned int lightNum, isect* colIn = nullptr)
	{
		const float angle =
			scene->getNormal(vertIndex) * sample.dir;

		Ray ray;
		ray.direction = sample.dir;
		ray.invDirection = {
			1.f / ray.direction[0],
			1.f / ray.direction[1],
			1.f / ray.direction[2]
		};

		ray.origin = scene->getPosition(vertIndex);

		isect col;

		bool hit = scene->findCollision(ray, col);

		if (colIn != nullptr)
			*colIn = col;

		if (hasEnergy != nullptr && (angle <= 0.f || !hit))
			hasEnergy[lightNum][vertIndex] = true;

		if (angle <= 0.f)
		{
			// use depth for contribution
			float tInfluence = min(max(col.t, 0.f), 1.f);
			tInfluence = 1.f - tInfluence * 4.f;
			tInfluence = min(max(tInfluence, 0.f), 1.f);
			return tInfluence / 4.f * color;
		}
		else if (!hit) // no shadow
		{
			return angle * color;
		}

		// shadow if hitmarks valid
		if (hitMarks != nullptr && col.normal * ray.direction < 0.f)
			hitMarks[lightNum][vertIndex] = true;

		return Vec3f(0.f, 0.f, 0.f);
	}

private:
	BVH* scene;
	size_t vertIndex;
	Vec3f color;
	bool** hitMarks;
	bool** hasEnergy;
};

PRT::PRT()
{

}

PRT::~PRT()
{

}

float ran()
{
	return (float)(rand() % 20000) / 20000;
}

float ranSign()
{
	return ran() > .5f ? 1.f : -1.f;
}

void PRT::init()
{
	
}

void PRT::calculate(BVH* bvh, size_t radNumSamples)
{
	vertNum = bvh->numberofVerts();

	size_t numSamples;
	PRTSample* samples;
	bool** hitMarks, **hasEnergy;
	
	if (!loadShadow(samples, &numSamples, hitMarks, hasEnergy, 		"Output/HeadSubSurf.shadow"))
	{
		numSamples = radNumSamples * radNumSamples;
		samples = genSamples(radNumSamples);
		calculateSHForSamples(samples, numSamples);
		
		hitMarks = new bool*[numSamples];
		
		for (size_t i = 0; i < numSamples; i++)
		{
			hitMarks[i] = new bool[bvh->numberofVerts()];

			memset(hitMarks[i], 0, sizeof(bool) * bvh->numberofVerts());
		}

		hasEnergy = new bool*[numSamples];

		for (size_t i = 0; i < numSamples; i++)
		{
			hasEnergy[i] = new bool[bvh->numberofVerts()];

			memset(hasEnergy[i], 0, sizeof(bool) * bvh->numberofVerts());
		}

		vertCoeffs = calcShadow(bvh, samples, numSamples, hitMarks, hasEnergy);
		
		// calculate subsurface scattering
		addSubsurfaceMonteCarlo(bvh, samples, numSamples, vertCoeffs, hasEnergy);

		// calculate  interreflection
		addInterreflection(bvh, samples, numSamples, vertCoeffs, hitMarks);
	}
	
	// clear mem
	for (size_t i = 0; i < numSamples; i++)
	{
		delete[] hitMarks[i], hasEnergy[i];
	}

	delete[] hitMarks, hasEnergy;
}

float PRT::evalCubic(float r, float rMax)
{
	float rm_5 = rMax * rMax * rMax * rMax * rMax;
	float deltaR = rMax - r;
	float deltaR_3 = deltaR * deltaR * deltaR;

	return 10.f * deltaR_3 / (M_PI * rm_5);
}

float cubicRoot(float xi)
{
	const float tolerance = 1e-6f;
	float x = .25f;

	for (int i = 0; i < 10; i++)
	{
		float x2 = x*x;
		float x3 = x2*x;
		float nx = (1.0f - x);

		float f = 10.0f*x2 - 20.0f*x3 + 15.0f*x2*x2 - 4.0f*x2*x3 - xi;
		float f_ = 20.0f*(x*nx)*(nx*nx);

		if (fabsf(f) < tolerance || f_ == 0.0f)
			break;

		x = max(min(x - f / f_, 1.f), 0.f);
	}
	
	return x;
}

void PRT::cubicSample(float& r, float& h, float xi)
{
	r = cubicRoot(xi) * MAX_R;
	h = sqrt(MAX_R * MAX_R - r * r);
}

void PRT::addSubsurfaceMonteCarlo(BVH* bvh, PRTSample* samples,
	size_t numSamples, Vec3f** diffuseCoeff, bool** hasEnergy)
{
	// calculate shadows
	vector<vector<array<atomFloat, 3>>> scatterCoeffs;
	scatterCoeffs.resize(bvh->numberofVerts());

	for (size_t i = 0; i < bvh->numberofVerts(); i++)
	{
		scatterCoeffs[i].resize(NUM_BANDS * NUM_BANDS);

		for (size_t j = 0; j < NUM_BANDS * NUM_BANDS; j++)
		{
			scatterCoeffs[i][j][0] = 0;
			scatterCoeffs[i][j][1] = 0;
			scatterCoeffs[i][j][2] = 0;
		}
	}
	
	// split the jobs up
	std::thread th[PRT_THREADS + 1];

	// set completion
	completion.store(0);

	const size_t loadFactor = numSamples / PRT_THREADS + 1LL;

	for (size_t i = 0; i < PRT_THREADS + 1; i++)
		th[i] = std::thread(
			subsurfaceMonteCarloThread,
			this,
			loadFactor,
			i,
			bvh,
			scatterCoeffs,
			diffuseCoeff,
			samples,
			numSamples,
			hasEnergy);

	// wait for everything to finish
	for (uint i = 0; i < PRT_THREADS + 1; i++)
		th[i].join();
	

	// sum all bounces
	for (size_t k = 0; k < bvh->numberofVerts(); k++)
	{
		for (size_t l = 0; l < NUM_BANDS * NUM_BANDS; l++)
		{
			Vec3f val = {
				scatterCoeffs[k][l][0].load(),
				scatterCoeffs[k][l][1].load(),
				scatterCoeffs[k][l][2].load()
			};

			val *= (4.f * M_PI) / numSamples;
			diffuseCoeff[k][l] += val;
		}
	}
}

void PRT::runSubsurfaceMonteCarlo(
	size_t ID, size_t loadFactor,
	BVH* bvh,
	vector<vector<array<atomFloat, 3>>> scatterCoeffs,
	Vec3f** diffuseCoeff,
	PRTSample* samples,
	size_t numSamples,
	bool** hasEnergy)
{
	const float minDistance = .005f;
	const float maxDistance = .01f;
	const float ir = .43f;

	// print thread
	while (ID == PRT_THREADS)
	{
		size_t progress = completion.load();

		cout << "SubScat Completion: "
			<< float(progress)
			/ float(numSamples * bvh->numberofVerts() * SUBSCAT_SAMPLES) * 100.f << "%" << endl;

		// check if finished
		if (progress == numSamples * bvh->numberofVerts() * SUBSCAT_SAMPLES)
			return;

		Sleep(1000);
	}

	size_t upperBound = (ID + 1LL) * loadFactor;

	// go through samples
	for (size_t i = ID * loadFactor;
		i < numSamples
		&& i < upperBound; i++)
	{
		for (size_t vert = 0; vert < bvh->numberofVerts(); vert++)
		{
			if (!hasEnergy[i][vert])
			{
				completion.fetch_add(SUBSCAT_SAMPLES);
				continue;
			}

			Vec3f color = Vec3f(0, 0, 0);

			// get color
			for (size_t m = 0; m < NUM_BANDS * NUM_BANDS; m++)
				color += diffuseCoeff[vert][m] * samples->harmonic[m];
			color /= M_PI;

			//cout << "COL " << color << endl;
			for (size_t sampleNum = 0; sampleNum < SUBSCAT_SAMPLES; sampleNum++)
			{
				Vec3f point = bvh->getPosition(vert);
				Vec3f direction = samples[i].dir;
				float distanceTraveled = 0;
				float weight = 1.f;

				for (size_t bounce = 0; bounce < SUBSCAT_BOUNCES; bounce++)
				{
					// random walk
					Vec3f orthoNormal = Vec3f(-direction[1], direction[0], 0);
					orthoNormal.normalize();

					Mat4f rot = Mat4f::createRotation(ir,
						orthoNormal[0], orthoNormal[1], orthoNormal[2]);
					Vec3f newDir = rot *
						Vec4f(direction[0], direction[1], direction[2], 1);

					// rotate randomly around former dir
					rot = Mat4f::createRotation(ran() * 2.f * M_PI,
						direction[0], direction[1], direction[2]);
					newDir = rot * newDir;

					// update direction
					direction = newDir;

					// choose a distance
					float distance = minDistance + ran() * (maxDistance - minDistance);

					Ray ray;
					ray.direction = direction;
					ray.invDirection = Vec3f(
						1.f / direction[0],
						1.f / direction[1],
						1.f / direction[2]
					);
					ray.origin = point;

					// update weight
					float dw = SIGMA_A / SIGMA_T;
					weight -= dw;
					weight = max(weight, 0.f);

					if (weight < .001f)
						if (1.f / CORRECTION < ran())
							break;
						else
							weight *= CORRECTION;

					isect col;

					// check for collision
					if (bvh->findCollision(ray, col))
					{


						// add to color
						for (size_t m = 0; m < NUM_BANDS * NUM_BANDS; m++)
						{
							for (size_t k = 0; k < 3; k++)
							{
								const float angle =
									bvh->getNormal(col.index[k]) * samples[i].dir;

								const float oldAngle =
									bvh->getNormal(vert) * samples[i].dir;
								Vec3f val = oldAngle * diffuseCoeff[vert][m];

								// add to scatter
								Vec3f add = weight * angle * color * samples->harmonic[m] / float(SUBSCAT_SAMPLES) * col.bary[k];

								for (size_t l = 0; l < 3; l++)
								{
									float old = scatterCoeffs[col.index[k]][m][l].val.load(std::memory_order_consume);
									float sum = old + add[l];

									while (!scatterCoeffs[col.index[k]][m][l].val.compare_exchange_weak(
										old, sum, std::memory_order_release, std::memory_order_consume))
									{
										sum = old + add[l];
									}
								}
							}
						}

						break;
					}

					// add to distance traveled
					distanceTraveled += distance;

					// probability ray dies
					float probNotAbsorbed = SIGMA_T * pow(M_E, -SIGMA_T * distanceTraveled);

					// absorbed
					if (probNotAbsorbed < ran())
						break;

					// move some distance
					point += direction + distance;
				}

				// finished ray
				completion.fetch_add(1);
			}
		}
	}
}

void subsurfaceMonteCarloThread(PRT* prt,
	size_t loadFactor,
	size_t ID, BVH* bvh,
	vector<vector<array<atomFloat, 3>>> subsurfaceCoeffs,
	Vec3f** diffuseCoeff,
	PRTSample* samples,
	size_t numSamples,
	bool** hasEnergy
)
{
	prt->runSubsurfaceMonteCarlo(ID, loadFactor, bvh, subsurfaceCoeffs, diffuseCoeff, samples, numSamples, hasEnergy);
}

void PRT::addSubsurfaceScattering(BVH* bvh, PRTSample* samples,
	size_t numSamples, Vec3f** diffuseCoeff, bool** hasEnergy)
{
	// consts
	float Ni = .5;
	float sigmaTr = MAX_R;
	float alphaPrime = .5;

	Vec3f** scatterCoeffs;

	scatterCoeffs = new Vec3f*[bvh->numberofVerts()];

	for (size_t i = 0; i < bvh->numberofVerts(); i++)
	{
		scatterCoeffs[i] = new Vec3f[NUM_BANDS * NUM_BANDS];

		memset(scatterCoeffs[i], 0, sizeof(Vec3f) * NUM_BANDS * NUM_BANDS);
	}

	// go through samples
	for (size_t i = 0; i < numSamples; i++)
	{
		for (size_t j = 0; j < bvh->numberofVerts(); j++)
		{
			if (j % 100 == 0)
			cout << "SubScat Completion: "
				<< float(i * bvh->numberofVerts() + j) / float(bvh->numberofVerts() * numSamples) * 100.f << "%" << endl;

			//if (!hasEnergy[i][j])
				//continue;

			Vec3f pos = bvh->getPosition(i);
			Vec3f color = bvh->getColor(i) / M_PI;

			Vec3f diffuseScatter = Vec3f(0.f, 0.f, 0.f);

			for (size_t k = 0; k < 2; k++)
			{
				for (size_t l = 0; l < SUBSCAT_SAMPLES; l++)
				{
					// grab random radius
					float xPos, height;
					cubicSample(xPos, height, ran());
					xPos *= ranSign();

					Vec3f normal = bvh->getNormal(i);
					normal.normalize();

					Vec3f orthoNormal = normal ^ pos;
					orthoNormal.normalize();

					// flip normal and ortho normal for horiz
					if (k == 1)
						swap(normal, orthoNormal);

					// place sample on  sphere
					float distance = sqrt(MAX_R * MAX_R - xPos * xPos);

					Vec3f startPos = pos + xPos * orthoNormal + distance * normal;
					Vec3f endPos = pos + xPos * orthoNormal - distance * normal;
					//cout << "NORM :" << normal << " POS:" << startPos << " X:" << xPos << endl;
					// search for a hit
					Ray ray;
					ray.origin = startPos;

					ray.direction =
						-normal;
					ray.direction.normalize();

					ray.invDirection = Vec3f(
						1.f / ray.direction[0],
						1.f / ray.direction[1],
						1.f / ray.direction[2]
					);

					ray.invDirection.normalize();

					// no collision
					isect col;
					if (!bvh->findCollision(ray, col))
						continue;

					Vec3f colPoint = ray.origin + ray.direction * col.t;
					float r = (colPoint - pos).length();

					// outside radius
					if (MAX_R < r)
						continue;

					float A;
					
					// calculate scatter color
					/*
					float zr = sqrt(3.f * (1.f - alphaPrime)) / sigmaTr;
					float zv = A * zr;
					float dr = sqrt(r * r + zr * zr);
					float dv = sqrt(r * r + zv * zv);
					float sigmaTrDr = sigmaTr * dr;
					float sigmaTrDv = sigmaTr * dv;
					float Rd = (sigmaTrDr + 1.f) *
						exp(-sigmaTrDr) * zr / pow(dr, 3)
						+ (sigmaTrDv + 1.f) *
						exp(-sigmaTrDv) * zr / pow(dv, 3);

					diffuseScatter += Ni * Rd / (
						sigmaTr * sigmaTr * exp(-sigmaTr * r));*/

					for (size_t m = 0; m < NUM_BANDS * NUM_BANDS; m++)
					{
						Vec3f interp = col.bary[0] * diffuseCoeff[col.index[0]][l]
							+ col.bary[1] * diffuseCoeff[col.index[1]][l]
							+ col.bary[2] * diffuseCoeff[col.index[2]][l];

						const float angle =
							bvh->getNormal(j) * samples[j].dir;

						interp *= angle;
						//diffuseScatter * 
						scatterCoeffs[k][l] += prod(interp, color);
					}
				}
			}
		}
	}

	// scale
	for (size_t j = 0; j < bvh->numberofVerts(); j++)
	{
		for (size_t k = 0; k < NUM_BANDS * NUM_BANDS; k++)
			scatterCoeffs[j][k] *= alphaPrime / (SUBSCAT_SAMPLES * SUBSCAT_SAMPLES);
	}

	for (size_t k = 0; k < bvh->numberofVerts(); k++)
	{
		for (size_t l = 0; l < NUM_BANDS * NUM_BANDS; l++)
		{
			diffuseCoeff[k][l] += scatterCoeffs[k][l];
		}
	}

	// free data
	for (size_t i = 0; i < bvh->numberofVerts(); i++)
	{
		delete scatterCoeffs[i];
	}

	delete[] scatterCoeffs;
}

void PRT::addInterreflection(BVH* bvh, PRTSample* samples,
	size_t numSamples, Vec3f** diffuseCoeff, bool** hitMarks)
{
	Vec3f** interrefCoeffs[NUM_BOUNCES];

	// store shadow coeffs
	interrefCoeffs[0] = diffuseCoeff;

	for (size_t i = 1; i < NUM_BOUNCES; i++)
	{
		interrefCoeffs[i] = new Vec3f*[bvh->numberofVerts()];

		for (size_t j = 0; j < bvh->numberofVerts(); j++)
		{
			interrefCoeffs[i][j] = new Vec3f[NUM_BANDS * NUM_BANDS];

			memset(interrefCoeffs[i][j], 0, sizeof(Vec3f) * NUM_BANDS * NUM_BANDS);
		}
	}

	const size_t loadFactor = bvh->numberofVerts() / PRT_THREADS + 1LL;

	// set completion
	completion.store(0);

	for (size_t i = 1; i < NUM_BOUNCES; i++)
	{
		// split the jobs up
		std::thread th[PRT_THREADS + 1];

		for (size_t j = 0; j < PRT_THREADS + 1; j++)
			th[j] = std::thread(
				interreflectionThread,
				this,
				loadFactor,
				j,
				bvh,
				interrefCoeffs,
				samples,
				numSamples,
				hitMarks,
				i);

		for (size_t j = 0; j < PRT_THREADS + 1; j++)
			th[j].join();
	}

	// sum all bounces
	for (size_t j = 1; j < NUM_BOUNCES; j++)
	{
		for (size_t k = 0; k < bvh->numberofVerts(); k++)
		{
			for (size_t l = 0; l < NUM_BANDS * NUM_BANDS; l++)
			{
				diffuseCoeff[k][l] += interrefCoeffs[j][k][l];
			}
		}
	}

	for (size_t i = 1; i < NUM_BOUNCES; i++)
	{
		for (size_t j = 0; j < bvh->numberofVerts(); j++)
			delete[] interrefCoeffs[i][j];

		delete[] interrefCoeffs[i];
	}
}

void PRT::runInterref(
	size_t loadFactor,
	size_t ID, BVH* bvh,
	Vec3f*** interrefCoeffs,
	PRTSample* samples,
	size_t numSamples,
	bool** hitMarks,
	size_t bounce
)
{
	// print thread
	while (ID == PRT_THREADS)
	{
		size_t progress = completion.load();
		
		cout << "Interreflection Completion: "
			<< (float)progress / (float)(bvh->numberofVerts() * numSamples)
			/ (float)(NUM_BOUNCES - 1) * 100.f << "%" << endl;

		// check if finished
		if (progress == bvh->numberofVerts() * numSamples * bounce)
			return;

		Sleep(1000);
	}

	size_t upperBound = (ID + 1LL) * loadFactor;

	// add intensity
	for (size_t k = ID * loadFactor;
		k < bvh->numberofVerts()
		&& k < upperBound; k++)
	{
		const Vec3f color = bvh->getColor(k) / M_PI;

		ShadowModel model(bvh, k, color);

		// loop through all lights
		for (size_t j = 0; j < numSamples; j++)
		{
			// no reason to run raytracing
			if (!hitMarks[j][k])
			{
				completion.fetch_add(1);
				continue;
			}

			isect col;

			model.getColor(samples[j], j, &col);

			for (size_t l = 0; l < NUM_BANDS * NUM_BANDS; l++)
			{
				Vec3f interp = col.bary[0] * interrefCoeffs[bounce - 1][col.index[0]][l]
					+ col.bary[1] * interrefCoeffs[bounce - 1][col.index[1]][l]
					+ col.bary[2] * interrefCoeffs[bounce - 1][col.index[2]][l];

				const float angle =
					bvh->getNormal(k) * samples[j].dir;

				interp *= angle;

				interrefCoeffs[bounce][k][l] += prod(interp, color);
			}

			// job finished
			completion.fetch_add(1);
		}
	}

	// scale
	for (size_t j = ID * loadFactor;
		j < bvh->numberofVerts()
		&& j < upperBound; j++)
	{
		for (size_t k = 0; k < NUM_BANDS * NUM_BANDS; k++)
			interrefCoeffs[bounce][j][k] *= (4.f * M_PI) / numSamples;
	}
}

void interreflectionThread(PRT* prt,
	size_t loadFactor,
	size_t ID, BVH* bvh,
	Vec3f*** interrefCoeffs,
	PRTSample* samples,
	size_t numSamples,
	bool** hitMarks,
	size_t bounce)
{
	prt->runInterref(loadFactor, ID, bvh, interrefCoeffs, samples, numSamples, hitMarks, bounce);
}

Vec3f** PRT::calcShadow(BVH* bvh, PRTSample* samples,
	size_t numSamples, bool** hitMarks, bool** hasEnergy)
{
	// calculate shadows
	Vec3f** shadowCoeff = new Vec3f*[bvh->numberofVerts()];

	for (size_t i = 0; i < bvh->numberofVerts(); i++)
		shadowCoeff[i] = new Vec3f[NUM_BANDS * NUM_BANDS];

	// split the jobs up
	std::thread th[PRT_THREADS + 1];

	// set completion
	completion.store(0);

	const size_t loadFactor = bvh->numberofVerts() / PRT_THREADS + 1LL;

	for (size_t i = 0; i < PRT_THREADS + 1; i++)
		th[i] = std::thread(
			shadowThread,
			this,
			loadFactor,
			i,
			bvh,
			shadowCoeff,
			samples,
			numSamples,
			hitMarks,
			hasEnergy);

	// wait for everything to finish
	for (uint i = 0; i < PRT_THREADS + 1; i++)
		th[i].join();

	return shadowCoeff;
}

void PRT::runShadow(size_t ID,
	size_t loadFactor,
	BVH* bvh,
	Vec3f** shadowCoeff,
	PRTSample* samples,
	size_t numSamples,
	bool** hitMarks,
	bool** hasEnergy)
{
	// print thread
	while (ID == PRT_THREADS)
	{
		size_t progress = completion.load();

		cout << "Shadow Completion: "
			<< (float)progress / (float)bvh->numberofVerts()
			* 100.f << "%" << endl;

		// check if finished
		if (progress == bvh->numberofVerts())
			return;

		Sleep(1000);
	}

	size_t upperBound = (ID + 1LL) * loadFactor;
	
	for (size_t i = ID * loadFactor;
		i < bvh->numberofVerts()
		&& i < upperBound; i++)
	{
		const Vec3f color = bvh->getColor(i) / M_PI;

		// TODO: fix color
		ShadowModel model(bvh, i, color, hitMarks, hasEnergy);

		runLighting(model,
			shadowCoeff[i], samples, numSamples);

		// job finished
		completion.fetch_add(1);
	}
}

void shadowThread(PRT* prt,
	size_t loadFactor,
	size_t ID, BVH* bvh,
	Vec3f** shadowCoeff,
	PRTSample* samples,
	size_t numSamples,
	bool** hitMarks,
	bool** hasEnergy)
{
	prt->runShadow(ID, loadFactor, bvh, shadowCoeff, samples, numSamples, hitMarks, hasEnergy);
}

void PRT::calculateSHForSamples(PRTSample* samples, size_t numSamples)
{
	for (size_t i = 0; i < numSamples; i++)
	{
		SphericalHarmonic::evaluateSphericalHarmonic6(
			samples[i].dir, samples[i].harmonic
		);
	}
}
// mogriphy image convert
void PRT::runLighting(LightingModel& model,
	Vec3f* coeffs, PRTSample* samples, size_t numSamples)
{
	// clear coeffs
	for (size_t i = 0; i < NUM_BANDS * NUM_BANDS; i++)
		coeffs[i] = Vec3f(0, 0, 0);

	// sample
	for (size_t i = 0; i < numSamples; i++)
	{
		// grab sample
		Vec3f col = model.getColor(samples[i], i);

		// color present
		if (!col.iszero())
		{
			for (size_t j = 0; j < NUM_BANDS * NUM_BANDS; j++)
			{
				coeffs[j] += col * samples[i].harmonic[j];
			}
		}
	}

	// scale
	for (size_t i = 0; i < NUM_BANDS * NUM_BANDS; i++)
	{
		coeffs[i] *= (4.f * M_PI) / numSamples;
	}
}

PRTSample* PRT::genSamples(size_t numSamples)
{
	PRTSample* samples = new PRTSample[numSamples * numSamples];

	for (size_t i = 0; i < numSamples; i++)
	{
		for (size_t j = 0; j < numSamples; j++)
		{
			const size_t index = i * numSamples + j;

			samples[index].sphereCoord.theta =
				2.f *
				acos(sqrt(1.f - ((float)i + ran()) / (float)numSamples));
			samples[index].sphereCoord.phi = 2.f * M_PI *
				((float)j + ran()) / (float)numSamples;

			const float sinTheta =
				sin(samples[index].sphereCoord.theta);
			const float cosTheta =
				cos(samples[index].sphereCoord.theta);

			const float sinPhi =
				sin(samples[index].sphereCoord.phi);
			const float cosPhi =
				cos(samples[index].sphereCoord.phi);

			samples[index].dir = {
				sinTheta * cosPhi,
				sinTheta * sinPhi,
				cosTheta
			};
			samples[index].dir.normalize();
		}
	}

	return samples;
}

void PRT::dumpShadow(BVH* bvh, PRTSample* samples,
	size_t numSamples, bool** hitMarks, bool** hasEnergy, const char* fileName)
{
	ofstream out(fileName);

	// output verts

	out << vertNum << endl;

	for (size_t i = 0; i < vertNum; i++)
	{
		for (size_t j = 0; j < NUM_BANDS * NUM_BANDS; j++)
		{
			for (size_t k = 0; k < 3; k++)
			{
				out << vertCoeffs[i][j][k] << endl;
			}
		}
	}

	// output samples

	out << numSamples << endl;

	for (size_t i = 0; i < numSamples; i++)
	{
		out << samples[i].sphereCoord.theta << endl;

		out << samples[i].sphereCoord.phi << endl;

		for (size_t j = 0; j < 3; j++)
			out << samples[i].dir[j] << endl;

		for (size_t j = 0; j < bvh->numberofVerts(); j++)
		{
			out << hitMarks[i][j] << endl;
		}

		for (size_t j = 0; j < bvh->numberofVerts(); j++)
		{
			out << hasEnergy[i][j] << endl;
		}
	}

	out.close();
}

bool PRT::loadShadow(PRTSample*& samples,
	size_t* numSamples, bool**& hitMarks, bool**& hasEnergy, const char* fileName)
{
	ifstream in(fileName);

	string line;

	if (!getline(in, line))
		return false;

	vertNum = strtoul(line.c_str(), NULL, 0);

	// allocate mem for verts

	vertCoeffs = new Vec3f*[vertNum];

	for (size_t i = 0; i < vertNum; i++)
		vertCoeffs[i] = new Vec3f[NUM_BANDS * NUM_BANDS];

	for (size_t i = 0; i < vertNum; i++)
	{
		for (size_t j = 0; j < NUM_BANDS * NUM_BANDS; j++)
		{
			for (size_t k = 0; k < 3; k++)
			{
				if (!getline(in, line))
					return false;

				vertCoeffs[i][j][k] = strtof(line.c_str(), NULL);
			}
		}
	}

	// allocate mem for samples
	if (!getline(in, line))
		return false;

	*numSamples = strtoul(line.c_str(), NULL, 0);

	samples = new PRTSample[*numSamples];

	hitMarks = new bool*[*numSamples];

	hasEnergy = new bool*[*numSamples];

	for (size_t i = 0; i < *numSamples; i++)
	{
		hitMarks[i] = new bool[vertNum];
		hasEnergy[i] = new bool[vertNum];

		memset(hitMarks[i], 0, sizeof(bool) * vertNum);
		memset(hasEnergy[i], 0, sizeof(bool) * vertNum);
	}

	for (size_t i = 0; i < *numSamples; i++)
	{
		if (!getline(in, line))
			return false;

		samples[i].sphereCoord.theta = strtof(line.c_str(), NULL);

		if (!getline(in, line))
			return false;

		samples[i].sphereCoord.phi = strtof(line.c_str(), NULL);

		for (size_t j = 0; j < 3; j++)
		{
			if (!getline(in, line))
				return false;

			samples[i].dir[j] = strtof(line.c_str(), NULL);
		}

		for (size_t j = 0; j < vertNum; j++)
		{
			if (!getline(in, line))
				return false;

			hitMarks[i][j] = strtof(line.c_str(), NULL);
		}

		for (size_t j = 0; j < vertNum; j++)
		{
			if (!getline(in, line))
				return false;

			hasEnergy[i][j] = strtof(line.c_str(), NULL);
		}
	}

	in.close();

	// calculate harmonics
	calculateSHForSamples(samples, *numSamples);

	return true;
}

void PRT::dump(const char* fileName)
{
	ofstream out(fileName);

	out << vertNum << endl;

	for (size_t i = 0; i < vertNum; i++)
	{
		for (size_t j = 0; j < NUM_BANDS * NUM_BANDS; j++)
		{
			for (size_t k = 0; k < 3; k++)
			{
				out << vertCoeffs[i][j][k] << endl;
			}
		}
	}

	out.close();
}

bool PRT::load(const char* fileName)
{
	ifstream in(fileName);

	string line;

	if (!getline(in, line))
		return false;

	vertNum = strtoul(line.c_str(), NULL, 0);

	// allocate mem

	vertCoeffs = new Vec3f*[vertNum];

	for (size_t i = 0; i < vertNum; i++)
		vertCoeffs[i] = new Vec3f[NUM_BANDS * NUM_BANDS];


	for (size_t i = 0; i < vertNum; i++)
	{
		for (size_t j = 0; j < NUM_BANDS * NUM_BANDS; j++)
		{
			for (size_t k = 0; k < 3; k++)
			{
				if (!getline(in, line))
					return false;

				vertCoeffs[i][j][k] = strtof(line.c_str(), NULL);
			}
		}
	}

	return true;
}

void PRT::renderPRT(ObjLoader* obj)
{
	obj->drawPRT((const Vec3f**)vertCoeffs);
}