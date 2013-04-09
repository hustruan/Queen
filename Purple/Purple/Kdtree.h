#ifndef KdTree_h__
#define KdTree_h__

#include "Prerequisites.h"
#include "MemoryArena.h"
#include "Ray.h"

namespace Purple {

struct KDNode;
struct BoundEdge;

class KDTree 
{
public:
	KDTree(float icost = 80.0f, float tcost = 1.0f,  float ebonus = 0.5f, int maxp = 1, int maxDepth = -1);
	~KDTree();

	const BoundingBoxf& WorldBound() const                         { return mBound; }

	inline void     SetTraversalCost(float traversalCost)          { mTraversalCost = traversalCost; }
	inline float    GetTraversalCost() const                       { return mTraversalCost; }

	inline void     SetQueryCost(float queryCost)                  { mIntersectCost = queryCost; }
	inline float    GetQueryCost() const                           { return mIntersectCost; }

	inline void     SetEmptySpaceBonus(float emptySpaceBonus)      { mEmptySpaceBonus = emptySpaceBonus; }
	inline float    GetEmptySpaceBonus() const                     { return mEmptySpaceBonus; }

	inline void     SetMaxDepth(int32_t maxDepth)                 { mMaxDepth = maxDepth; }
	inline int32_t  GetMaxDepth() const                            { return mMaxDepth; }

	void AddShape(const shared_ptr<Shape>& shape);

	uint32_t GetPrimitiveCount() const;

	void BuildTree();

	bool Intersect(const Ray& ray, Intersection* isect) const;

protected:
	inline bool IsBuilt() const  { return mNodes != NULL; }
	BoundingBoxf GetBound(uint32_t index) const;
	uint32_t FindShape( uint32_t& idx ) const;

	
	void BuildInternal(int32_t nodeNum, const BoundingBoxf& nodeBounds, const vector<BoundingBoxf>& allPrimBounds, uint32_t* primNums,
		int32_t nPrimitives, int32_t depth, BoundEdge* edges[3], uint32_t* prims0, uint32_t* prims1, int32_t badRefines = 0);

protected:

	KDNode* mNodes;

	BoundingBoxf mBound;

	std::vector<shared_ptr<Shape> > mShapes;
	std::vector<uint32_t> mShapeMap;
	std::vector<bool> mTriangleFlag;

	uint32_t* mIndices;

	float mTraversalCost;
	float mIntersectCost;
	float mEmptySpaceBonus;
	int32_t mMaxDepth;
	int32_t mMaxPrimitive;


	int32_t mNextFreeNode;
	int32_t mNumAllocedNodes;

	MemoryArena mArena;
};



}



#endif // KdTree_h__
