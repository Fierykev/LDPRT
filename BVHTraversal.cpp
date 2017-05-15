#include <stack>

#include <algorithm>
#include "vec.h"
#include "BVH.h"

using namespace std;

/*
The slab method is used for collision.
*/

bool BVH::rayBoxCollision(Ray origRay, Box box, bool collision, float distance) const
{
	const float3 deltaMin = vecToFloat(((Vec3f)box.bbMin - origRay.origin) * origRay.invDirection);
	const float3 deltaMax = vecToFloat(((Vec3f)box.bbMax - origRay.origin) * origRay.invDirection);

	const float3 minVector = float3(
		min(deltaMin.x, deltaMax.x),
		min(deltaMin.y, deltaMax.y),
		min(deltaMin.z, deltaMax.z)
	);
	const float3 maxVector = float3(
		max(deltaMin.x, deltaMax.x),
		max(deltaMin.y, deltaMax.y),
		max(deltaMin.z, deltaMax.z)
	);

	const float minVal = max(max(minVector.x, minVector.y), minVector.z);
	const float maxVal = min(min(maxVector.x, maxVector.y), maxVector.z);

	return 0 <= maxVal && minVal <= maxVal && (!collision || minVal <= distance);
}

bool BVH::findCollision(Ray origRay, isect& col) const
{
	bool globalCollision = false;

	// create the stack vars
	std::stack<uint> stack;
	stack.push(MAX_UTYPE);

	// start from the root
	uint nodeID = numObjects;

	// tmp vars
	uint leftChildNode, rightChildNode;

	bool leftChildCollision, rightChildCollision;
	uint leftLeaf, rightLeaf;

	float distance;

	uint index;

	isect tmpCol;

	do
	{
		// record the index of the children
		leftChildNode = BVHTree[nodeID].childL;
		rightChildNode = BVHTree[nodeID].childR;

		//cout << nodeID << " " << leftChildNode << " " << rightChildNode << " " << MAX_UTYPE << endl;

		// check for leaf node
		if (leftChildNode == MAX_UTYPE && rightChildNode == MAX_UTYPE)
		{
			// get the index
			index = BVHTree[nodeID].index;

			//  update the triangle
			if (index != MAX_UTYPE && geo->intersectLocal(origRay, tmpCol, index) &&
				(!globalCollision || tmpCol.t < col.t))
			{
				// set collision to true
				globalCollision = true;

				// copy over the data
				col = tmpCol;
			}

			// remove the stack top
			nodeID = stack.top();
			stack.pop();

			continue;
		}

		// check for collisions with children
		leftChildCollision = rayBoxCollision(origRay, BVHTree[leftChildNode].bbox, globalCollision, col.t);
		rightChildCollision = rayBoxCollision(origRay, BVHTree[rightChildNode].bbox, globalCollision, col.t);

		if (!leftChildCollision && !rightChildCollision)
		{
			// remove the stack top
			nodeID = stack.top();
			stack.pop();
		}
		else
		{
			// store the right node on the stack if both right and left
			// boxes intersect the ray
			if (leftChildCollision && rightChildCollision)
				stack.push(rightChildNode);

			// update the nodeID depending on if the left side intersected
			nodeID = leftChildCollision ? leftChildNode : rightChildNode;
		}
	} while (stack.size() != 0);

	return globalCollision;
}