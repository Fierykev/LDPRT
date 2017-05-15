#include "BVH.h"

/**************************************
MORTON CODES
**************************************/

uint32_t bitTwiddling(uint32_t var)
{
	const uint byteMasks[] = { 0x09249249, 0x030c30c3, 0x0300f00f, 0x030000ff, 0x000003ff };
	uint curVal = 16;

	for (uint i = 4; 0 < i; i--)
	{
		var &= byteMasks[i]; // set everything other than the last 10 bits to zero
		var |= var << curVal; // add in the byte offset

		curVal >>= 1; // divide by two
	}

	// add in the final mask
	var &= byteMasks[0];

	return var;
}

uint calcMortonCode(float3 p)
{
	uint3 code;

	// multiply out
	for (int i = 0; i < 3; i++)
	{
		// change to base .5
		p[i] *= 1024.f;

		// clamp between [0, 1024)
		p[i] = clamp(p[i], 0, 1023);

		code[i] = bitTwiddling((uint32_t)p[i]);
	}

	// combine codes for the output
	return code[0] | code[1] << 1 | code[2] << 2;
}

void BVH::mortonCodes(uint3 threadID, uint groupThreadID, uint3 groupID)
{
	/***********************************************
	Generate the Morton Codes
	***********************************************/

	float3 bbMin, bbMax;
	float3 avg;

	float3 vertData;

	uint index = 0, mortonCode = 0;

	float3 sceneBBMin = float3(
		geo->getBoundingBoxMin()[0],
		geo->getBoundingBoxMin()[1],
		geo->getBoundingBoxMin()[2]
	);
	float3 sceneBBMax = float3(
		geo->getBoundingBoxMax()[0],
		geo->getBoundingBoxMax()[1],
		geo->getBoundingBoxMax()[2]
	);

	// note unroll causes error
	for (uint loadi = 0; loadi < loadFactor; loadi++)
	{
		bbMin = sceneBBMin;
		bbMax = sceneBBMin;

		index = (threadID.x * loadFactor) + loadi;

		// store MAX_UTYPE for the case that the index is out of bounds
		BVHTree[index].index = MAX_UTYPE;
		
		// check if data is in bounds
		if (index < geo->numberOfObjects())
		{
			bbMin = vecToFloat(geo->getMinBB(index));
			bbMax = vecToFloat(geo->getMaxBB(index));
			avg = vecToFloat(geo->getCentroid(index));

			mortonCode = calcMortonCode((avg - sceneBBMin) /
				(sceneBBMax - sceneBBMin));

			// store vert index data
			BVHTree[index].index = index;
		}

		// place the centroid point in the unit cube and calculate / store the morton code
		// store globally as well

		// store the code
		BVHTree[index].code = mortonCode;

		// store bounding box info
		BVHTree[index].bbox.bbMin = bbMin;
		BVHTree[index].bbox.bbMax = bbMax;

		// set children to invalid
		BVHTree[index].childL = MAX_UTYPE;
		BVHTree[index].childR = MAX_UTYPE;
	}
}