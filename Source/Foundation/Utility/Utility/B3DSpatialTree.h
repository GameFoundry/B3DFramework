//************************************ B3D Framework - Copyright 2014 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DMath.h"
#include "Math/B3DVector4I.h"
#include "Math/B3DSIMD.h"
#include "Allocators/B3DPoolAlloc.h"

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

	/** Traits to help with specialization of the spatial tree for different dimensions. */
	template<int Dimension>
	struct TSpatialTreeTraits
	{
		static_assert(sizeof(TSpatialTreeTraits) == -1, "Unsupported dimension count.");
	};

	/** Traits for 1D tree. */
	template<>
	struct TSpatialTreeTraits<1>
	{
		using SIMDBoundsType = simd::Range;
		using ScalarBoundsType = Range;
		using CenterType = float;
		static constexpr u32 kChildNodeCount = 2;
	};

	/** Traits for quad-tree. */
	template<>
	struct TSpatialTreeTraits<2>
	{
		using SIMDBoundsType = simd::Area2;
		using ScalarBoundsType = Area2;
		using CenterType = Vector2;
		static constexpr u32 kChildNodeCount = 4;
	};

	/** Traits for oct-tree. */
	template<>
	struct TSpatialTreeTraits<3>
	{
		using SIMDBoundsType = simd::AABox;
		using ScalarBoundsType = AABox;
		using CenterType = Vector3;
		static constexpr u32 kChildNodeCount = 8;
	};

	/** Identifier that may be used for finding an element in the spatial tree. */
	class SpatialTreeElementId
	{
	public:
		SpatialTreeElementId() = default;

		SpatialTreeElementId(void* node, u32 indexInNode)
			: Node(node), IndexInNode(indexInNode)
		{}

		bool IsValid() const { return Node != nullptr && IndexInNode != ~0u; }

	private:
		template <typename, typename, u32>
		friend class TSpatialTree;

		void* Node = nullptr;
		u32 IndexInNode = ~0u;
	};

	/** Contains a reference to one child nodes in a spatial tree node. */
	template<int Dimension>
	struct TSpatialTreeChildNodeId
	{
		static_assert(sizeof(TSpatialTreeChildNodeId) == -1, "Unsupported dimension count.");
	};

	/** Specialization of TSpatialTreeChildNodeId for a 1D tree. */
	template<>
	struct TSpatialTreeChildNodeId<1>
	{
		union
		{
			struct
			{
				u32 X : 1;
				u32 Empty : 1;
			};

			struct
			{
				u32 Index : 1;
				u32 Empty2 : 1;
			};
		};

		TSpatialTreeChildNodeId()
			: Empty(true)
		{}

		TSpatialTreeChildNodeId(u32 index)
			: Index(index), Empty2(false)
		{}
	};

	/** Specialization of TSpatialTreeChildNodeId for a quad-tree. */
	template<>
	struct TSpatialTreeChildNodeId<2>
	{
		union
		{
			struct
			{
				u32 X : 1;
				u32 Y : 1;
				u32 Empty : 1;
			};

			struct
			{
				u32 Index : 2;
				u32 Empty2 : 1;
			};
		};

		TSpatialTreeChildNodeId()
			: Empty(true)
		{}

		TSpatialTreeChildNodeId(u32 index)
			: Index(index), Empty2(false)
		{}
	};

	/** Specialization of TSpatialTreeChildNodeId for a oct-tree. */
	template<>
	struct TSpatialTreeChildNodeId<3>
	{
		union
		{
			struct
			{
				u32 X : 1;
				u32 Y : 1;
				u32 Z : 1;
				u32 Empty : 1;
			};

			struct
			{
				u32 Index : 3;
				u32 Empty2 : 1;
			};
		};

		TSpatialTreeChildNodeId()
			: Empty(true)
		{}

		TSpatialTreeChildNodeId(u32 index)
			: Index(index), Empty2(false)
		{}
	};

	/** Contains a range of child nodes in a spatial tree node (any or all of the possible child nodes). */
	template<int Dimension>
	struct TSpatialTreeChildNodeIdRange
	{
		static_assert(sizeof(TSpatialTreeChildNodeIdRange) == -1, "Unsupported dimension count.");
	};

	/** Specialization of TSpatialTreeChildNodeIdRange for a 1D tree. */
	template<>
	struct TSpatialTreeChildNodeIdRange<1>
	{
		enum { Dimension = 1};

		union
		{
			struct
			{
				u32 PositiveX : 1;
				u32 NegativeX : 1;
			};

			struct
			{
				u32 PositiveBits : 1;
				u32 NegativeBits : 1;
			};

			u32 AllBits : 2;
		};

		/** Constructs a range overlapping no nodes. */
		TSpatialTreeChildNodeIdRange()
			: AllBits(0)
		{}

		/** Constructs a range overlapping a single node. */
		TSpatialTreeChildNodeIdRange(TSpatialTreeChildNodeId<Dimension> child)
			: PositiveBits(child.Index), NegativeBits(~child.Index)
		{}

		/** Checks if the range contains the provided child. */
		bool Contains(TSpatialTreeChildNodeId<Dimension> child) const
		{
			TSpatialTreeChildNodeIdRange childRange(child);
			return (AllBits & childRange.AllBits) == childRange.AllBits;
		}
	};

	/** Specialization of TSpatialTreeChildNodeIdRange for a quad-tree. */
	template<>
	struct TSpatialTreeChildNodeIdRange<2>
	{
		enum { Dimension = 2};

		union
		{
			struct
			{
				u32 PositiveX : 1;
				u32 PositiveY : 1;
				u32 NegativeX : 1;
				u32 NegativeY : 1;
			};

			struct
			{
				u32 PositiveBits : 2;
				u32 NegativeBits : 2;
			};

			u32 AllBits : 4;
		};

		/** Constructs a range overlapping no nodes. */
		TSpatialTreeChildNodeIdRange()
			: AllBits(0)
		{}

		/** Constructs a range overlapping a single node. */
		TSpatialTreeChildNodeIdRange(TSpatialTreeChildNodeId<Dimension> child)
			: PositiveBits(child.Index), NegativeBits(~child.Index)
		{}

		/** Checks if the range contains the provided child. */
		bool Contains(TSpatialTreeChildNodeId<Dimension> child) const
		{
			TSpatialTreeChildNodeIdRange childRange(child);
			return (AllBits & childRange.AllBits) == childRange.AllBits;
		}
	};

	/** Specialization of TSpatialTreeChildNodeIdRange for a oct-tree. */
	template<>
	struct TSpatialTreeChildNodeIdRange<3>
	{
		enum { Dimension = 3};

		union
		{
			struct
			{
				u32 PositiveX : 1;
				u32 PositiveY : 1;
				u32 PositiveZ : 1;
				u32 NegativeX : 1;
				u32 NegativeY : 1;
				u32 NegativeZ : 1;
			};

			struct
			{
				u32 PositiveBits : 3;
				u32 NegativeBits : 3;
			};

			u32 AllBits : 6;
		};

		/** Constructs a range overlapping no nodes. */
		TSpatialTreeChildNodeIdRange()
			: AllBits(0)
		{}

		/** Constructs a range overlapping a single node. */
		TSpatialTreeChildNodeIdRange(TSpatialTreeChildNodeId<Dimension> child)
			: PositiveBits(child.Index), NegativeBits(~child.Index)
		{}

		/** Checks if the range contains the provided child. */
		bool Contains(TSpatialTreeChildNodeId<Dimension> child) const
		{
			TSpatialTreeChildNodeIdRange childRange(child);
			return (AllBits & childRange.AllBits) == childRange.AllBits;
		}
	};

	/**
	 * Spatial partitioning tree for 1D/2D/3D space (i.e. quadtree for 2D, octree for 3D).
	 *
	 * @tparam	ElementType	Type of elements to be stored in the tree.
	 * @tparam	Options		Class that controls various options of the tree. It must provide the following enums:
	 *							- LoosePadding: Denominator used to determine how much padding to add to each child node.
	 *											The extra padding percent is determined as (1.0f / LoosePadding). Larger
	 *											padding ensures elements are less likely to get stuck on a higher node
	 *											due to them straddling the boundary between the nodes.
	 *							- MinimumElementsPerNode: Determines at which point should node's children be removed and moved
	 *													  back into the parent (node is collapsed). This can occur on element
	 *													  removal, when the element count drops below the specified number.
	 *							- MaximumElementsPerNode: Determines at which point should a node be split into child nodes.
	 *													  If an element counter moves past this number the elements will be
	 *													  added to child nodes, if possible. If a node is already at maximum
	 *													  depth, this is ignored.
	 *							- MaximumDepth: Maximum depth of nodes in the tree. Nodes at this depth will not be subdivided
	 *											even if they element counts go past MaximumElementsPerNode.
	 *						It must also provide the following methods:
	 *							- "static typename TSpatialTreeTraits<Dimension>::SIMDBoundsType GetBounds(const ElementType&, void*)"
	 *								- Returns the bounds for the provided element
	 *							- "static void SetElementId(const TSpatialTreeElementId&, void*)"
	 *								- Gets called when element's ID is first assigned or subsequently modified
	 */
	template <typename ElementType, typename Options, u32 Dimension>
	class TSpatialTree
	{
		using SIMDBoundsType = typename TSpatialTreeTraits<Dimension>::SIMDBoundsType;
		using ScalarBoundsType = typename TSpatialTreeTraits<Dimension>::ScalarBoundsType;
		using CenterType = typename TSpatialTreeTraits<Dimension>::CenterType;
		static constexpr u32 kChildNodeCount = TSpatialTreeTraits<Dimension>::kChildNodeCount;

		/**
		 * A sequential group of elements within a node. If number of elements exceeds the limit of the block multiple
		 * blocks will be linked together in a linked list fashion.
		 */
		struct NodeElementSuballocationBlock
		{
			ElementType Elements[Options::MaximumElementsPerNode];
			NodeElementSuballocationBlock* NextBlock = nullptr;
		};

		/**
		 * A sequential group of element bounds within a node. If number of elements exceeds the limit of the block multiple
		 * blocks will be linked together in a linked list fashion.
		 */
		struct NodeElementBoundsSuballocationBlock
		{
			SIMDBoundsType Bounds[Options::MaximumElementsPerNode];
			NodeElementBoundsSuballocationBlock* NextBlock = nullptr;
		};

		/** Contains data about elements within a node. */
		struct NodeElementData
		{
			NodeElementSuballocationBlock* ElementsBlock = nullptr;
			NodeElementBoundsSuballocationBlock* ElementBoundsBlock = nullptr;
			u32 ElementCount = 0; /**< Number of elements on this node only. Only relevant for leaf nodes. */
		};

	public:
		/** Represents a single spatial tree node. */
		class Node
		{
		public:
			/** Constructs a new leaf node with the specified parent. */
			Node(Node* parent): mParent(parent), mElementCountWithChildren(0), mIsLeaf(true) {}

			/** Returns a child node with the specified index. May return null. */
			Node* GetChild(TSpatialTreeChildNodeId<Dimension> child) const { return mChildren[child.Index]; }

			/** Checks has the specified child node been created. */
			bool HasChild(TSpatialTreeChildNodeId<Dimension> child) const { return mChildren[child.Index] != nullptr; }

			/** Returns range containing all children of the current node. */
			TSpatialTreeChildNodeIdRange<Dimension> GetChildren() const;

		private:
			friend class ElementIterator;
			friend class TSpatialTree;

			/**
			 * Maps an element index within the node to a specific sub-allocation block in which the element is located. Returns pointers
			 * to the relevant sub-allocation blocks as output parameters, and index within those blocks as the return value.
			 */
			u32 MapElementIndexToBlock(u32 indexInNode, NodeElementSuballocationBlock** elements, NodeElementBoundsSuballocationBlock** bounds);

			NodeElementData mElementData;
			Node* mParent;
			Node* mChildren[kChildNodeCount] = { };
			u32 mElementCountWithChildren : 31; /**< Total number of elements including all children of the node. */
			u32 mIsLeaf : 1;
		};

		/**
		 * Contains bounds for a specific node. This is necessary since the nodes themselves do not store bounds
		 * information. Instead we construct it on-the-fly as we traverse the tree, using this class.
		 */
		class NodeBounds
		{
		public:
			NodeBounds() = default;

			/** Initializes a new bounds object using the provided node bounds. */
			NodeBounds(const SIMDBoundsType& bounds);

			/** Returns the bounds of the node this object represents. */
			const SIMDBoundsType& GetBounds() const { return mBounds; }

			/** Attempts to find a child node that can fully contain the provided bounds. */
			TSpatialTreeChildNodeId<Dimension> FindContainingChild(const SIMDBoundsType& bounds) const;

			/** Returns a range of child nodes that intersect the provided bounds. */
			TSpatialTreeChildNodeIdRange<Dimension> FindIntersectingChildren(const SIMDBoundsType& bounds) const;

			/** Calculates bounds for the provided child node. */
			NodeBounds GetChild(TSpatialTreeChildNodeId<Dimension> child) const;

		private:
			SIMDBoundsType mBounds;
			float mChildExtent;
			float mChildOffset;
		};

		/** Contains information about a specific tree node, to be used during node traversal. */
		class NodeTraversalContext
		{
		public:
			NodeTraversalContext() = default;
			NodeTraversalContext(const Node* node, const NodeBounds& bounds): Node(node), Bounds(bounds) {}

			const Node* Node = nullptr;
			NodeBounds Bounds;
		};

		/**
		 * Iterator that iterates over tree nodes. By default only the first inserted node will be iterated over and it
		 * is up the the user to add new ones using PushChild(). The iterator takes care of updating the node bounds
		 * accordingly.
		 */
		class NodeIterator
		{
		public:
			/** Initializes the iterator, starting with the root tree node. */
			NodeIterator(const TSpatialTree& tree);

			/** Initializes the iterator using a specific node and its bounds. */
			NodeIterator(const Node* node, const NodeBounds& bounds);

			/**
			 * Returns information about the current node. MoveNext() must be called at least once and it must return true
			 * prior to attempting to access this data.
			 */
			const NodeTraversalContext& GetCurrent() const { return mCurrentNodeContext; }

			/**
			 * Moves to the next entry in the iterator. Iterator starts at a position before the first element, therefore
			 * this method must be called at least once before attempting to access the current node. If the method returns
			 * false it means the iterator end has been reached and attempting to access data will result in an error.
			 */
			bool MoveNext();

			/** Inserts a child of the current node to be iterated over. */
			void PushChild(const TSpatialTreeChildNodeId<Dimension>& child);

		private:
			NodeTraversalContext mCurrentNodeContext;
			TInlineArray<NodeTraversalContext, Options::MaximumDepth * kChildNodeCount> mNodeStack;
		};

		/** Iterator that iterates over all elements in a single node. */
		class ElementIterator
		{
		public:
			ElementIterator() = default;

			/** Constructs an iterator that iterates over the specified node's elements. */
			ElementIterator(const Node* node);

			/**
			 * Moves to the next element in the node. Iterator starts at a position before the first element, therefore
			 * this method must be called at least once before attempting to access the current element data. If the method
			 * returns false it means iterator end has been reached and attempting to access data will result in an error.
			 */
			bool MoveNext();

			/**
			 * Returns the bounds of the current element. MoveNext() must be called at least once and it must return true
			 * prior to attempting to access this data.
			 */
			const SIMDBoundsType& GetCurrentBounds() const { return mCurrentElementBoundsBlock->Bounds[mCurrentIndex]; }

			/**
			 * Returns the contents of the current element. MoveNext() must be called at least once and it must return true
			 * prior to attempting to access this data.
			 */
			const ElementType& GetCurrentElement() const { return mCurrentElementBlock->Elements[mCurrentIndex]; }

		private:
			i32 mCurrentIndex = -1;
			NodeElementSuballocationBlock* mCurrentElementBlock = nullptr;
			NodeElementBoundsSuballocationBlock* mCurrentElementBoundsBlock = nullptr;
			u32 mEndOfGroupElementIndex = 0;
		};

		/** Iterator that iterates over all elements intersecting the specified AABox. */
		class AreaIntersectIterator
		{
		public:
			/** Constructs an iterator that iterates over all elements in the specified tree that intersect the specified bounds. */
			AreaIntersectIterator(const TSpatialTree& tree, const ScalarBoundsType& bounds);

			/**
			 * Returns the contents of the current element. MoveNext() must be called at least once and it must return true
			 * prior to attempting to access this data.
			 */
			const ElementType& GetElement() const { return mElementIterator.GetCurrentElement(); }

			/**
			 * Moves to the next intersecting element. Iterator starts at a position before the first element, therefore
			 * this method must be called at least once before attempting to access the current element data. If the method
			 * returns false it means iterator end has been reached and attempting to access data will result in an error.
			 */
			bool MoveNext();

		private:
			NodeIterator mNodeIterator;
			ElementIterator mElementIterator;
			SIMDBoundsType mBounds;
		};

		/** Iterator that iterates over all elements in the tree. */
		class TreeIterator
		{
		public:
			/** Constructs an iterator that iterates over all elements in the specified tree. */
			TreeIterator(const TSpatialTree& tree);

			/**
			 * Returns the contents of the current element. MoveNext() must be called at least once and it must return true
			 * prior to attempting to access this data.
			 */
			const ElementType& GetElement() const { return mElementIterator.GetCurrentElement(); }

			/**
			 * Moves to the next intersecting element. Iterator starts at a position before the first element, therefore
			 * this method must be called at least once before attempting to access the current element data. If the method
			 * returns false it means iterator end has been reached and attempting to access data will result in an error.
			 */
			bool MoveNext();

		private:
			NodeIterator mNodeIterator;
			ElementIterator mElementIterator;
		};

		/**
		 * Constructs a spatial tree with the specified bounds.
		 *
		 * @param	center		Origin of the root node.
		 * @param	extent		Extent (half-size) of the root node in all directions;
		 * @param	context		Optional user context that will be passed along to GetBounds() and SetElementId() methods on the provided Options class.
		 */
		TSpatialTree(const CenterType& center, float extent, void* context = nullptr);

		~TSpatialTree() { FreeNode(&mRoot); }

		/** Adds a new element to the tree. */
		void AddElement(const ElementType& element) { AddElementToNode(element, &mRoot, mRootBounds); }

		/** Removes an existing element from the tree. */
		void RemoveElement(const SpatialTreeElementId& elementId);

	private:
		/**
		 * Adds a new element to the specified node. If the provided node is not a leaf node, traverses the hierarchy until it finds the correct node.
		 * Potentially also subdivides the leaf node if subdivision conditions have been reached.
		 */
		void AddElementToNode(const ElementType& element, Node* node, const NodeBounds& nodeBounds);

		/**
		 * Adds a new element to the node's element list. This method does no additional checks so caller must ensure the node is a leaf node
		 * and does not need to be subdivided.
		 */
		void AddElementToLeafNode(Node* node, const ElementType& element, const SIMDBoundsType& elementBounds);

		/** Removes the specified element from the node's element list. */
		void RemoveElementFromLeafNode(Node* node, u32 elementIndexInNode);

		/** Frees all elements from a node. */
		void FreeElementData(NodeElementData& elementData);

		/** Cleans up memory used by the provided node. Should be called instead of the node destructor. */
		void FreeNode(Node* node);

		Node mRoot{ nullptr };
		NodeBounds mRootBounds;
		float mMinimumNodeExtent;
		void* mContext;

		PoolAlloc<sizeof(Node)> mNodeAllocator;
		PoolAlloc<sizeof(NodeElementSuballocationBlock)> mElementBlockAllocator;
		PoolAlloc<sizeof(NodeElementBoundsSuballocationBlock), 512, 16> mElementBoundsBlockAllocator;
	};

	template <typename ElementType, typename Options, u32 Dimension>
	TSpatialTreeChildNodeIdRange<Dimension> TSpatialTree<ElementType, Options, Dimension>::Node::GetChildren() const
	{
		TSpatialTreeChildNodeIdRange<Dimension> output;

		if constexpr(Dimension == 1)
		{
			output.PositiveBits = 0x1;
			output.NegativeBits = 0x1;
		}
		else if constexpr(Dimension == 2)
		{
			output.PositiveBits = 0x3;
			output.NegativeBits = 0x3;
		}
		else if constexpr(Dimension == 3)
		{
			output.PositiveBits = 0x7;
			output.NegativeBits = 0x7;
		}

		return output;
	}

	template <typename ElementType, typename Options, u32 Dimension>
	u32 TSpatialTree<ElementType, Options, Dimension>::Node::MapElementIndexToBlock(u32 indexInNode, NodeElementSuballocationBlock** elements, NodeElementBoundsSuballocationBlock** bounds)
	{
		const u32 groupCount = Math::DivideAndRoundUp(mElementData.ElementCount, (u32)Options::MaximumElementsPerNode);
		const u32 groupIndex = groupCount - indexInNode / Options::MaximumElementsPerNode - 1;

		*elements = mElementData.ElementsBlock;
		*bounds = mElementData.ElementBoundsBlock;
		for(u32 blockIndex = 0; blockIndex < groupIndex; blockIndex++)
		{
			*elements = (*elements)->NextBlock;
			*bounds = (*bounds)->NextBlock;
		}

		return indexInNode % Options::MaximumElementsPerNode;
	}

	template <typename ElementType, typename Options, u32 Dimension>
	TSpatialTree<ElementType, Options, Dimension>::NodeBounds::NodeBounds(const SIMDBoundsType& bounds)
		: mBounds(bounds)
	{
		static constexpr float kChildExtentScale = 0.5f * (1.0f + 1.0f / (float)Options::LoosePadding);

		mChildExtent = bounds.Extents.X * kChildExtentScale;
		mChildOffset = bounds.Extents.X - mChildExtent;
	}

	template <typename ElementType, typename Options, u32 Dimension>
	TSpatialTreeChildNodeId<Dimension> TSpatialTree<ElementType, Options, Dimension>::NodeBounds::FindContainingChild(const SIMDBoundsType& bounds) const
	{
		auto queryCenter = simd::load<simd::float32x4>(&bounds.Center);

		auto nodeCenter = simd::load<simd::float32x4>(&mBounds.Center);
		auto childOffset = simd::load_splat<simd::float32x4>(&mChildOffset);

		auto negativeCenter = simd::sub(nodeCenter, childOffset);
		auto negativeDiff = simd::sub(queryCenter, negativeCenter);

		auto positiveCenter = simd::add(nodeCenter, childOffset);
		auto positiveDiff = simd::sub(positiveCenter, queryCenter);

		auto diff = simd::min(negativeDiff, positiveDiff);

		auto queryExtents = simd::load<simd::float32x4>(&bounds.Extents);
		auto childExtent = simd::load_splat<simd::float32x4>(&mChildExtent);

		TSpatialTreeChildNodeId<Dimension> output;

		simd::mask_float32x4 mask = simd::cmp_gt(simd::add(queryExtents, diff), childExtent);
		if(simd::test_bits_any(simd::bit_cast<simd::uint32x4>(mask)) == false)
		{
#if B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_32 || B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64
			if constexpr(Dimension == 1)
				output.Index = _mm_movemask_ps(simd::cmp_gt(queryCenter, nodeCenter).e.native()) & 0x1;
			else if constexpr(Dimension == 2)
				output.Index = _mm_movemask_ps(simd::cmp_gt(queryCenter, nodeCenter).e.native()) & 0x3;
			else if constexpr(Dimension == 3)
				output.Index = _mm_movemask_ps(simd::cmp_gt(queryCenter, nodeCenter).e.native()) & 0x7;
#else // Generic path
			auto ones = simd::make_uint<simd::uint32x4>(1, 1, 1, 1);
			auto zeroes = simd::make_uint<simd::uint32x4>(0, 0, 0, 0);

			// Find node closest to the query center
			mask = simd::cmp_gt(queryCenter, nodeCenter);
			auto result = simd::blend(ones, zeroes, mask);

			Vector4I scalarResult;
			simd::store(&scalarResult, result);

			output.X = scalarResult.X;

			if constexpr(Dimension >= 2)
				output.Y = scalarResult.Y;

			if constexpr(Dimension >= 3)
				output.Z = scalarResult.Z;
#endif

			output.Empty = false;
		}

		return output;
	}

	template <typename ElementType, typename Options, u32 Dimension>
	TSpatialTreeChildNodeIdRange<Dimension> TSpatialTree<ElementType, Options, Dimension>::NodeBounds::FindIntersectingChildren(const SIMDBoundsType& bounds) const
	{
		auto queryCenter = simd::load<simd::float32x4>(&bounds.Center);
		auto queryExtents = simd::load<simd::float32x4>(&bounds.Extents);

		auto queryMax = simd::add(queryCenter, queryExtents);
		auto queryMin = simd::sub(queryCenter, queryExtents);

		auto nodeCenter = simd::load<simd::float32x4>(&mBounds.Center);
		auto childOffset = simd::load_splat<simd::float32x4>(&mChildOffset);

		auto negativeCenter = simd::sub(nodeCenter, childOffset);
		auto positiveCenter = simd::add(nodeCenter, childOffset);

		auto childExtent = simd::load_splat<simd::float32x4>(&mChildExtent);
		auto negativeMax = simd::add(negativeCenter, childExtent);
		auto positiveMin = simd::sub(positiveCenter, childExtent);

		TSpatialTreeChildNodeIdRange<Dimension> output;

#if B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_32 || B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64
		if constexpr(Dimension == 1)
		{
			output.PositiveBits = _mm_movemask_ps(simd::cmp_gt(queryMax, positiveMin).e.native()) & 0x1;
			output.NegativeBits = _mm_movemask_ps(simd::cmp_le(queryMin, negativeMax).e.native()) & 0x1;
		}
		else if constexpr(Dimension == 2)
		{
			output.PositiveBits = _mm_movemask_ps(simd::cmp_gt(queryMax, positiveMin).e.native()) & 0x3;
			output.NegativeBits = _mm_movemask_ps(simd::cmp_le(queryMin, negativeMax).e.native()) & 0x3;
			
		}
		else if constexpr(Dimension == 3)
		{
			output.PositiveBits = _mm_movemask_ps(simd::cmp_gt(queryMax, positiveMin).e.native()) & 0x7;
			output.NegativeBits = _mm_movemask_ps(simd::cmp_le(queryMin, negativeMax).e.native()) & 0x7;
		}
#else // Generic path
		auto ones = simd::make_uint<simd::uint32x4>(1, 1, 1, 1);
		auto zeroes = simd::make_uint<simd::uint32x4>(0, 0, 0, 0);

		simd::mask_float32x4 mask = simd::cmp_gt(queryMax, positiveMin);
		simd::uint32x4 result = simd::blend(ones, zeroes, mask);

		Vector4I scalarResult;
		simd::store(&scalarResult, result);

		output.PositiveX = scalarResult.X;

		if constexpr(Dimension >= 2)
			output.PositiveY = scalarResult.Y;

		if constexpr(Dimension >= 3)
			output.PositiveZ = scalarResult.Z;

		mask = simd::cmp_le(queryMin, negativeMax);
		result = simd::blend(ones, zeroes, mask);

		simd::store(&scalarResult, result);

		output.NegativeX = scalarResult.X;

		if constexpr(Dimension >= 2)
			output.NegativeY = scalarResult.Y;

		if constexpr(Dimension >= 3)
			output.NegativeZ = scalarResult.Z;
#endif

		return output;
	}

	template <typename ElementType, typename Options, u32 Dimension>
	typename TSpatialTree<ElementType, Options, Dimension>::NodeBounds TSpatialTree<ElementType, Options, Dimension>::NodeBounds::GetChild(TSpatialTreeChildNodeId<Dimension> child) const
	{
		static constexpr float kMap[2] = { -1.0f, 1.0f };

		if constexpr(Dimension == 2)
		{
			return NodeBounds(
				SIMDBoundsType(
					CenterType(
						mBounds.Center.X + mChildOffset * kMap[child.X],
						mBounds.Center.Y + mChildOffset * kMap[child.Y]),
					mChildExtent));
		}
		else if constexpr(Dimension == 3)
		{
			return NodeBounds(
				SIMDBoundsType(
					CenterType(
						mBounds.Center.X + mChildOffset * kMap[child.X],
						mBounds.Center.Y + mChildOffset * kMap[child.Y],
						mBounds.Center.Z + mChildOffset * kMap[child.Z]),
					mChildExtent));
		}
	}

	template <typename ElementType, typename Options, u32 Dimension>
	TSpatialTree<ElementType, Options, Dimension>::NodeIterator::NodeIterator(const TSpatialTree& tree)
		: mCurrentNodeContext(NodeTraversalContext(&tree.mRoot, tree.mRootBounds))
	{
		mNodeStack.Add(mCurrentNodeContext);
	}

	template <typename ElementType, typename Options, u32 Dimension>
	TSpatialTree<ElementType, Options, Dimension>::NodeIterator::NodeIterator(const Node* node, const NodeBounds& bounds)
		: mCurrentNodeContext(NodeTraversalContext(node, bounds))
	{
		mNodeStack.Add(mCurrentNodeContext);
	}

	template <typename ElementType, typename Options, u32 Dimension>
	bool TSpatialTree<ElementType, Options, Dimension>::NodeIterator::MoveNext()
	{
		if(mNodeStack.Empty())
		{
			mCurrentNodeContext = NodeTraversalContext();
			return false;
		}

		mCurrentNodeContext = mNodeStack.Back();
		mNodeStack.Erase(mNodeStack.End() - 1);

		return true;
	}

	template <typename ElementType, typename Options, u32 Dimension>
	void TSpatialTree<ElementType, Options, Dimension>::NodeIterator::PushChild(const TSpatialTreeChildNodeId<Dimension>& child)
	{
		Node* const childNode = mCurrentNodeContext.Node->GetChild(child);
		NodeBounds childBounds = mCurrentNodeContext.Bounds.GetChild(child);

		mNodeStack.EmplaceBack(childNode, childBounds);
	}

	template <typename ElementType, typename Options, u32 Dimension>
	TSpatialTree<ElementType, Options, Dimension>::ElementIterator::ElementIterator(const Node* node)
		: mCurrentIndex(-1)
		, mCurrentElementBlock(node->mElementData.ElementsBlock)
		, mCurrentElementBoundsBlock(node->mElementData.ElementBoundsBlock)
	{
		const u32 groupCount = Math::DivideAndRoundUp(node->mElementData.ElementCount, (u32)Options::MaximumElementsPerNode);
		mEndOfGroupElementIndex = node->mElementData.ElementCount - (groupCount - 1) * Options::MaximumElementsPerNode;
	}

	template <typename ElementType, typename Options, u32 Dimension>
	bool TSpatialTree<ElementType, Options, Dimension>::ElementIterator::MoveNext()
	{
		if(!mCurrentElementBlock)
			return false;

		mCurrentIndex++;

		if((u32)mCurrentIndex == mEndOfGroupElementIndex) // Next group
		{
			mCurrentElementBlock = mCurrentElementBlock->NextBlock;
			mCurrentElementBoundsBlock = mCurrentElementBoundsBlock->NextBlock;
			mEndOfGroupElementIndex = Options::MaximumElementsPerNode; // Following groups are always full
			mCurrentIndex = 0;

			if(!mCurrentElementBlock)
				return false;
		}

		return true;
	}

	template <typename ElementType, typename Options, u32 Dimension>
	TSpatialTree<ElementType, Options, Dimension>::AreaIntersectIterator::AreaIntersectIterator(const TSpatialTree& tree, const ScalarBoundsType& bounds)
		: mNodeIterator(tree), mBounds(SIMDBoundsType(bounds))
	{}

	template <typename ElementType, typename Options, u32 Dimension>
	bool TSpatialTree<ElementType, Options, Dimension>::AreaIntersectIterator::MoveNext()
	{
		while(true)
		{
			// First check elements of the current node (if any)
			while(mElementIterator.MoveNext())
			{
				const SIMDBoundsType& bounds = mElementIterator.GetCurrentBounds();
				if(bounds.Overlaps(mBounds))
					return true;
			}

			// No more elements in this node, move to the next one
			if(!mNodeIterator.MoveNext())
				return false; // No more nodes to check

			const NodeTraversalContext& nodeTraversalContext = mNodeIterator.GetCurrent();
			mElementIterator = ElementIterator(nodeTraversalContext.Node);

			// Add all intersecting child nodes to the iterator
			TSpatialTreeChildNodeIdRange<Dimension> childRange = nodeTraversalContext.Bounds.FindIntersectingChildren(mBounds);
			for(u32 childIndex = 0; childIndex < kChildNodeCount; childIndex++)
			{
				if(childRange.Contains(childIndex) && nodeTraversalContext.Node->HasChild(childIndex))
					mNodeIterator.PushChild(childIndex);
			}
		}

		return false;
	}

	template <typename ElementType, typename Options, u32 Dimension>
	TSpatialTree<ElementType, Options, Dimension>::TreeIterator::TreeIterator(const TSpatialTree& tree)
		: mNodeIterator(tree)
	{}

	template <typename ElementType, typename Options, u32 Dimension>
	bool TSpatialTree<ElementType, Options, Dimension>::TreeIterator::MoveNext()
	{
		while(true)
		{
			// First check elements of the current node (if any)
			while(mElementIterator.MoveNext())
				return true;

			// No more elements in this node, move to the next one
			if(!mNodeIterator.MoveNext())
				return false; // No more nodes to check

			const NodeTraversalContext& nodeTraversalContext = mNodeIterator.GetCurrent();
			mElementIterator = ElementIterator(nodeTraversalContext.Node);

			// Add all intersecting child nodes to the iterator
			TSpatialTreeChildNodeIdRange<Dimension> childRange = nodeTraversalContext.Node->GetChildren();
			for(u32 childIndex = 0; childIndex < kChildNodeCount; childIndex++)
			{
				if(childRange.Contains(childIndex) && nodeTraversalContext.Node->HasChild(childIndex))
					mNodeIterator.PushChild(childIndex);
			}
		}

		return false;
	}

	template <typename ElementType, typename Options, u32 Dimension>
	TSpatialTree<ElementType, Options, Dimension>::TSpatialTree(const CenterType& center, float extent, void* context)
		: mRootBounds(SIMDBoundsType(center, extent))
		, mMinimumNodeExtent(extent * std::pow(0.5f * (1.0f + 1.0f / (float)Options::LoosePadding), Options::MaximumDepth))
		, mContext(context)
	{ }

	template <typename ElementType, typename Options, u32 Dimension>
	void TSpatialTree<ElementType, Options, Dimension>::RemoveElement(const SpatialTreeElementId& elementId)
	{
		Node* node = (Node*)elementId.Node;

		RemoveElementFromLeafNode(node, elementId.IndexInNode);

		// Reduce element counts in this and any parent nodes, check if nodes need collapsing
		Node* currentNode = node;
		Node* nodeToCollapse = nullptr;
		while(currentNode)
		{
			--currentNode->mElementCountWithChildren;

			if(currentNode->mElementCountWithChildren < Options::MinimumElementsPerNode)
				nodeToCollapse = currentNode;

			currentNode = currentNode->mParent;
		}

		if(nodeToCollapse)
		{
			// Add all the child node elements to the current node
			auto fnAddChildElementsRecursive = [this, node](Node* nodeToIterate, auto&& fnAddChilElementsRecursive) -> void
			{
				for(u32 childIndex = 0; childIndex < kChildNodeCount; childIndex++)
				{
					if(nodeToIterate->HasChild(childIndex))
					{
						Node* const childNode = nodeToIterate->GetChild(childIndex);

						ElementIterator elementIterator(childNode);
						while(elementIterator.MoveNext())
							AddElementToLeafNode(node, elementIterator.GetCurrentElement(), elementIterator.GetCurrentBounds());

						fnAddChilElementsRecursive(childNode, fnAddChilElementsRecursive);
					}
				}
			};

			fnAddChildElementsRecursive(node, fnAddChildElementsRecursive);

			node->mIsLeaf = true;

			// Recursively delete all child nodes
			for(u32 childIndex = 0; childIndex < kChildNodeCount; childIndex++)
			{
				if(node->mChildren[childIndex])
				{
					FreeNode(node->mChildren[childIndex]);

					mNodeAllocator.Destruct(node->mChildren[childIndex]);
					node->mChildren[childIndex] = nullptr;
				}
			}
		}
	}

	template <typename ElementType, typename Options, u32 Dimension>
	void TSpatialTree<ElementType, Options, Dimension>::AddElementToNode(const ElementType& element, Node* node, const NodeBounds& nodeBounds)
	{
		const SIMDBoundsType elementBounds = Options::GetBounds(element, mContext);

		++node->mElementCountWithChildren;
		if(node->mIsLeaf)
		{
			const SIMDBoundsType& bounds = nodeBounds.GetBounds();

			// Check if the node has too many elements and should be broken up
			if((node->mElementData.ElementCount + 1) > Options::MaximumElementsPerNode && bounds.Extents.X > mMinimumNodeExtent)
			{
				NodeElementData elementData = node->mElementData;

				ElementIterator elementIterator(node);
				node->mElementData = NodeElementData();

				// Mark the node as non-leaf, allowing children to be created
				node->mIsLeaf = false;
				node->mElementCountWithChildren = 0;

				// Re-add all the elements to this node, but this time it will add them to child elements due to it not being a leaf flag
				while(elementIterator.MoveNext())
					AddElementToNode(elementIterator.GetCurrentElement(), node, nodeBounds);

				// Free the element and element bound blocks from this node
				FreeElementData(elementData);

				// Insert the current element
				AddElementToNode(element, node, nodeBounds);
			}
			else
			{
				// No need to subdivide, just add the element to this node
				AddElementToLeafNode(node, element, elementBounds);
			}
		}
		else
		{
			// Attempt to find a child the element fits into
			TSpatialTreeChildNodeId<Dimension> child = nodeBounds.FindContainingChild(elementBounds);

			if(child.Empty)
			{
				// Element doesn't fit into a child, add it to this node
				AddElementToLeafNode(node, element, elementBounds);
			}
			else
			{
				// Create the child node if needed, and add the element to it
				if(!node->mChildren[child.Index])
					node->mChildren[child.Index] = mNodeAllocator.template Construct<Node>(node);

				AddElementToNode(element, node->mChildren[child.Index], nodeBounds.GetChild(child));
			}
		}
	}

	template <typename ElementType, typename Options, u32 Dimension>
	void TSpatialTree<ElementType, Options, Dimension>::AddElementToLeafNode(Node* node, const ElementType& element, const SIMDBoundsType& elementBounds)
	{
		NodeElementData& elementData = node->mElementData;

		const u32 freeElementIndex = elementData.ElementCount % Options::MaximumElementsPerNode;
		if(freeElementIndex == 0) // New sub-allocation block needed
		{
			NodeElementSuballocationBlock* elementsBlock = (NodeElementSuballocationBlock*)mElementBlockAllocator.template Construct<NodeElementSuballocationBlock>();
			NodeElementBoundsSuballocationBlock* elementBoundsBlock = (NodeElementBoundsSuballocationBlock*)mElementBoundsBlockAllocator.template Construct<NodeElementBoundsSuballocationBlock>();

			elementsBlock->NextBlock = elementData.ElementsBlock;
			elementBoundsBlock->NextBlock = elementData.ElementBoundsBlock;

			elementData.ElementsBlock = elementsBlock;
			elementData.ElementBoundsBlock = elementBoundsBlock;
		}

		elementData.ElementsBlock->Elements[freeElementIndex] = element;
		elementData.ElementBoundsBlock->Bounds[freeElementIndex] = elementBounds;

		const u32 elementIndex = elementData.ElementCount;
		Options::SetElementId(element, SpatialTreeElementId(node, elementIndex), mContext);

		++elementData.ElementCount;
	}

	template <typename ElementType, typename Options, u32 Dimension>
	void TSpatialTree<ElementType, Options, Dimension>::RemoveElementFromLeafNode(Node* node, u32 elementIndexInNode)
	{
		B3D_ENSURE(node->mElementData.ElementCount > 0);
		NodeElementData& elementData = node->mElementData;

		NodeElementSuballocationBlock* elementsBlock;
		NodeElementBoundsSuballocationBlock* elementBoundsBlock;
		const u32 elementIndexInBlock = node->MapElementIndexToBlock(elementIndexInNode, &elementsBlock, &elementBoundsBlock);

		NodeElementSuballocationBlock* lastElementsBlock;
		NodeElementBoundsSuballocationBlock* lastElementBoundsBlock;
		const u32 lastElementIndexInLastBlock = node->MapElementIndexToBlock(elementData.ElementCount - 1, &lastElementsBlock, &lastElementBoundsBlock);

		if(elementIndexInNode != elementData.ElementCount -1)
		{
			std::swap(elementsBlock->Elements[elementIndexInBlock], lastElementsBlock->Elements[lastElementIndexInLastBlock]);
			std::swap(elementBoundsBlock->Bounds[elementIndexInBlock], lastElementBoundsBlock->Bounds[lastElementIndexInLastBlock]);

			Options::SetElementId(elementsBlock->Elements[elementIndexInBlock], SpatialTreeElementId(node, elementIndexInNode), mContext);
		}

		if(lastElementIndexInLastBlock == 0) // Last element in that block, remove the block completely
		{
			elementData.ElementsBlock = lastElementsBlock->NextBlock;
			elementData.ElementBoundsBlock = lastElementBoundsBlock->NextBlock;

			mElementBlockAllocator.Destruct(lastElementsBlock);
			mElementBoundsBlockAllocator.Destruct(lastElementBoundsBlock);
		}

		--elementData.ElementCount;
	}

	template <typename ElementType, typename Options, u32 Dimension>
	void TSpatialTree<ElementType, Options, Dimension>::FreeElementData(NodeElementData& elementData)
	{
		// Free the element and element bounds blocks from this node
		NodeElementSuballocationBlock* currentElementsBlock = elementData.ElementsBlock;
		while(currentElementsBlock)
		{
			NodeElementSuballocationBlock* const toDelete = currentElementsBlock;
			currentElementsBlock = currentElementsBlock->NextBlock;

			mElementBlockAllocator.Destruct(toDelete);
		}

		NodeElementBoundsSuballocationBlock* currentElementBoundsBlock = elementData.ElementBoundsBlock;
		while(currentElementBoundsBlock)
		{
			NodeElementBoundsSuballocationBlock* const toDelete = currentElementBoundsBlock;
			currentElementBoundsBlock = currentElementBoundsBlock->NextBlock;

			mElementBoundsBlockAllocator.Destruct(toDelete);
		}

		elementData.ElementsBlock = nullptr;
		elementData.ElementBoundsBlock = nullptr;
		elementData.ElementCount = 0;
	}

	template <typename ElementType, typename Options, u32 Dimension>
	void TSpatialTree<ElementType, Options, Dimension>::FreeNode(Node* node)
	{
		FreeElementData(node->mElementData);

		for(auto& entry : node->mChildren)
		{
			if(entry != nullptr)
			{
				FreeNode(entry);
				mNodeAllocator.Destruct(entry);
			}
		}
	}

	template<typename ElementType, typename Options>
	using TOctTree = TSpatialTree<ElementType, Options, 3>;

	template<typename ElementType, typename Options>
	using TQuadTree = TSpatialTree<ElementType, Options, 2>;

	/** @} */
} // namespace b3d
