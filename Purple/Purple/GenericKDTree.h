#ifndef GeneralKDTree_h__
#define GeneralKDTree_h__

#include "Prerequisites.h"
#include "BoundingBox.hpp"

namespace Purple {

#define MAX_KD_DEPTH 48


template <typename BoundType> 
class KDTreeBase
{
public:

	struct KDNode 
	{
		union
		{
			/* Inner node */
			struct
			{
				/* Bit layout:
				   31   : False (inner node)
				   30   : Indirection node flag
				   29-3 : Offset to the left child
				          or indirection table entry
				   2-0  : Split axis
				*/
				uint32_t combined;

				/// Split plane coordinate
				float split;
			} inner;

			/* Leaf node */
			struct
			{
				/* Bit layout:
				   31   : True (leaf node)
				   30-0 : Offset to the node's primitive list
				*/
				uint32_t combined;

				/// End offset of the primitive list
				uint32_t end;
			} leaf;
		};

		enum EMask {
			ETypeMask = 1 << 31,
				EIndirectionMask = 1 << 30,
				ELeafOffsetMask = ~ETypeMask,
				EInnerAxisMask = 0x3,
				EInnerOffsetMask = ~(EInnerAxisMask + EIndirectionMask),
				ERelOffsetLimit = (1<<28) - 1
		};

		inline void InitLeafNode(uint32_t offset, uint32_t numPrims)
		{
			leaf.combined = (uint32_t) ETypeMask | offset;
			leaf.end = offset + numPrims;
		}

		inline bool InitInnerNode(int axis, float split, ptrdiff_t relOffset)
		{
			if (relOffset < 0 || relOffset > ERelOffsetLimit)
				return false;
			inner.combined = axis | ((uint32_t) relOffset << 2);
			inner.split = split;
			return true;
		}
	};

	/// Return the root node of the kd-tree
	inline const KDNode *GetRoot() const  { return mNodes; }

	/// Return whether or not the kd-tree has been built
	inline bool IsBuilt() const { return mNodes != NULL; }

	/// Return a (slightly enlarged) axis-aligned bounding box containing all primitives
	inline const BoundType &GetAABB() const { return mAABB; }

	/// Return a tight axis-aligned bounding box containing all primitives
	inline const BoundType &GetTightAABB() const { return mTightAABB;}


protected:
	virtual ~KDTreeBase() {}

protected:
	KDNode* mNodes;
	BoundType mAABB, mTightAABB;
};


template <typename BoundType, typename TreeConstructionHeuristic, typename Derived>
class GenericKDTree : public KDTreeBase<BoundType> 
{
public:
	GenericKDTree() : mIndices(NULL)
	{
		mNodes = NULL;
		mTraversalCost = 15;
		mQueryCost = 20;
		mEmptySpaceBonus = 0.9f;
	}

	inline void     SetTraversalCost(float traversalCost)          { mTraversalCost = traversalCost; }
	inline float    GetTraversalCost() const                       { return mTraversalCost; }

	inline void     SetQueryCost(float queryCost)                  { mQueryCost = queryCost; }
	inline float    GetQueryCost() const                           { return mQueryCost; }

	inline void     SetEmptySpaceBonus(float emptySpaceBonus)      { mEmptySpaceBonus = emptySpaceBonus; }
	inline float    GetEmptySpaceBonus() const                     { return mEmptySpaceBonus; }

	inline void     SetMaxDepth(uint32_t maxDepth)                 { mMaxDepth = maxDepth; }
	inline uint32_t GetMaxDepth() const                            { return mMaxDepth; }

protected:
	/**
	 * \brief Build a KD-tree over the supplied geometry
	 *
	 * To be called by the subclass.
	 */
	void BuildInternal() 
	{
		if (IsBuilt())
			return;

		/* Establish an ad-hoc depth cutoff value (Formula from PBRT) */
		uint32_t primCount = Cast()->GetPrimitiveCount();
		if (mMaxDepth == 0)
			mMaxDepth = (uint32_t) (8 + 1.3f * std::log((float) primCount)/std::log(2.0f));

		mMaxDepth = std::min(mMaxDepth, uint32_t(MAX_KD_DEPTH));


		uint32_t* indices = new uint32_t[primCount];
		
		// Compute bound
		for (uint32_t i=0; i < primCount; ++i)
		{
			mAABB = Merge(mAABB, Cast()->GetAABB(i));
			indices[i] = i;
		}
	}


	
protected:
	
	/// Cast to the derived class
	inline Derived* Cast()             { return static_cast<Derived *>(this); }

	/// Cast to the derived class (const version)
	inline const Derived* Cast() const { return static_cast<const Derived *>(this); }

protected:
	uint32_t* mIndices;
	float mTraversalCost;
	float mQueryCost;
	float mEmptySpaceBonus;

	bool m_clip, m_retract, m_parallelBuild;
	uint32_t mMaxDepth;
	uint32_t m_stopPrims;
	/*uint32_t m_maxBadRefines;
	uint32_t m_exactPrimThreshold;
	uint32_t m_minMaxBins;
	uint32_t m_nodeCount;
	uint32_t m_indexCount;
	std::vector<TreeBuilder *> m_builders;
	std::vector<KDNode *> m_indirections;
	ref<Mutex> m_indirectionLock;
	BuildInterface m_interface;*/
};

class SurfaceAreaHeuristic3 
{
	
};



}



#endif // GeneralKDTree_h__
