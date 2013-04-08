#include "KdTree.h"
#include "Shape.h"
#include "Mesh.h"

namespace Purple {


struct BoundEdge
{
	// BoundEdge Public Methods
	BoundEdge() { }
	BoundEdge(float tt, int pn, bool starting) {
		t = tt;
		primNum = pn;
		type = starting ? START : END;
	}
	bool operator<(const BoundEdge &e) const {
		if (t == e.t)
			return (int)type < (int)e.type;
		else return t < e.t;
	}
	float t;
	int primNum;
	enum { START, END } type;
};

//-----------------------------------------------------------------------------------------
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
	
	uint32_t* indices = new uint32_t[primCount];

	// Cache prim bounds
	vector<BoundingBoxf> primBounds;
	primBounds.reserve(primCount);
	for (uint32_t i = 0; i < GetPrimitiveCount(); ++i) 
	{
		indices[i] = i;
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
	idx -= *it;
	return (uint32_t) (it - mShapeMap.begin());
}


}