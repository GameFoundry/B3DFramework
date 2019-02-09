//************************************ bs::framework - Copyright 2014 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsMath.h"
#include "Math/BsVector4I.h"
#include "Math/BsSIMD.h"
#include "Allocators/BsPoolAlloc.h"

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	 /** Identifier that may be used for finding an element in the quadtree. */
	class QuadtreeElementId
	{
	public:
		QuadtreeElementId() = default;

		QuadtreeElementId(void* node, UINT32 elementIdx)
			:node(node), elementIdx(elementIdx)
		{ }

	private:
		template<class, class>
		friend class Quadtree;

		void* node = nullptr;
		UINT32 elementIdx = 0u;
	};

	/**
	 * Spatial partitioning tree for 2D space.
	 *
	 * @tparam	ElemType	Type of elements to be stored in the tree.
	 * @tparam	Options		Class that controls various options of the tree. It must provide the following enums:
	 *							- LoosePadding: Denominator used to determine how much padding to add to each child node.
	 *											The extra padding percent is determined as (1.0f / LoosePadding). Larger
	 *											padding ensures elements are less likely to get stuck on a higher node
	 *											due to them straddling the boundary between the nodes.
	 *							- MinElementsPerNode: Determines at which point should node's children be removed and moved
	 *												  back into the parent (node is collapsed). This can occurr on element
	 *												  removal, when the element count drops below the specified number.
	 *							- MaxElementsPerNode: Determines at which point should a node be split into child nodes.
	 *												  If an element counter moves past this number the elements will be
	 *												  added to child nodes, if possible. If a node is already at maximum
	 *												  depth, this is ignored.
	 *							- MaxDepth: Maximum depth of nodes in the tree. Nodes at this depth will not be subdivided
	 *										even if they element counts go past MaxElementsPerNode.
	 *						It must also provide the following methods:
	 *							- "static simd::Rect2 getBounds(const ElemType&, void*)"
	 *								- Returns the bounds for the provided element
	 *							- "static void setElementId(const Quadtree::ElementId&, void*)"
	 *								- Gets called when element's ID is first assigned or subsequentily modified
	 */
	template<class ElemType, class Options>
	class Quadtree
	{
		/**
		 * A sequential group of elements within a node. If number of elements exceeds the limit of the group multiple
		 * groups will be linked together in a linked list fashion.
		 */
		struct ElementGroup
		{
			ElemType v[Options::MaxElementsPerNode];
			ElementGroup* next = nullptr;
		};

		/**
		 * A sequential group of element bounds within a node. If number of elements exceeds the limit of the group multiple
		 * groups will be linked together in a linked list fashion.
		 */
		struct ElementBoundGroup
		{
			simd::Rect2 v[Options::MaxElementsPerNode];
			ElementBoundGroup* next = nullptr;
		};

		/** Container class for all elements (and their bounds) within a single node. */
		struct NodeElements
		{
			ElementGroup* values = nullptr;
			ElementBoundGroup* bounds = nullptr;
			UINT32 count = 0;
		};
	public:
		/** Contains a reference to one of the eight child nodes in an quadtree node. */
		struct HChildNode
		{
			union
			{
				struct
				{
					UINT32 x : 1;
					UINT32 y : 1;
					UINT32 empty : 1;
				};

				struct
				{
					UINT32 index : 2;
					UINT32 empty2 : 1;
				};
			};

			HChildNode()
				:empty(true)
			{ }

			HChildNode(UINT32 x, UINT32 y)
				:x(x), y(y), empty(false)
			{ }

			HChildNode(UINT32 index)
				:index(index), empty2(false)
			{ }
		};

		/** Contains a range of child nodes in an quadtree node. */
		struct NodeChildRange
		{
			union
			{
				struct
				{
					UINT32 posX : 1;
					UINT32 posY : 1;
					UINT32 negX : 1;
					UINT32 negY : 1;
				};

				struct
				{
					UINT32 posBits : 2;
					UINT32 negBits : 2;
				};

				UINT32 allBits : 4;
			};

			/** Constructs a range overlapping no nodes. */
			NodeChildRange()
				:allBits(0)
			{ }

			/** Constructs a range overlapping a single node. */
			NodeChildRange(HChildNode child)
				:posBits(child.index), negBits(~child.index)
			{ }

			/** Checks if the range contains the provided child. */
			bool contains(HChildNode child)
			{
				NodeChildRange childRange(child);
				return (allBits & childRange.allBits) == childRange.allBits;
			}
		};

		/** Represents a single quadtree node. */
		class Node
		{
		public:
			/** Constructs a new leaf node with the specified parent. */
			Node(Node* parent)
				:mParent(parent), mTotalNumElements(0), mIsLeaf(true)
			{ }

			/** Returns a child node with the specified index. May return null. */
			Node* getChild(HChildNode child) const
			{
				return mChildren[child.index];
			}

			/** Checks has the specified child node been created. */
			bool hasChild(HChildNode child) const
			{
				return mChildren[child.index] != nullptr;
			}

		private:
			friend class ElementIterator;
			friend class Quadtree;

			/** Maps a global element index to a set of element groups and an index within those groups. */
			UINT32 mapToGroup(UINT32 elementIdx, ElementGroup** elements, ElementBoundGroup** bounds)
			{
				UINT32 numGroups = Math::divideAndRoundUp(mElements.count, (UINT32)Options::MaxElementsPerNode);
				UINT32 groupIdx = numGroups - elementIdx / Options::MaxElementsPerNode - 1;

				*elements = mElements.values;
				*bounds = mElements.bounds;
				for (UINT32 i = 0; i < groupIdx; i++)
				{
					*elements = (*elements)->next;
					*bounds = (*bounds)->next;
				}

				return elementIdx % Options::MaxElementsPerNode;
			}

			NodeElements mElements;

			Node* mParent;
			Node* mChildren[4] = { nullptr, nullptr, nullptr, nullptr };

			UINT32 mTotalNumElements : 31;
			UINT32 mIsLeaf : 1;
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
			NodeBounds(const simd::Rect2& bounds)
				:mBounds(bounds)
			{
				static constexpr float childExtentScale = 0.5f * (1.0f + 1.0f / Options::LoosePadding);

				mChildExtent = bounds.extents.x * childExtentScale;
				mChildOffset = bounds.extents.x - mChildExtent;
			}

			/** Returns the bounds of the node this object represents. */
			const simd::Rect2& getBounds() const { return mBounds; }

			/** Attempts to find a child node that can fully contain the provided bounds. */
			HChildNode findContainingChild(const simd::Rect2& bounds) const
			{
				auto queryCenter = simd::load<simd::float32x4>(&bounds.center);

				auto nodeCenter = simd::load<simd::float32x4>(&mBounds.center);
				auto childOffset = simd::load_splat<simd::float32x4>(&mChildOffset);

				auto negativeCenter = simd::sub(nodeCenter, childOffset);
				auto negativeDiff = simd::sub(queryCenter, negativeCenter);

				auto positiveCenter = simd::add(nodeCenter, childOffset);
				auto positiveDiff = simd::sub(positiveCenter, queryCenter);

				auto diff = simd::min(negativeDiff, positiveDiff);

				auto queryExtents = simd::load<simd::float32x4>(&bounds.extents);
				auto childExtent = simd::load_splat<simd::float32x4>(&mChildExtent);

				HChildNode output;

				simd::mask_float32x4 mask = simd::cmp_gt(simd::add(queryExtents, diff), childExtent);
				if (simd::test_bits_any(simd::bit_cast<simd::uint32x4>(mask)) == false)
				{
					auto ones = simd::make_uint<simd::uint32x4>(1, 1, 1, 1);
					auto zeroes = simd::make_uint<simd::uint32x4>(0, 0, 0, 0);

					// Find node closest to the query center
					mask = simd::cmp_gt(queryCenter, nodeCenter);
					auto result = simd::blend(ones, zeroes, mask);

					Vector4I scalarResult;
					simd::store(&scalarResult, result);

					output.x = scalarResult.x;
					output.y = scalarResult.y;

					output.empty = false;
				}

				return output;
			}

			/** Returns a range of child nodes that intersect the provided bounds. */
			NodeChildRange findIntersectingChildren(const simd::Rect2& bounds) const
			{
				auto queryCenter = simd::load<simd::float32x4>(&bounds.center);
				auto queryExtents = simd::load<simd::float32x4>(&bounds.extents);

				auto queryMax = simd::add(queryCenter, queryExtents);
				auto queryMin = simd::sub(queryCenter, queryExtents);

				auto nodeCenter = simd::load<simd::float32x4>(&mBounds.center);
				auto childOffset = simd::load_splat<simd::float32x4>(&mChildOffset);

				auto negativeCenter = simd::sub(nodeCenter, childOffset);
				auto positiveCenter = simd::add(nodeCenter, childOffset);

				auto childExtent = simd::load_splat<simd::float32x4>(&mChildExtent);
				auto negativeMax = simd::add(negativeCenter, childExtent);
				auto positiveMin = simd::sub(positiveCenter, childExtent);

				NodeChildRange output;

				auto ones = simd::make_uint<simd::uint32x4>(1, 1, 1, 1);
				auto zeroes = simd::make_uint<simd::uint32x4>(0, 0, 0, 0);

				simd::mask_float32x4 mask = simd::cmp_gt(queryMax, positiveMin);
				simd::uint32x4 result = simd::blend(ones, zeroes, mask);

				Vector4I scalarResult;
				simd::store(&scalarResult, result);

				output.posX = scalarResult.x;
				output.posY = scalarResult.y;

				mask = simd::cmp_le(queryMin, negativeMax);
				result = simd::blend(ones, zeroes, mask);

				simd::store(&scalarResult, result);

				output.negX = scalarResult.x;
				output.negY = scalarResult.y;

				return output;
			}

			/** Calculates bounds for the provided child node. */
			NodeBounds getChild(HChildNode child) const
			{
				static constexpr const float map[2] = { -1.0f, 1.0f };

				return NodeBounds(
					simd::Rect2(
						Vector2(
							mBounds.center.x + mChildOffset * map[child.x],
							mBounds.center.y + mChildOffset * map[child.y]
						),
						mChildExtent
					)
				);
			}

		private:
			simd::Rect2 mBounds;
			float mChildExtent;
			float mChildOffset;
		};

		/** Contains a reference to a specific quadtree node, as well as information about its bounds. */
		class HNode
		{
		public:
			HNode() = default;

			HNode(const Node* node, const NodeBounds& bounds)
				:mNode(node), mBounds(bounds)
			{ }

			/** Returns the referenced node. */
			const Node* getNode() const { return mNode; }

			/** Returns the node bounds. */
			const NodeBounds& getBounds() const { return mBounds; }

		private:
			const Node* mNode = nullptr;
			NodeBounds mBounds;
		};

		/**
		 * Iterator that iterates over quadtree nodes. By default only the first inserted node will be iterated over and it
		 * is up the the user to add new ones using pushChild(). The iterator takes care of updating the node bounds
		 * accordingly.
		 */
		class NodeIterator
		{
		public:
			/** Initializes the iterator, starting with the root quadtree node. */
			NodeIterator(const Quadtree& tree)
				:mCurrentNode(HNode(&tree.mRoot, tree.mRootBounds)), mStackAlloc(), mNodeStack(&mStackAlloc)
			{
				mNodeStack.push_back(mCurrentNode);
			}

			/** Initializes the iterator using a specific node and its bounds. */
			NodeIterator(const Node* node, const NodeBounds& bounds)
				:mCurrentNode(HNode(node, bounds)), mStackAlloc(), mNodeStack(&mStackAlloc)
			{
				mNodeStack.push_back(mCurrentNode);
			}

			/**
			 * Returns a reference to the current node. moveNext() must be called at least once and it must return true
			 * prior to attempting to access this data.
			 */
			const HNode& getCurrent() const { return mCurrentNode; }

			/**
			 * Moves to the next entry in the iterator. Iterator starts at a position before the first element, therefore
			 * this method must be called at least once before attempting to access the current node. If the method returns
			 * false it means the iterator end has been reached and attempting to access data will result in an error.
			 */
			bool moveNext()
			{
				if (mNodeStack.empty())
				{
					mCurrentNode = HNode();
					return false;
				}

				mCurrentNode = mNodeStack.back();
				mNodeStack.erase(mNodeStack.end() - 1);

				return true;
			}

			/** Inserts a child of the current node to be iterated over. */
			void pushChild(const HChildNode& child)
			{
				Node* childNode = mCurrentNode.getNode()->getChild(child);
				NodeBounds childBounds = mCurrentNode.getBounds().getChild(child);

				mNodeStack.emplace_back(childNode, childBounds);
			}

		private:
			HNode mCurrentNode;
			StaticAlloc<Options::MaxDepth * 4 * sizeof(HNode), FreeAlloc> mStackAlloc;
			StaticVector<HNode, Options::MaxDepth * 4> mNodeStack;
		};

		/** Iterator that iterates over all elements in a single node. */
		class ElementIterator
		{
		public:
			ElementIterator() = default;

			/** Constructs an iterator that iterates over the specified node's elements. */
			ElementIterator(const Node* node)
				: mCurrentIdx(-1)
				, mCurrentElemGroup(node->mElements.values)
				, mCurrentBoundGroup(node->mElements.bounds)
			{
				UINT32 numGroups = Math::divideAndRoundUp(node->mElements.count, (UINT32)Options::MaxElementsPerNode);
				mElemsInGroup = node->mElements.count - (numGroups - 1) * Options::MaxElementsPerNode;
			}

			/**
			 * Moves to the next element in the node. Iterator starts at a position before the first element, therefore
			 * this method must be called at least once before attempting to access the current element data. If the method
			 * returns false it means iterator end has been reached and attempting to access data will result in an error.
			 */
			bool moveNext()
			{
				if (!mCurrentElemGroup)
					return false;

				mCurrentIdx++;

				if ((UINT32)mCurrentIdx == mElemsInGroup) // Next group
				{
					mCurrentElemGroup = mCurrentElemGroup->next;
					mCurrentBoundGroup = mCurrentBoundGroup->next;
					mElemsInGroup = Options::MaxElementsPerNode; // Following groups are always full
					mCurrentIdx = 0;

					if (!mCurrentElemGroup)
						return false;
				}

				return true;
			}

			/**
			 * Returns the bounds of the current element. moveNext() must be called at least once and it must return true
			 * prior to attempting to access this data.
			 */
			const simd::Rect2& getCurrentBounds() const { return mCurrentBoundGroup->v[mCurrentIdx]; }

			/**
			 * Returns the contents of the current element. moveNext() must be called at least once and it must return true
			 * prior to attempting to access this data.
			 */
			const ElemType& getCurrentElem() const { return mCurrentElemGroup->v[mCurrentIdx]; }

		private:
			INT32 mCurrentIdx = -1;
			ElementGroup* mCurrentElemGroup = nullptr;
			ElementBoundGroup* mCurrentBoundGroup = nullptr;
			UINT32 mElemsInGroup = 0;
		};

		/** Iterators that iterates over all elements intersecting the specified Rect2. */
		class BoxIntersectIterator
		{
		public:
			/**
			 * Constructs an iterator that iterates over all elements in the specified tree that intersect the specified
			 * bounds.
			 */
			BoxIntersectIterator(const Quadtree& tree, const Rect2& bounds)
				:mNodeIter(tree), mBounds(simd::Rect2(bounds))
			{ }

			/**
			 * Returns the contents of the current element. moveNext() must be called at least once and it must return true
			 * prior to attempting to access this data.
			 */
			const ElemType& getElement() const
			{
				return mElemIter.getCurrentElem();
			}

			/**
			 * Moves to the next intersecting element. Iterator starts at a position before the first element, therefore
			 * this method must be called at least once before attempting to access the current element data. If the method
			 * returns false it means iterator end has been reached and attempting to access data will result in an error.
			 */
			bool moveNext()
			{
				while (true)
				{
					// First check elements of the current node (if any)
					while (mElemIter.moveNext())
					{
						const simd::Rect2& bounds = mElemIter.getCurrentBounds();
						if (bounds.overlaps(mBounds))
							return true;
					}

					// No more elements in this node, move to the next one
					if (!mNodeIter.moveNext())
						return false; // No more nodes to check

					const HNode& nodeRef = mNodeIter.getCurrent();
					mElemIter = ElementIterator(nodeRef.getNode());

					// Add all intersecting child nodes to the iterator
					NodeChildRange childRange = nodeRef.getBounds().findIntersectingChildren(mBounds);
					for (UINT32 i = 0; i < 4; i++)
					{
						if (childRange.contains(i) && nodeRef.getNode()->hasChild(i))
							mNodeIter.pushChild(i);
					}
				}

				return false;
			}

		private:
			NodeIterator mNodeIter;
			ElementIterator mElemIter;
			simd::Rect2 mBounds;
		};

		/**
		 * Constructs an quadtree with the specified bounds.
		 *
		 * @param[in]	center		Origin of the root node.
		 * @param[in]	extent		Extent (half-size) of the root node in all directions;
		 * @param[in]	context		Optional user context that will be passed along to getBounds() and setElementId()
		 *							methods on the provided Options class.
		 */
		Quadtree(const Vector2& center, float extent, void* context = nullptr)
			: mRootBounds(simd::Rect2(center, extent))
			, mMinNodeExtent(extent * std::pow(0.5f * (1.0f + 1.0f / Options::LoosePadding), Options::MaxDepth))
			, mContext(context)
		{
		}

		~Quadtree()
		{
			destroyNode(&mRoot);
		}

		/** Adds a new element to the quadtree. */
		void addElement(const ElemType& elem)
		{
			addElementToNode(elem, &mRoot, mRootBounds);
		}

		/** Removes an existing element from the quadtree. */
		void removeElement(const QuadtreeElementId& elemId)
		{
			Node* node = (Node*)elemId.node;

			popElement(node, elemId.elementIdx);

			// Reduce element counts in this and any parent nodes, check if nodes need collapsing
			Node* iterNode = node;
			Node* nodeToCollapse = nullptr;
			while (iterNode)
			{
				--iterNode->mTotalNumElements;

				if (iterNode->mTotalNumElements < Options::MinElementsPerNode)
					nodeToCollapse = iterNode;

				iterNode = iterNode->mParent;
			}

			if (nodeToCollapse)
			{
				// Add all the child node elements to the current node
				bs_frame_mark();
				{
					FrameStack<Node*> todo;
					todo.push(node);

					while (!todo.empty())
					{
						Node* curNode = todo.top();
						todo.pop();

						for (UINT32 i = 0; i < 4; i++)
						{
							if (curNode->hasChild(i))
							{
								Node* childNode = curNode->getChild(i);

								ElementIterator elemIter(childNode);
								while (elemIter.moveNext())
									pushElement(node, elemIter.getCurrentElem(), elemIter.getCurrentBounds());

								todo.push(childNode);
							}
						}
					}
				}
				bs_frame_clear();

				node->mIsLeaf = true;

				// Recursively delete all child nodes
				for (UINT32 i = 0; i < 4; i++)
				{
					if (node->mChildren[i])
					{
						destroyNode(node->mChildren[i]);

						mNodeAlloc.destruct(node->mChildren[i]);
						node->mChildren[i] = nullptr;
					}
				}
			}
		}

	private:
		/** Adds a new element to the specified node. Potentially also subdivides the node. */
		void addElementToNode(const ElemType& elem, Node* node, const NodeBounds& nodeBounds)
		{
			simd::Rect2 elemBounds = Options::getBounds(elem, mContext);

			++node->mTotalNumElements;
			if (node->mIsLeaf)
			{
				const simd::Rect2& bounds = nodeBounds.getBounds();

				// Check if the node has too many elements and should be broken up
				if ((node->mElements.count + 1) > Options::MaxElementsPerNode && bounds.extents.x > mMinNodeExtent)
				{
					// Clear all elements from the current node
					NodeElements elements = node->mElements;

					ElementIterator elemIter(node);
					node->mElements = NodeElements();

					// Mark the node as non-leaf, allowing children to be created
					node->mIsLeaf = false;
					node->mTotalNumElements = 0;

					// Re-insert all previous elements into this node (likely creating child nodes)
					while (elemIter.moveNext())
						addElementToNode(elemIter.getCurrentElem(), node, nodeBounds);

					// Free the element and bound groups from this node
					freeElements(elements);

					// Insert the current element
					addElementToNode(elem, node, nodeBounds);
				}
				else
				{
					// No need to sub-divide, just add the element to this node
					pushElement(node, elem, elemBounds);
				}
			}
			else
			{
				// Attempt to find a child the element fits into
				HChildNode child = nodeBounds.findContainingChild(elemBounds);

				if (child.empty)
				{
					// Element doesn't fit into a child, add it to this node
					pushElement(node, elem, elemBounds);
				}
				else
				{
					// Create the child node if needed, and add the element to it
					if (!node->mChildren[child.index])
						node->mChildren[child.index] = mNodeAlloc.template construct<Node>(node);

					addElementToNode(elem, node->mChildren[child.index], nodeBounds.getChild(child));
				}
			}
		}

		/** Cleans up memory used by the provided node. Should be called instead of the node destructor. */
		void destroyNode(Node* node)
		{
			freeElements(node->mElements);

			for (auto& entry : node->mChildren)
			{
				if (entry != nullptr)
				{
					destroyNode(entry);
					mNodeAlloc.destruct(entry);
				}
			}
		}

		/** Adds a new element to the node's element list. */
		void pushElement(Node* node, const ElemType& elem, const simd::Rect2& bounds)
		{
			NodeElements& elements = node->mElements;

			UINT32 freeIdx = elements.count % Options::MaxElementsPerNode;
			if (freeIdx == 0) // New group needed
			{
				ElementGroup* elementGroup = (ElementGroup*)mElemAlloc.template construct<ElementGroup>();
				ElementBoundGroup* boundGroup = (ElementBoundGroup*)mElemBoundsAlloc.template construct<ElementBoundGroup>();

				elementGroup->next = elements.values;
				boundGroup->next = elements.bounds;

				elements.values = elementGroup;
				elements.bounds = boundGroup;
			}

			elements.values->v[freeIdx] = elem;
			elements.bounds->v[freeIdx] = bounds;

			UINT32 elementIdx = elements.count;
			Options::setElementId(elem, QuadtreeElementId(node, elementIdx), mContext);

			++elements.count;
		}

		/** Removes the specified element from the node's element list. */
		void popElement(Node* node, UINT32 elementIdx)
		{
			NodeElements& elements = node->mElements;

			ElementGroup* elemGroup;
			ElementBoundGroup* boundGroup;
			elementIdx = node->mapToGroup(elementIdx, &elemGroup, &boundGroup);

			ElementGroup* lastElemGroup;
			ElementBoundGroup* lastBoundGroup;
			UINT32 lastElementIdx = node->mapToGroup(elements.count - 1, &lastElemGroup, &lastBoundGroup);

			if (elements.count > 1)
			{
				std::swap(elemGroup->v[elementIdx], lastElemGroup->v[lastElementIdx]);
				std::swap(boundGroup->v[elementIdx], lastBoundGroup->v[lastElementIdx]);

				Options::setElementId(elemGroup->v[elementIdx], QuadtreeElementId(node, elementIdx), mContext);
			}

			if (lastElementIdx == 0) // Last element in that group, remove it completely
			{
				elements.values = lastElemGroup->next;
				elements.bounds = lastBoundGroup->next;

				mElemAlloc.destruct(lastElemGroup);
				mElemBoundsAlloc.destruct(lastBoundGroup);
			}

			--elements.count;
		}

		/** Clears all elements from a node. */
		void freeElements(NodeElements& elements)
		{
			// Free the element and bound groups from this node
			ElementGroup* curElemGroup = elements.values;
			while (curElemGroup)
			{
				ElementGroup* toDelete = curElemGroup;
				curElemGroup = curElemGroup->next;

				mElemAlloc.destruct(toDelete);
			}

			ElementBoundGroup* curBoundGroup = elements.bounds;
			while (curBoundGroup)
			{
				ElementBoundGroup* toDelete = curBoundGroup;
				curBoundGroup = curBoundGroup->next;

				mElemBoundsAlloc.destruct(toDelete);
			}

			elements.values = nullptr;
			elements.bounds = nullptr;
			elements.count = 0;
		}

		Node mRoot{ nullptr };
		NodeBounds mRootBounds;
		float mMinNodeExtent;
		void* mContext;

		PoolAlloc<sizeof(Node)> mNodeAlloc;
		PoolAlloc<sizeof(ElementGroup)> mElemAlloc;
		PoolAlloc<sizeof(ElementBoundGroup), 512, 16> mElemBoundsAlloc;
	};

	/** @} */
}
