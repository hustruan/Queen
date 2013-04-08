#ifndef KdTree_h__
#define KdTree_h__

#include "Prerequisites.h"
#include "GenericKDTree.h"

namespace Purple {


struct KDNode;

class KDTree 
{
public:
	KDTree();
	~KDTree();

	const BoundingBoxf& WorldBound() const                         { return mBound; }

	inline void     SetTraversalCost(float traversalCost)          { mTraversalCost = traversalCost; }
	inline float    GetTraversalCost() const                       { return mTraversalCost; }

	inline void     SetQueryCost(float queryCost)                  { mQueryCost = queryCost; }
	inline float    GetQueryCost() const                           { return mQueryCost; }

	inline void     SetEmptySpaceBonus(float emptySpaceBonus)      { mEmptySpaceBonus = emptySpaceBonus; }
	inline float    GetEmptySpaceBonus() const                     { return mEmptySpaceBonus; }

	inline void     SetMaxDepth(uint32_t maxDepth)                 { mMaxDepth = maxDepth; }
	inline uint32_t GetMaxDepth() const                            { return mMaxDepth; }

	void AddShape(const shared_ptr<Shape>& shape);

	uint32_t GetPrimitiveCount() const;

	void BuildTree();

protected:
	inline bool IsBuilt() const  { return mNodes != NULL; }
	BoundingBoxf GetBound(uint32_t index) const;
	uint32_t FindShape( uint32_t& idx ) const;

protected:

	KDNode* mNodes;

	BoundingBoxf mBound;

	std::vector<shared_ptr<Shape> > mShapes;
	std::vector<uint32_t> mShapeMap;
	std::vector<bool> mTriangleFlag;

	uint32_t* mIndices;
	float mTraversalCost;
	float mQueryCost;
	float mEmptySpaceBonus;
	uint32_t mMaxDepth;



};



}



#endif // KdTree_h__
