#include "BVH.h"

void BVH::prefixSum(uint ID)
{
	// up sweep
	for (uint upSweepi = 1; upSweepi < DATA_SIZE; upSweepi <<= 1)
	{
		for (uint i = 0; i < loadFactor >> 1; i++)
		{
			uint indexL = (ID + i) * (upSweepi << 1) + upSweepi - 1;
			uint indexR = (ID + i) * (upSweepi << 1) + (upSweepi << 1) - 1;

			if (indexR < DATA_SIZE)
				positionNotPresent[indexR] += positionNotPresent[indexL];
		}

		// pause for group
		GroupMemoryBarrierWithGroupSync();
	}

	// down sweep

	if (ID == 0)
		positionNotPresent[DATA_SIZE - 1] = 0;

	// pause for group
	GroupMemoryBarrierWithGroupSync();

	for (uint downSweepi = DATA_SIZE >> 1; 0 < downSweepi; downSweepi >>= 1)
	{
		for (uint i = 0; i < loadFactor >> 1; i++)
		{
			uint ID_index = (ID + i) * (downSweepi << 1);

			uint index_1 = ID_index + downSweepi - 1;
			uint index_2 = ID_index + (downSweepi << 1) - 1;

			if (index_2 < DATA_SIZE)
			{
				uint tmp = positionNotPresent[ID_index + downSweepi - 1];
				positionNotPresent[index_1] = positionNotPresent[index_2];
				positionNotPresent[index_2] = tmp + positionNotPresent[index_2];
			}
		}

		// pause for group
		GroupMemoryBarrierWithGroupSync();
	}
}

void BVH::radixPart1(uint3 threadID, uint groupThreadID, uint3 groupID, uint radixi)
{
	/***********************************************
	Radix Sort P1
	***********************************************/

	// load in the data
	for (uint loadi = 0; loadi < loadFactor; loadi++)
	{
		uint index = (groupThreadID * loadFactor) + loadi;

		uint globalIndex = (threadID.x * loadFactor) + loadi;

		uint tmpData;

		if (radixi & 0x1)
			tmpData = BVHTree[globalIndex + numObjects].code;
		else
			tmpData = BVHTree[globalIndex].code;

		// store the inverted version of the important bit
		positionNotPresent[index] = !(tmpData & (1 << radixi));
	}

	uint totalOnes = 0;

	// set the one's count
	if (groupThreadID == NUM_THREADS - 1)
		totalOnes = positionNotPresent[DATA_SIZE - 1];

	// pause for the device accesses
	GroupMemoryBarrierWithGroupSync();

	// run a prefix sum
	prefixSum(groupThreadID * (loadFactor >> 1));

	// add the number of ones in this group to a global buffer
	if (groupThreadID == NUM_THREADS - 1)
		numOnesBuffer = (totalOnes += positionNotPresent[DATA_SIZE - 1]);
}

void BVH::radixPart2(uint3 threadID, uint groupThreadID, uint3 groupID, uint radixi)
{
	uint netOnes = numOnesBuffer;

	uint numPrecOnes = 0;

	// calculate position if the bit is not present
	for (uint loadi = 0; loadi < loadFactor; loadi++)
	{
		uint index = (groupThreadID * loadFactor) + loadi;
		uint globalIndex = (threadID.x * loadFactor) + loadi;

		uint tmpData;

		if (radixi & 0x1)
			tmpData = BVHTree[globalIndex + numObjects].code;
		else
			tmpData = BVHTree[globalIndex].code;

		uint tmpPositionNotPresent =
			positionNotPresent[globalIndex];

		uint positionPresent =
			globalIndex
			- tmpPositionNotPresent - numPrecOnes
			+ netOnes;

		uint sIndex =
			((tmpData & (1 << radixi)) ?
				positionPresent :
				tmpPositionNotPresent + numPrecOnes);

		uint lookupIndex = groupID.x * DATA_SIZE + index;

		if (radixi & 0x1) // odd
			BVHTree[sIndex] = BVHTree[lookupIndex + numObjects];
		else // even
			BVHTree[sIndex + numObjects] = BVHTree[lookupIndex];
	}
}