//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
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

	/** Identifier that may be used for finding an element in the octree. */
	class OctreeElementId
	{
	public:
		OctreeElementId() = default;

		OctreeElementId(void* node, u32 elementIdx)
			: node(node), elementIdx(elementIdx)
		{}

	private:
		template <class, class>
		friend class Octree;

		void* node = nullptr;
		u32 elementIdx = 0u;
	};

	/**
	 * Spatial partitioning tree for 3D space.
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
	 *							- "static simd::AABox getBounds(const ElemType&, void*)"
	 *								- Returns the bounds for the provided element
	 *							- "static void setElementId(const Octree::ElementId&, void*)"
	 *								- Gets called when element's ID is first assigned or subsequentily modified
	 */
	template <class ElemType, class Options>
	class Octree
	{
		/**
		 * A sequential group of elements within a node. If number of elements exceeds the limit of the group multiple
		 * groups will be linked together in a linked list fashion.
		 */
		struct ElementGroup
		{
			ElemType V[Options::MaxElementsPerNode];
			ElementGroup* Next = nullptr;
		};

		/**
		 * A sequential group of element bounds within a node. If number of elements exceeds the limit of the group multiple
		 * groups will be linked together in a linked list fashion.
		 */
		struct ElementBoundGroup
		{
			simd::AABox V[Options::MaxElementsPerNode];
			ElementBoundGroup* Next = nullptr;
		};

		/** Container class for all elements (and their bounds) within a single node. */
		struct NodeElements
		{
			ElementGroup* Values = nullptr;
			ElementBoundGroup* Bounds = nullptr;
			u32 Count = 0;
		};

	public:
		/** Contains a reference to one of the eight child nodes in an octree node. */
		struct HChildNode
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

			HChildNode()
				: Empty(true)
			{}

			HChildNode(u32 x, u32 y, u32 z)
				: X(x), Y(y), Z(z),Empty(false)
			{}

			HChildNode(u32 index)
				: Index(index), Empty2(false)
			{}
		};

		/** Contains a range of child nodes in an octree node. */
		struct NodeChildRange
		{
			union
			{
				struct
				{
					u32 PosX : 1;
					u32 PosY : 1;
					u32 PosZ : 1;
					u32 NegX : 1;
					u32 NegY : 1;
					u32 NegZ : 1;
				};

				struct
				{
					u32 PosBits : 3;
					u32 NegBits : 3;
				};

				u32 AllBits : 6;
			};

			/** Constructs a range overlapping no nodes. */
			NodeChildRange()
				: AllBits(0)
			{}

			/** Constructs a range overlapping a single node. */
			NodeChildRange(HChildNode child)
				: PosBits(child.Index), NegBits(~child.Index)
			{}

			/** Checks if the range contains the provided child. */
			bool Contains(HChildNode child)
			{
				NodeChildRange childRange(child);
				return (AllBits & childRange.AllBits) == childRange.AllBits;
			}
		};

		/** Represents a single octree node. */
		class Node
		{
		public:
			/** Constructs a new leaf node with the specified parent. */
			Node(Node* parent)
				: mParent(parent), mTotalNumElements(0), mIsLeaf(true)
			{}

			/** Returns a child node with the specified index. May return null. */
			Node* GetChild(HChildNode child) const
			{
				return mChildren[child.Index];
			}

			/** Checks has the specified child node been created. */
			bool HasChild(HChildNode child) const
			{
				return mChildren[child.Index] != nullptr;
			}

		private:
			friend class ElementIterator;
			friend class Octree;

			/** Maps a global element index to a set of element groups and an index within those groups. */
			u32 MapToGroup(u32 elementIdx, ElementGroup** elements, ElementBoundGroup** bounds)
			{
				u32 numGroups = Math::DivideAndRoundUp(mElements.Count, (u32)Options::MaxElementsPerNode);
				u32 groupIdx = numGroups - elementIdx / Options::MaxElementsPerNode - 1;

				*elements = mElements.Values;
				*bounds = mElements.Bounds;
				for(u32 i = 0; i < groupIdx; i++)
				{
					*elements = (*elements)->Next;
					*bounds = (*bounds)->Next;
				}

				return elementIdx % Options::MaxElementsPerNode;
			}

			NodeElements mElements;

			Node* mParent;
			Node* mChildren[8] = { nullptr, nullptr, nullptr, nullptr,
								   nullptr, nullptr, nullptr, nullptr };

			u32 mTotalNumElements : 31;
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
			NodeBounds(const simd::AABox& bounds)
				: mBounds(bounds)
			{
				static constexpr float kChildExtentScale = 0.5f * (1.0f + 1.0f / Options::LoosePadding);

				mChildExtent = bounds.Extents.X * kChildExtentScale;
				mChildOffset = bounds.Extents.X - mChildExtent;
			}

			/** Returns the bounds of the node this object represents. */
			const simd::AABox& GetBounds() const { return mBounds; }

			/** Attempts to find a child node that can fully contain the provided bounds. */
			HChildNode FindContainingChild(const simd::AABox& bounds) const
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

				HChildNode output;

				simd::mask_float32x4 mask = simd::cmp_gt(simd::add(queryExtents, diff), childExtent);
				if(simd::test_bits_any(simd::bit_cast<simd::uint32x4>(mask)) == false)
				{
					auto ones = simd::make_uint<simd::uint32x4>(1, 1, 1, 1);
					auto zeroes = simd::make_uint<simd::uint32x4>(0, 0, 0, 0);

					// Find node closest to the query center
					mask = simd::cmp_gt(queryCenter, nodeCenter);
					auto result = simd::blend(ones, zeroes, mask);

					Vector4I scalarResult;
					simd::store(&scalarResult, result);

					output.X = scalarResult.X;
					output.Y = scalarResult.Y;
					output.Z = scalarResult.Z;

					output.Empty = false;
				}

				return output;
			}

			/** Returns a range of child nodes that intersect the provided bounds. */
			NodeChildRange FindIntersectingChildren(const simd::AABox& bounds) const
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

				NodeChildRange output;

				auto ones = simd::make_uint<simd::uint32x4>(1, 1, 1, 1);
				auto zeroes = simd::make_uint<simd::uint32x4>(0, 0, 0, 0);

				simd::mask_float32x4 mask = simd::cmp_gt(queryMax, positiveMin);
				simd::uint32x4 result = simd::blend(ones, zeroes, mask);

				Vector4I scalarResult;
				simd::store(&scalarResult, result);

				output.PosX = scalarResult.X;
				output.PosY = scalarResult.Y;
				output.PosZ = scalarResult.Z;

				mask = simd::cmp_le(queryMin, negativeMax);
				result = simd::blend(ones, zeroes, mask);

				simd::store(&scalarResult, result);

				output.NegX = scalarResult.X;
				output.NegY = scalarResult.Y;
				output.NegZ = scalarResult.Z;

				return output;
			}

			/** Calculates bounds for the provided child node. */
			NodeBounds GetChild(HChildNode child) const
			{
				static constexpr const float map[2] = { -1.0f, 1.0f };

				return NodeBounds(
					simd::AABox(
						Vector3(
							mBounds.Center.X + mChildOffset * map[child.X],
							mBounds.Center.Y + mChildOffset * map[child.Y],
							mBounds.Center.Z + mChildOffset * map[child.Z]),
						mChildExtent));
			}

		private:
			simd::AABox mBounds;
			float mChildExtent;
			float mChildOffset;
		};

		/** Contains a reference to a specific octree node, as well as information about its bounds. */
		class HNode
		{
		public:
			HNode() = default;

			HNode(const Node* node, const NodeBounds& bounds)
				: mNode(node), mBounds(bounds)
			{}

			/** Returns the referenced node. */
			const Node* GetNode() const { return mNode; }

			/** Returns the node bounds. */
			const NodeBounds& GetBounds() const { return mBounds; }

		private:
			const Node* mNode = nullptr;
			NodeBounds mBounds;
		};

		/**
		 * Iterator that iterates over octree nodes. By default only the first inserted node will be iterated over and it
		 * is up the the user to add new ones using pushChild(). The iterator takes care of updating the node bounds
		 * accordingly.
		 */
		class NodeIterator
		{
		public:
			/** Initializes the iterator, starting with the root octree node. */
			NodeIterator(const Octree& tree)
				: mCurrentNode(HNode(&tree.mRoot, tree.mRootBounds)), mStackAlloc(), mNodeStack(&mStackAlloc)
			{
				mNodeStack.push_back(mCurrentNode);
			}

			/** Initializes the iterator using a specific node and its bounds. */
			NodeIterator(const Node* node, const NodeBounds& bounds)
				: mCurrentNode(HNode(node, bounds)), mStackAlloc(), mNodeStack(&mStackAlloc)
			{
				mNodeStack.push_back(mCurrentNode);
			}

			/**
			 * Returns a reference to the current node. moveNext() must be called at least once and it must return true
			 * prior to attempting to access this data.
			 */
			const HNode& GetCurrent() const { return mCurrentNode; }

			/**
			 * Moves to the next entry in the iterator. Iterator starts at a position before the first element, therefore
			 * this method must be called at least once before attempting to access the current node. If the method returns
			 * false it means the iterator end has been reached and attempting to access data will result in an error.
			 */
			bool MoveNext()
			{
				if(mNodeStack.empty())
				{
					mCurrentNode = HNode();
					return false;
				}

				mCurrentNode = mNodeStack.back();
				mNodeStack.erase(mNodeStack.end() - 1);

				return true;
			}

			/** Inserts a child of the current node to be iterated over. */
			void PushChild(const HChildNode& child)
			{
				Node* childNode = mCurrentNode.GetNode()->GetChild(child);
				NodeBounds childBounds = mCurrentNode.GetBounds().GetChild(child);

				mNodeStack.emplace_back(childNode, childBounds);
			}

		private:
			HNode mCurrentNode;
			StaticAlloc<Options::MaxDepth * 8 * sizeof(HNode), FreeAlloc> mStackAlloc;
			StaticVector<HNode, Options::MaxDepth * 8> mNodeStack;
		};

		/** Iterator that iterates over all elements in a single node. */
		class ElementIterator
		{
		public:
			ElementIterator() = default;

			/** Constructs an iterator that iterates over the specified node's elements. */
			ElementIterator(const Node* node)
				: mCurrentIdx(-1)
				, mCurrentElemGroup(node->mElements.Values)
				, mCurrentBoundGroup(node->mElements.Bounds)
			{
				u32 numGroups = Math::DivideAndRoundUp(node->mElements.Count, (u32)Options::MaxElementsPerNode);
				mElemsInGroup = node->mElements.Count - (numGroups - 1) * Options::MaxElementsPerNode;
			}

			/**
			 * Moves to the next element in the node. Iterator starts at a position before the first element, therefore
			 * this method must be called at least once before attempting to access the current element data. If the method
			 * returns false it means iterator end has been reached and attempting to access data will result in an error.
			 */
			bool MoveNext()
			{
				if(!mCurrentElemGroup)
					return false;

				mCurrentIdx++;

				if((u32)mCurrentIdx == mElemsInGroup) // Next group
				{
					mCurrentElemGroup = mCurrentElemGroup->Next;
					mCurrentBoundGroup = mCurrentBoundGroup->Next;
					mElemsInGroup = Options::MaxElementsPerNode; // Following groups are always full
					mCurrentIdx = 0;

					if(!mCurrentElemGroup)
						return false;
				}

				return true;
			}

			/**
			 * Returns the bounds of the current element. moveNext() must be called at least once and it must return true
			 * prior to attempting to access this data.
			 */
			const simd::AABox& GetCurrentBounds() const { return mCurrentBoundGroup->V[mCurrentIdx]; }

			/**
			 * Returns the contents of the current element. moveNext() must be called at least once and it must return true
			 * prior to attempting to access this data.
			 */
			const ElemType& GetCurrentElem() const { return mCurrentElemGroup->V[mCurrentIdx]; }

		private:
			i32 mCurrentIdx = -1;
			ElementGroup* mCurrentElemGroup = nullptr;
			ElementBoundGroup* mCurrentBoundGroup = nullptr;
			u32 mElemsInGroup = 0;
		};

		/** Iterators that iterates over all elements intersecting the specified AABox. */
		class BoxIntersectIterator
		{
		public:
			/**
			 * Constructs an iterator that iterates over all elements in the specified tree that intersect the specified
			 * bounds.
			 */
			BoxIntersectIterator(const Octree& tree, const AABox& bounds)
				: mNodeIter(tree), mBounds(simd::AABox(bounds))
			{}

			/**
			 * Returns the contents of the current element. moveNext() must be called at least once and it must return true
			 * prior to attempting to access this data.
			 */
			const ElemType& GetElement() const
			{
				return mElemIter.GetCurrentElem();
			}

			/**
			 * Moves to the next intersecting element. Iterator starts at a position before the first element, therefore
			 * this method must be called at least once before attempting to access the current element data. If the method
			 * returns false it means iterator end has been reached and attempting to access data will result in an error.
			 */
			bool MoveNext()
			{
				while(true)
				{
					// First check elements of the current node (if any)
					while(mElemIter.MoveNext())
					{
						const simd::AABox& bounds = mElemIter.GetCurrentBounds();
						if(bounds.Intersects(mBounds))
							return true;
					}

					// No more elements in this node, move to the next one
					if(!mNodeIter.MoveNext())
						return false; // No more nodes to check

					const HNode& nodeRef = mNodeIter.GetCurrent();
					mElemIter = ElementIterator(nodeRef.GetNode());

					// Add all intersecting child nodes to the iterator
					NodeChildRange childRange = nodeRef.GetBounds().FindIntersectingChildren(mBounds);
					for(u32 i = 0; i < 8; i++)
					{
						if(childRange.Contains(i) && nodeRef.GetNode()->HasChild(i))
							mNodeIter.PushChild(i);
					}
				}

				return false;
			}

		private:
			NodeIterator mNodeIter;
			ElementIterator mElemIter;
			simd::AABox mBounds;
		};

		/**
		 * Constructs an octree with the specified bounds.
		 *
		 * @param[in]	center		Origin of the root node.
		 * @param[in]	extent		Extent (half-size) of the root node in all directions;
		 * @param[in]	context		Optional user context that will be passed along to getBounds() and setElementId()
		 *							methods on the provided Options class.
		 */
		Octree(const Vector3& center, float extent, void* context = nullptr)
			: mRootBounds(simd::AABox(center, extent))
			, mMinNodeExtent(extent * std::pow(0.5f * (1.0f + 1.0f / Options::LoosePadding), Options::MaxDepth))
			, mContext(context)
		{
		}

		~Octree()
		{
			DestroyNode(&mRoot);
		}

		/** Adds a new element to the octree. */
		void AddElement(const ElemType& elem)
		{
			AddElementToNode(elem, &mRoot, mRootBounds);
		}

		/** Removes an existing element from the octree. */
		void RemoveElement(const OctreeElementId& elemId)
		{
			Node* node = (Node*)elemId.node;

			PopElement(node, elemId.elementIdx);

			// Reduce element counts in this and any parent nodes, check if nodes need collapsing
			Node* iterNode = node;
			Node* nodeToCollapse = nullptr;
			while(iterNode)
			{
				--iterNode->mTotalNumElements;

				if(iterNode->mTotalNumElements < Options::MinElementsPerNode)
					nodeToCollapse = iterNode;

				iterNode = iterNode->mParent;
			}

			if(nodeToCollapse)
			{
				// Add all the child node elements to the current node
				B3DMarkAllocatorFrame();
				{
					FrameStack<Node*> todo;
					todo.push(node);

					while(!todo.empty())
					{
						Node* curNode = todo.top();
						todo.pop();

						for(u32 i = 0; i < 8; i++)
						{
							if(curNode->HasChild(i))
							{
								Node* childNode = curNode->GetChild(i);

								ElementIterator elemIter(childNode);
								while(elemIter.MoveNext())
									PushElement(node, elemIter.GetCurrentElem(), elemIter.GetCurrentBounds());

								todo.push(childNode);
							}
						}
					}
				}
				B3DClearAllocatorFrame();

				node->mIsLeaf = true;

				// Recursively delete all child nodes
				for(u32 i = 0; i < 8; i++)
				{
					if(node->mChildren[i])
					{
						DestroyNode(node->mChildren[i]);

						mNodeAlloc.Destruct(node->mChildren[i]);
						node->mChildren[i] = nullptr;
					}
				}
			}
		}

	private:
		/** Adds a new element to the specified node. Potentially also subdivides the node. */
		void AddElementToNode(const ElemType& elem, Node* node, const NodeBounds& nodeBounds)
		{
			simd::AABox elemBounds = Options::GetBounds(elem, mContext);

			++node->mTotalNumElements;
			if(node->mIsLeaf)
			{
				const simd::AABox& bounds = nodeBounds.GetBounds();

				// Check if the node has too many elements and should be broken up
				if((node->mElements.Count + 1) > Options::MaxElementsPerNode && bounds.Extents.X > mMinNodeExtent)
				{
					// Clear all elements from the current node
					NodeElements elements = node->mElements;

					ElementIterator elemIter(node);
					node->mElements = NodeElements();

					// Mark the node as non-leaf, allowing children to be created
					node->mIsLeaf = false;
					node->mTotalNumElements = 0;

					// Re-insert all previous elements into this node (likely creating child nodes)
					while(elemIter.MoveNext())
						AddElementToNode(elemIter.GetCurrentElem(), node, nodeBounds);

					// Free the element and bound groups from this node
					FreeElements(elements);

					// Insert the current element
					AddElementToNode(elem, node, nodeBounds);
				}
				else
				{
					// No need to sub-divide, just add the element to this node
					PushElement(node, elem, elemBounds);
				}
			}
			else
			{
				// Attempt to find a child the element fits into
				HChildNode child = nodeBounds.FindContainingChild(elemBounds);

				if(child.Empty)
				{
					// Element doesn't fit into a child, add it to this node
					PushElement(node, elem, elemBounds);
				}
				else
				{
					// Create the child node if needed, and add the element to it
					if(!node->mChildren[child.Index])
						node->mChildren[child.Index] = mNodeAlloc.template Construct<Node>(node);

					AddElementToNode(elem, node->mChildren[child.Index], nodeBounds.GetChild(child));
				}
			}
		}

		/** Cleans up memory used by the provided node. Should be called instead of the node destructor. */
		void DestroyNode(Node* node)
		{
			FreeElements(node->mElements);

			for(auto& entry : node->mChildren)
			{
				if(entry != nullptr)
				{
					DestroyNode(entry);
					mNodeAlloc.Destruct(entry);
				}
			}
		}

		/** Adds a new element to the node's element list. */
		void PushElement(Node* node, const ElemType& elem, const simd::AABox& bounds)
		{
			NodeElements& elements = node->mElements;

			u32 freeIdx = elements.Count % Options::MaxElementsPerNode;
			if(freeIdx == 0) // New group needed
			{
				ElementGroup* elementGroup = (ElementGroup*)mElemAlloc.template Construct<ElementGroup>();
				ElementBoundGroup* boundGroup = (ElementBoundGroup*)mElemBoundsAlloc.template Construct<ElementBoundGroup>();

				elementGroup->Next = elements.Values;
				boundGroup->Next = elements.Bounds;

				elements.Values = elementGroup;
				elements.Bounds = boundGroup;
			}

			elements.Values->V[freeIdx] = elem;
			elements.Bounds->V[freeIdx] = bounds;

			u32 elementIdx = elements.Count;
			Options::SetElementId(elem, OctreeElementId(node, elementIdx), mContext);

			++elements.Count;
		}

		/** Removes the specified element from the node's element list. */
		void PopElement(Node* node, u32 elementIdx)
		{
			NodeElements& elements = node->mElements;

			ElementGroup* elemGroup;
			ElementBoundGroup* boundGroup;
			elementIdx = node->MapToGroup(elementIdx, &elemGroup, &boundGroup);

			ElementGroup* lastElemGroup;
			ElementBoundGroup* lastBoundGroup;
			u32 lastElementIdx = node->MapToGroup(elements.Count - 1, &lastElemGroup, &lastBoundGroup);

			if(elements.Count > 1)
			{
				std::swap(elemGroup->V[elementIdx], lastElemGroup->V[lastElementIdx]);
				std::swap(boundGroup->V[elementIdx], lastBoundGroup->V[lastElementIdx]);

				Options::SetElementId(elemGroup->V[elementIdx], OctreeElementId(node, elementIdx), mContext);
			}

			if(lastElementIdx == 0) // Last element in that group, remove it completely
			{
				elements.Values = lastElemGroup->Next;
				elements.Bounds = lastBoundGroup->Next;

				mElemAlloc.Destruct(lastElemGroup);
				mElemBoundsAlloc.Destruct(lastBoundGroup);
			}

			--elements.Count;
		}

		/** Clears all elements from a node. */
		void FreeElements(NodeElements& elements)
		{
			// Free the element and bound groups from this node
			ElementGroup* curElemGroup = elements.Values;
			while(curElemGroup)
			{
				ElementGroup* toDelete = curElemGroup;
				curElemGroup = curElemGroup->Next;

				mElemAlloc.Destruct(toDelete);
			}

			ElementBoundGroup* curBoundGroup = elements.Bounds;
			while(curBoundGroup)
			{
				ElementBoundGroup* toDelete = curBoundGroup;
				curBoundGroup = curBoundGroup->Next;

				mElemBoundsAlloc.Destruct(toDelete);
			}

			elements.Values = nullptr;
			elements.Bounds = nullptr;
			elements.Count = 0;
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
} // namespace bs
