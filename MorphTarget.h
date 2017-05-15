#ifndef __MORPH_TARGET_H__
#define __MORPH_TARGET_H__

#define NUM_COEFF_GRPS 2
#define MORPH_CHAN 3
#define TEX_SPLIT 3

#include "ObjectFileLoader.h"
#include "LDPRT.h"

class Morphs
{
public:
	Morphs();
	~Morphs();

	void setupViewport();
	void genTex(ObjLoader* obj, ObjLoader* baseObj);
	void setMorph(float amount, GLuint sceneTex, bool alpha);
	GLuint getMorphID();
	size_t width() { return size; }

	static GLuint morphSampler;

	static GLfloat viewportMorph[4 * MORPH_CHAN];

private:
	GLuint tex;
	size_t size;

	static GLuint morphProgram, morphVS, morphGS, morphFS;
	static GLuint morphBuffer;

	struct PlaneVerts
	{
		Vec3f position;
		Vec2f texcoord;

		PlaneVerts(Vec3f position, Vec2f texcoord)
			: position(position), texcoord(texcoord) {}
	};

	const PlaneVerts planeData[6] = {
		{ { -1, -1, 0 }, { 0, 0 } },
		{ { 1, -1, 0 }, { 1, 0 } },
		{ { 1, 1, 0 }, { 1, 1 } },

		{ { -1, 1, 0 }, { 0, 1 } },
		{ { -1, -1, 0 },{ 0, 0 } },
		{ { 1, 1, 0 },{ 1, 1 } }
	};
};

class MorphTarget
{
public:
	MorphTarget();
	~MorphTarget();

	static size_t toggle;
	static bool animate;
	static bool tess;

	void load(const char** morphList, size_t size);
	void draw();
	void reset();
private:
	void applyMorphs();

	LDPRT ldprt;
	Morphs* morphs = nullptr;
	GLuint sceneFB = 0, sceneTex = 0;

	GLuint* sceneBuffer = nullptr;

	ObjLoader baseObj;

	size_t numMorphs = 0;

	GLuint environTex;

	static GLuint morphTargetSampler;

	static GLuint morphTargetProgram, morphTargetVS,
		morphTargetTCS, morphTargetTES,
		morphTargetGS, morphTargetFS;

	struct MorphTargetUpload
	{
		GLuint vertID;
		Vec2f texCoord;
		Vec4f coeffs[3][NUM_COEFF_GRPS];
	};

	// options
	float oily;
	float timeM = 0;
};

#endif