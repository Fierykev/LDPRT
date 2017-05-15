#ifndef __BVH_H__
#define __BVH_H__

#include <condition_variable>
#include <mutex>
#include <atomic>

#include "HLSL_CPU.h"
#include "Geometry.h"

#define NUM_THREADS 8 // must be a power of 2

/*
DeBruijin lookup table.
The table cannot be inlined in the method
because it takes up extra local space.
*/

const int deBruijinLookup[] =
{
	0, 31, 9, 30, 3, 8, 13, 29, 2,
	5, 7, 21, 12, 24, 28, 19,
	1, 10, 4, 14, 6, 22, 25,
	20, 11, 15, 23, 26, 16, 27, 17, 18
};

template <typename T> int sign(T val)
{
	return (T(0) < val) - (val < T(0));
}

inline float3 minUnion(float3 data1, float3 data2)
{
	return float3(min(data1.x, data2.x),
		min(data1.y, data2.y), min(data1.z, data2.z));
}

inline float3 maxUnion(float3 data1, float3 data2)
{
	return float3(max(data1.x, data2.x),
		max(data1.y, data2.y), max(data1.z, data2.z));
}

class Sync
{
public:
	Sync(uint numThreads) : waiting(0), goal(numThreads), syncNum(0)
	{

	}

	void sync()
	{
		std::unique_lock<std::mutex> lock(mutex);
		const uint syncReuse = syncNum;
		waiting++;

		if (waiting != goal)
			var.wait(lock, [this, syncReuse]() { return syncNum != syncReuse; });
		else
		{
			waiting = 0;
			syncNum++;
			var.notify_all();
		}
	}

private:
	uint syncNum;
	uint waiting;
	uint goal;
	std::mutex mutex;
	std::condition_variable var;
};

class BVH : public Geometry
{
public:

	struct Box
	{
		float3 bbMin, bbMax;
	};

	struct Node
	{
		uint parent;
		uint childL, childR;

		uint code;

		// bounding box calc
		Box bbox;

		// index start value
		uint index;
	};

	struct LeadingPrefixRet
	{
		int64_t num;
		uint index;
	};

	uint DATA_SIZE = 127;
	uint numObjects = 0;

	Geometry* geo;

	~BVH();

	// general helpters

	uint leadingZero(unsigned int num);
	uint ceilLog2(size_t num);
	void createBVH(Geometry* geo);

	uint loadFactor;

	// morton codes
	void mortonCodes(uint3 threadID, uint groupThreadID, uint3 groupID);

	// radix sort
	uint* positionNotPresent;
	uint numOnesBuffer;

	void prefixSum(uint ID);
	void radixPart1(uint3 threadID, uint groupThreadID, uint3 groupID,
		uint radixi);
	void radixPart2(uint3 threadID, uint groupThreadID, uint3 groupID,
		uint radixi);

	// bvh construction
	std::atomic<uint>* transferBuffer;

	LeadingPrefixRet leadingPrefix(uint d1, uint d2, uint index1, uint index2);
	LeadingPrefixRet leadingPrefixBounds(uint d1, uint d1Index, uint index);
	bool lessThanPrefix(LeadingPrefixRet a, LeadingPrefixRet b);
	uint2 getChildren(uint index);
	void BVHConstructPart1(uint3 threadID, uint groupThreadID, uint3 groupID);
	void BVHConstructPart2(uint3 threadID, uint groupThreadID, uint3 groupID);

	// bvh traversal
	bool rayBoxCollision(Ray origRay, Box box, bool collision, float distance) const;
	bool findCollision(Ray origRay, isect& col) const;

	// bvh tree
	Node* BVHTree = nullptr;

	Sync sync{ NUM_THREADS };

	void GlobalMemoryBarrier()
	{
		sync.sync();
	}

	void GroupMemoryBarrierWithGroupSync()
	{
		GlobalMemoryBarrier();
	}

	Vec3f getBoundingBoxMin() {
		return geo->getBoundingBoxMin();
	}

	Vec3f getBoundingBoxMax() {
		return geo->getBoundingBoxMax();
	}

	Vec3f getMinBB(size_t index) {
		return geo->getMinBB(index);
	}

	Vec3f getMaxBB(size_t index) {
		return geo->getMaxBB(index);
	}

	Vec3f getCentroid(size_t index) {
		return geo->getCentroid(index);
	}

	Vec3f getNormal(size_t vertID) {
		return geo->getNormal(vertID);
	}

	Vec3f getPosition(size_t vertID) {
		return geo->getPosition(vertID);
	}

	Vec3f getColor(size_t vertID) {
		return geo->getColor(vertID);
	}

	size_t numberOfObjects() { return geo->numberOfObjects(); }

	size_t numberofVerts() { return geo->numberofVerts(); };

	bool intersectLocal(Ray ray, isect& isect, size_t index) {
		return geo->intersectLocal(ray, isect, index);
	}
};

#endif