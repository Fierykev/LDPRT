#include "BVH.h"

#include <algorithm>
#include <thread>

using namespace std;

void threadedBVHConstruction(BVH* bvh, uint ID);

BVH::~BVH()
{
	if (BVHTree != nullptr)
		delete[] BVHTree;
}

uint BVH::leadingZero(unsigned int num)
{
	// use the index as a tie breaker if they are the same
	uint data = num ^ 0;

	// remove non-leading zeros

	data |= data >> 1;
	data |= data >> 2;
	data |= data >> 4;
	data |= data >> 8;
	data |= data >> 16;
	data++;

	return deBruijinLookup[data * 0x076be629 >> 27];
}

uint BVH::ceilLog2(size_t num)
{
	if (sizeof(num) == 4)
	{
		if (num == 0)
			return 0;

		// round up to power of two
		num--;

		for (int i = 1; i < 32; i <<= 1)
			num |= num >> i;

		num++;

		// use leading zeros to find log2
		return (32 - leadingZero(num)) - 1;
	}

	return (uint)ceil(log2((double)num));
}

void BVH::createBVH(Geometry* geo)
{
	this->geo = geo;

	// clear out old BVH if needed
	if (BVHTree != nullptr)
		delete[] BVHTree;

	// generate Morton Codes
	loadFactor = max(
		(1 << max(ceilLog2(geo->numberOfObjects()),
		(uint)log2(NUM_THREADS))) / NUM_THREADS,
		1);
	DATA_SIZE = loadFactor * NUM_THREADS;
	numObjects = DATA_SIZE;

	BVHTree = new Node[DATA_SIZE * 2];
	positionNotPresent = new uint[DATA_SIZE];

	transferBuffer = new std::atomic<uint>[DATA_SIZE];

	std::thread th[NUM_THREADS];

	// launch threads
	for (uint i = 0; i < NUM_THREADS; i++)
		th[i] = std::thread(threadedBVHConstruction, this, i);

	// wait for everything to finish
	for (uint i = 0; i < NUM_THREADS; i++)
		th[i].join();

	// clean up
	delete[] positionNotPresent;
}

void threadedBVHConstruction(BVH* bvh, uint ID)
{
	// create morton codes
	bvh->mortonCodes(uint3(ID, 0, 0), ID, uint3(0, 0, 0));
	bvh->GlobalMemoryBarrier();

	// radix sort for 30 bits (morton code size)
	for (uint i = 0; i < 30; i++)
	{
		bvh->radixPart1(uint3(ID, 0, 0), ID, uint3(0, 0, 0), i);
		bvh->GlobalMemoryBarrier();
		bvh->radixPart2(uint3(ID, 0, 0), ID, uint3(0, 0, 0), i);
		bvh->GlobalMemoryBarrier();

		bvh->GlobalMemoryBarrier();
	}

	// construct BVH tree
	
	for (uint i = 0; i < bvh->loadFactor; i++)
		bvh->BVHConstructPart1(
			uint3(ID * bvh->loadFactor + i, 0, 0),
			ID * bvh->loadFactor + i, uint3(0, 0, 0));
	bvh->GlobalMemoryBarrier();
	/*
	// TMP
	bvh->BVHTree[0].code = 1;
	bvh->BVHTree[1].code = 2;
	bvh->BVHTree[2].code = 4;
	bvh->BVHTree[3].code = 5;
	bvh->BVHTree[4].code = 19;
	bvh->BVHTree[5].code = 24;
	bvh->BVHTree[6].code = 25;
	bvh->BVHTree[7].code = 24;

	bvh->numObjects = 8;
	for (uint i = 0; i < bvh->numObjects; i++)
		bvh->BVHConstructPart1(
			uint3(i, 0, 0),
			i, uint3(0, 0, 0)
		);

	// END TMP*/

	for (uint i = 0; i < bvh->loadFactor; i++)
	{
		if (i % 1000 == 0 && ID == 0)
			cout << "BVH Construction (" << ID << "): " << double(i) / double(bvh->loadFactor) * 100.0 << "%" << endl;

		bvh->BVHConstructPart2(
			uint3(ID * bvh->loadFactor + i, 0, 0),
			ID * bvh->loadFactor + i, uint3(0, 0, 0));
	}
	bvh->GlobalMemoryBarrier();
}