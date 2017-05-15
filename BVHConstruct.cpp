#include "BVH.h"

#include <algorithm>

using namespace std;

/*
BVH Construction is based on:
http://devblogs.nvidia.com/parallelforall/wp-content/uploads/2012/11/karras2012hpg_paper.pdf
*/

/*
Gets number of leeading zeros by representing them
as ones pushed to the right.  Does not give meaningful
number but the relative output is correct.
*/

BVH::LeadingPrefixRet BVH::leadingPrefix(uint d1, uint d2, uint index1, uint index2)
{
	uint64_t data = ((d1 ^ d2) << 32) | (index1 ^ index2);

	unsigned long num = 0;
	if (_BitScanReverse64(&num, data))
		num = 63 - num;
	else
		num = 64;
		
	//cout << d1 << " " << d2 << " " << num << endl;
	LeadingPrefixRet lpr;

	lpr.num = num;
	lpr.index = index2;

	return lpr;

	/*
	// use the index as a tie breaker if they are the same
	uint32_t data = d1 ^ d2;

	// remove non-leading zeros

	data |= data >> 1;
	data |= data >> 2;
	data |= data >> 4;
	data |= data >> 8;
	data |= data >> 16;
	data++;

	LeadingPrefixRet lpr;

	lpr.num = data ? deBruijinLookup[data * 0x076be629 >> 27] : 32;
	lpr.index = index2;

	return lpr;*/
}

/*
Same as leadingPrefix but does bounds checks.
*/

BVH::LeadingPrefixRet BVH::leadingPrefixBounds(uint d1, uint d1Index, uint index)
{
	LeadingPrefixRet lprERR;
	lprERR.num = -1;
	lprERR.index = 0;

	if (0 <= index && index < numObjects)
		return leadingPrefix(d1, BVHTree[index].code, d1Index, index);
	else
		return lprERR;
}

/*
Check if a < b.
*/

bool BVH::lessThanPrefix(LeadingPrefixRet a, LeadingPrefixRet b)
{
	return (a.num < b.num);// || (a.num == 32 && b.num == 32 && a.index < b.index);
}

/*
Find the children of the node.
*/

uint2 BVH::getChildren(uint index)
{
	uint codeCurrent = BVHTree[index].code;

	// get range direction
	int64_t direction = lessThanPrefix(leadingPrefixBounds(codeCurrent, index, index + 1),
		leadingPrefixBounds(codeCurrent, index, index - 1)) ? -1 : 1;

	// get upper bound of length range
	LeadingPrefixRet minLeadingZero =
		leadingPrefixBounds(codeCurrent, index, index - direction);
	uint boundLen = 2;

	// TODO: possibly change back to multiply by 4
	for (;
		lessThanPrefix(minLeadingZero, leadingPrefixBounds(
			codeCurrent, index, index + boundLen * direction));
		boundLen <<= 1) {
	}

	// find lower bound
	int64_t delta = boundLen;

	int64_t deltaSum = 0;

	do
	{
		delta = (delta + 1) >> 1;

		if (lessThanPrefix(minLeadingZero,
			leadingPrefixBounds(
				codeCurrent, index, index + (deltaSum + delta) * direction)))
			deltaSum += delta;
	} while (1 < delta);
	
	uint boundStart = index + deltaSum * direction;
	
	// find slice range
	LeadingPrefixRet leadingZero = leadingPrefixBounds(codeCurrent, index, boundStart);

	delta = deltaSum;
	int64_t tmp = 0;

	do
	{
		delta = (delta + 1) >> 1;

		if (lessThanPrefix(leadingZero,
			leadingPrefixBounds(codeCurrent, index, index + (tmp + delta) * direction)))
			tmp += delta;
	} while (1 < delta);

	uint location = index + tmp * direction + min(direction, 0LL);

	uint2 children;

	if (min(index, boundStart) == location)
		children.x = location;
	else
		children.x = location + numObjects;

	if (max(index, boundStart) == location + 1)
		children.y = location + 1;
	else
		children.y = location + 1 + numObjects;

	return children;
}

void BVH::BVHConstructPart1(uint3 threadID, uint groupThreadID, uint3 groupID)
{
	// should only cause the last thread to diverge
	if (threadID.x < numObjects - 1)
	{
		// construct the tree
		uint2 children = getChildren(threadID.x);

		// set the children

		BVHTree[threadID.x + numObjects].childL = children.x;
		BVHTree[threadID.x + numObjects].childR = children.y;

		// set the parent
		BVHTree[children.x].parent = threadID.x + numObjects;
		BVHTree[children.y].parent = threadID.x + numObjects;
	}
	else // set the parent of the root node to MAX_UTYPE
		BVHTree[numObjects].parent = MAX_UTYPE;
}

/**
WARNING THE BELOW CODE IS HIGHLY DIVERGENT.
MAY WANT TO LOOK FOR A LESS DIVERGENT ALGORITHM.
**/

void BVH::BVHConstructPart2(uint3 threadID, uint groupThreadID, uint3 groupID)
{
	uint nodeID = BVHTree[threadID.x].parent;

	uint value;

	// atomically add to prevent two threads from entering a node
	value = transferBuffer[nodeID - numObjects].fetch_add(1) + 1;

	while (value)
	{
		// compute the union of the bounding boxes
		BVHTree[nodeID].bbox.bbMin = minUnion(BVHTree[BVHTree[nodeID].childL].bbox.bbMin,
			BVHTree[BVHTree[nodeID].childR].bbox.bbMin);

		BVHTree[nodeID].bbox.bbMax = maxUnion(BVHTree[BVHTree[nodeID].childL].bbox.bbMax,
			BVHTree[BVHTree[nodeID].childR].bbox.bbMax);

		// get the parent
		uint tmp = nodeID;
		nodeID = BVHTree[nodeID].parent;

		// atomically add to prevent two threads from entering a node
		if (nodeID != MAX_UTYPE)
		{
			if (nodeID < numObjects)
			{
				cout << "ERROR: BVH CONSTRUCTION INTERNAL FAILURE" << endl;
				system("PAUSE");
				exit(-1);
			}

			value = transferBuffer[nodeID - numObjects].fetch_add(1) + 1;
		}
		else
			return;
	}
}