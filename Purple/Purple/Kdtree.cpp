#include "KdTree.h"
#include "Shape.h"
#include "Mesh.h"
#include <aligned_allocator.h>

namespace Purple {

using namespace RxLib;

// KdTreeAccel Local Declarations
struct KDNode 
{
	// KdAccelNode Methods
	void InitLeaf(uint32_t* primNums, int32_t np, MemoryArena &arena)
	{
		flags = 3;
		nPrims |= (np << 2);
		// Store primitive ids for leaf node
		if (np == 0)
			onePrimitive = 0;
		else if (np == 1)
			onePrimitive = primNums[0];
		else {
			primitives = arena.Alloc<uint32_t>(np);
			for (int i = 0; i < np; ++i)
				primitives[i] = primNums[i];
		}
	}

	void InitInterior(uint32_t axis, uint32_t ac, float s) 
	{
		split = s;
		flags = axis;
		aboveChild |= (ac << 2);
	}
	float SplitPos() const { return split; }
	uint32_t Primitives() const { return nPrims >> 2; }
	uint32_t SplitAxis() const { return flags & 3; }
	bool IsLeaf() const { return (flags & 3) == 3; }
	uint32_t AboveChild() const { return aboveChild >> 2; }
	union 
	{
		float split;            // Interior
		uint32_t onePrimitive;  // Leaf
		uint32_t *primitives;   // Leaf
	};

private:
	union
	{
		uint32_t flags;         // Both
		uint32_t nPrims;        // Leaf
		uint32_t aboveChild;    // Interior
	};
};

struct BoundEdge
{
	// BoundEdge Public Methods
	BoundEdge() { }
	BoundEdge(float tt, uint32_t pn, bool starting)
	{
		t = tt;
		primNum = pn;
		type = starting ? START : END;
	}
	bool operator<(const BoundEdge &e) const
	{
		if (t == e.t)
			return (int)type < (int)e.type;
		else return t < e.t;
	}
	float t;
	uint32_t primNum;
	enum { START, END } type;
};

//-----------------------------------------------------------------------------------------
KDTree::KDTree( float icost /*= 80.0f*/, float tcost /*= 1.0f*/, float ebonus /*= 0.5f*/, int maxp /*= 1*/, int maxDepth /*= -1*/ )
	:mIntersectCost(icost), mTraversalCost(tcost), mEmptySpaceBonus(ebonus), mMaxPrimitive(maxp), mMaxDepth(mMaxDepth)
{

}

KDTree::~KDTree()
{
	aligned_free(mNodes);
}

void KDTree::AddShape( const shared_ptr<Shape>& shape )
{
	assert(!IsBuilt());
	const Mesh* mesh = shape->GetTriangleMesh();

	if (mesh)
	{
		// Triangle meshes are expanded into individual primitives,
		// which are visible to the tree construction code. Generic
		// primitives are only handled by their AABBs
		mShapeMap.push_back(mShapeMap.back() + mesh->GetTriangleCount());
		mTriangleFlag.push_back(true);
	} 
	else
	{
		mShapeMap.push_back(1);
		mTriangleFlag.push_back(false);
	}
	
	mShapes.push_back(shape);
}

uint32_t KDTree::GetPrimitiveCount() const
{
	return mShapeMap[mShapeMap.size()-1];
}

void KDTree::BuildTree()
{
	uint32_t primCount = GetPrimitiveCount();
	
	// Compute bounds for kd-tree construction
	uint32_t* primNums = new uint32_t[primCount];
	vector<BoundingBoxf> primBounds;
	primBounds.reserve(primCount);
	for (uint32_t i = 0; i < GetPrimitiveCount(); ++i) 
	{
		primNums[i] = i;
		BoundingBoxf b = GetBound(i);
		mBound = Merge(mBound, b);
		primBounds.push_back(b);
	}

	// Allocate working memory for kd-tree construction
	BoundEdge *edges[3];
	for (int i = 0; i < 3; ++i)
		edges[i] = new BoundEdge[2*primCount];
	uint32_t *prims0 = new uint32_t[primCount];
	uint32_t *prims1 = new uint32_t[(mMaxDepth+1) *primCount];

	mNextFreeNode = mNumAllocedNodes = 0;

	// Start recursive construction of kd-tree
	BuildInternal(0, mBound, primBounds, primNums, primCount, mMaxDepth, edges, prims0, prims1);

	// Free working memory for kd-tree construction
	delete[] primNums;
	for (int i = 0; i < 3; ++i)
		delete[] edges[i];
	delete[] prims0;
	delete[] prims1;

}

void KDTree::BuildInternal( int32_t nodeNum, const BoundingBoxf& nodeBounds, const vector<BoundingBoxf>& allPrimBounds, uint32_t* primNums, int32_t nPrimitives, int32_t depth, BoundEdge* edges[3], uint32_t* prims0, uint32_t* prims1, int badRefines )
{
	// Get next free node from _nodes_ array
	if (mNextFreeNode == mNumAllocedNodes)
	{
		int32_t nAlloc = std::max(2 * mNumAllocedNodes, 512);
		KDNode* n = (KDNode*)aligned_malloc(sizeof(KDNode) * nAlloc, 64); 

		if (nAlloc > 0) 
		{
			memcpy(n, mNodes, mNumAllocedNodes * sizeof(KDNode));
			aligned_free(mNodes);
		}
		mNodes = n;
		mNumAllocedNodes = nAlloc;
	}
	++mNextFreeNode;

	if (nPrimitives <= mMaxPrimitive || depth == 0)
	{
		// Init leaf
		mNodes[nodeNum].InitLeaf(primNums, nPrimitives, mArena);
		return;
	}

	// Initialize interior node and continue recursion
	int bestAxis = -1, bestOffset = -1;
	float bestCost = Mathf::INFINITY;
	float oldCost = mIntersectCost * float(nPrimitives);
	float totalSA = nodeBounds.SurfaceArea();
	float invTotalSA = 1.f / totalSA;
	float3 d = nodeBounds.Max - nodeBounds.Min;

	// Choose which axis to split along
	int32_t axis = nodeBounds.MaximumExtent();
	int retries = 0;

retrySplit:

	// Initialize edges for axis
	for (int32_t i = 0; i < nPrimitives; ++i)
	{
		uint32_t pn = primNums[i];
		const BoundingBoxf& bbox = allPrimBounds[pn];
		edges[axis][2*i] =   BoundEdge(bbox.Min[axis], pn, true);
		edges[axis][2*i+1] = BoundEdge(bbox.Max[axis], pn, false);
	}
	std::sort(&edges[axis][0], &edges[axis][2*nPrimitives]);

	// Compute cost of all splits for axis to find best
	int32_t nBelow = 0, nAbove = nPrimitives;
	for (int32_t i = 0; i < 2 * nPrimitives; ++i)
	{
		if (edges[axis][i].type == BoundEdge::END)
			nAbove--;

		float edget = edges[axis][i].t;

		if (edget > nodeBounds.Min[axis] && edget < nodeBounds.Max[axis])
		{
			int32_t otherAxis0 = (axis + 1) % 3, otherAxis1 = (axis + 2) % 3;

			float belowSA = 2.0f * (d[otherAxis0] * d[otherAxis1] + (edget - nodeBounds.Min[axis]) * (d[otherAxis0] + d[otherAxis1]));
			float aboveSA = 2.0f * (d[otherAxis0] * d[otherAxis1] + (nodeBounds.Max[axis] - edget) * (d[otherAxis0] + d[otherAxis1]));

			float pBelow = belowSA * invTotalSA;
			float pAbove = aboveSA * invTotalSA;
			
			float eb = (nAbove == 0 || nBelow == 0) ? mEmptySpaceBonus : 0.0f;

			float cost = mTraversalCost +
				mIntersectCost * (1.0f - eb) * (pBelow * nBelow + pAbove * nAbove);

			if (cost < bestCost)
			{
				bestCost = cost;
				bestAxis = axis;
				bestOffset = i;
			}
		}


		if (edges[axis][i].type == BoundEdge::START)
			nBelow++;
	}
	assert(nBelow == nPrimitives && nAbove == 0);

	// Create leaf if no good splits were found
	if (bestAxis == -1 && retries < 2) 
	{
		++retries;
		axis = (axis+1) % 3;
		goto retrySplit;
	}

	if (bestCost > oldCost) 
		++badRefines;

	if ((bestCost > 4.0f * oldCost && nPrimitives < 16) || bestAxis == -1 || badRefines == 3) 
	{
		mNodes[nodeNum].InitLeaf(primNums, nPrimitives, mArena);
		return;
	}

	int32_t n0 = 0, n1 = 0;
	for (int32_t i = 0; i < bestOffset; ++i)
		if (edges[bestAxis][i].type == BoundEdge::START)
			prims0[n0++] = edges[bestAxis][i].primNum;
	for (int i = bestOffset+1; i < 2*nPrimitives; ++i)
		if (edges[bestAxis][i].type == BoundEdge::END)
			prims1[n1++] = edges[bestAxis][i].primNum;

	// Recursively initialize children nodes
	float tsplit = edges[bestAxis][bestOffset].t;

	BoundingBoxf bounds0 = nodeBounds, bounds1 = nodeBounds;
	bounds0.Max[bestAxis] = bounds1.Min[bestAxis] = tsplit;
	
	BuildInternal(nodeNum+1, bounds0,
		allPrimBounds, prims0, n0, depth-1, edges,
		prims0, prims1 + nPrimitives, badRefines);
	
	uint32_t aboveChild = mNextFreeNode;
	mNodes[nodeNum].InitInterior(bestAxis, aboveChild, tsplit);
	BuildInternal(aboveChild, bounds1, allPrimBounds, prims1, n1,
		depth-1, edges, prims0, prims1 + nPrimitives, badRefines);
}

BoundingBoxf KDTree::GetBound( uint32_t idx ) const
{
	uint32_t meshIdx = FindShape(idx);

	const Mesh* mesh = mShapes[idx]->GetTriangleMesh();

	if (mesh)
	{
		return mesh->GetWorldBound(idx);
	}
	else
		return mShapes[meshIdx]->GetWorldBound();
}

uint32_t KDTree::FindShape( uint32_t& idx ) const
{
	auto it = std::lower_bound(mShapeMap.begin(), mShapeMap.end(), idx+1) - 1;

	// Compute triangle index in shape. 
	idx -= *it;

	// if it is not a mesh, directly use mesh index to get primitive
	return (uint32_t) (it - mShapeMap.begin());
}

}