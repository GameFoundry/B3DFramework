//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Math/B3DArea2.h"
#include "B3DUtilityPrerequisites.h"
#include "Math/B3DVector2.h"

namespace b3d
{
	/** @addtogroup Image
	 *  @{
	 */

	/** Organizes a set of textures into a single larger texture (an atlas) by minimizing empty space. Does not allow modifications after initial construction. */
	class B3D_EXPORT StaticTextureAtlasLayout
	{
		/** Represent a single node in the texture atlas binary tree. */
		class TexAtlasNode
		{
		public:
			constexpr TexAtlasNode() = default;

			constexpr TexAtlasNode(u32 x, u32 y, u32 width, u32 height)
				: X(x), Y(y), Width(width), Height(height)
			{}

			u32 X = 0;
			u32 Y = 0;
			u32 Width = 0;
			u32 Height = 0;
			u32 Children[2]{ std::numeric_limits<u32>::max(), std::numeric_limits<u32>::max() };
			bool NodeFull = false;
		};

	public:
		StaticTextureAtlasLayout() = default;

		/**
		 * Constructs a new texture atlas layout with the provided parameters.
		 *
		 * @param[in]	width 			Initial width of the atlas texture.
		 * @param[in]	height			Initial height of the atlas texture.
		 * @param[in]	maxWidth		Maximum width the atlas texture is allowed to grow to, when elements don't fit.
		 * @param[in]	maxHeight		Maximum height the atlas texture is allowed to grow to, when elements don't fit.
		 * @param[in]	pow2			When true the resulting atlas size will always be a power of two.
		 */
		StaticTextureAtlasLayout(u32 width, u32 height, u32 maxWidth, u32 maxHeight, bool pow2 = false)
			: mInitialWidth(width), mInitialHeight(height), mWidth(width), mHeight(height), mPow2(pow2)
		{
			mNodes.push_back(TexAtlasNode(0, 0, maxWidth, maxHeight));
		}

		/**
		 * Attempts to add a new element in the layout. Elements should be added to the atlas from largest to smallest,
		 * otherwise a non-optimal layout is likely to be generated.
		 *
		 * @param	width	Width of the new element, in pixels.
		 * @param	height	Height of the new element, in pixels.
		 * @param	outX	Horizontal position of the new element within the atlas. Only valid if method returns true.
		 * @param	outY	Vertical position of the new element within the atlas. Only valid if method returns true.
		 * @return			True if the element was added to the atlas, false if the element doesn't fit.
		 */
		bool AddElement(u32 width, u32 height, u32& outX, u32& outY);

		/** Removes all entries from the layout. */
		void Clear();

		/** Checks have any elements been added to the layout. */
		bool IsEmpty() const { return mNodes.size() == 1; }

		/** Returns the width of the atlas texture, in pixels. */
		u32 GetWidth() const { return mWidth; }

		/** Returns the height of the atlas texture, in pixels. */
		u32 GetHeight() const { return mHeight; }

	private:
		/*
		 * Attempts to add a new element to the specified layout node.
		 *
		 * @param	nodeIndex		Index of the node to which to add the element.
		 * @param	width			Width of the new element, in pixels.
		 * @param	height			Height of the new element, in pixels.
		 * @param	outX			Horizontal position of the new element within the atlas. Only valid if method
		 *							returns true.
		 * @param	outY			Vertical position of the new element within the atlas. Only valid if method returns
		 *							true.
		 * @param	allowGrowth		When true, the width/height of the atlas will be allowed to grow to fit the element.
		 * @return					True if the element was added to the atlas, false if the element doesn't fit.
		 */
		bool AddToNode(u32 nodeIndex, u32 width, u32 height, u32& outX, u32& outY, bool allowGrowth);

		u32 mInitialWidth = 0;
		u32 mInitialHeight = 0;
		u32 mWidth = 0;
		u32 mHeight = 0;
		bool mPow2 = false;

		Vector<TexAtlasNode> mNodes;
	};

	/** Settings that control how a TreeTextureAtlasLayout behaves. */
	struct TreeTextureAtlasLayoutSettings
	{
		Size2UI Size = Size2UI(2048, 2048); /**< Size of the atlas in pixels. */
		Size2UI Alignment = Size2UI(1, 1); /**< If >1 all allocations will be in multiple of this value. Higher values help reduce fragmentation. */
		u32 SmallSizeLimit = 32; /**< Free nodes with rectangles under this size are treated as small and will be placed in a separate bucket for faster lookup. */
		u32 LargeSizeLimit = 256; /**< Free nodes with rectangles over this size are treated as large and will be placed in a separate bucket for faster lookup. */
		u32 MaximumPageCount = 0; /**< Maximum number of pages to allocated. If 0 it will be unlimited. */
		u32 Padding = 0; /**< Padding in pixels to apply to left/right/top/bottom of each image in the atlas. */
	};

	/**
	 * Organizes a set of textures into a single larger texture (an atlas) by minimizing empty space. Uses a tree structure to minimize wasted space. Elements
	 * can be dynamically added and removed from the layout.
	 */
	class B3D_EXPORT TreeTextureAtlasLayout
	{
		/** Determines how are child nodes laid out in a container node. */
		enum class NodeOrientation
		{
			Vertical, Horizontal
		};

		/** State a node in the tree can be in. */
		enum class NodeState
		{
			Container, /**< Contains child nodes. */
			Allocated, /**< Leaf node that contains data. */
			Free, /**< Leaf node that is free. */
			Unused /**< Node not part of the tree. */
		};

		/** Represent a single node in the texture atlas tree. */
		struct Node
		{
			u32 ParentNodeId = ~0u;
			u32 NextSiblingId = ~0u;
			u32 PreviousSiblingId = ~0u;
			NodeState State = NodeState::Unused;
			NodeOrientation Orientation = NodeOrientation::Vertical;
			Area2I Area;
		};

		/** Contains a list of free nodes for a particular size. */
		struct FreeNodeBucket
		{
			u32 Size = ~0u; /**< Any node that is lower than this size in all dimensions will be part of this bucket. ~0u if unconstrained. */
			Vector<u32> FreeNodes;
		};

		/** Buckets in which to organize free nodes in. */
		enum class FreeNodeBucketType
		{
			Small, Medium, Large, Count
		};

		/** Result of a node split operation. */
		struct NodeSplitResult
		{
			Area2I SmallerLeftoverArea;
			Area2I LargerLeftoverArea;

			/**
			 * If horizontal, larger rectangle is placed to the right of the requested area (which is always in the top right of the node), and smaller
			 * rectangle is placed below the requested area.
			 *
			 * Similarly, if vertical, larger rectangle is placed below the requested area, and smaller to the right of the requested area.
			 */
			NodeOrientation LargerAreaOrientation = NodeOrientation::Vertical;
		};

		/** Single atlas page. */
		struct Page
		{
			u32 RootNodeId = ~0u;
			Array<FreeNodeBucket, (u32)FreeNodeBucketType::Count> FreeNodeBuckets;
		};

	public:
		struct Allocation
		{
			u32 NodeId = ~0u;
			u32 PageId = ~0u;
			Vector2I Position{kZeroTag};
		};

		TreeTextureAtlasLayout(const TreeTextureAtlasLayoutSettings& settings = TreeTextureAtlasLayoutSettings());

		/**
		 * Attempts to add a new element in the layout. 
		 *
		 * @param size	Size of the element to add, in pixels.
		 * @return		Location at which the element was stored, or null if no free space was located.
		 */
		TOptional<Allocation> AddElement(const Size2UI& size);

		/** Removes an element from the provided node. */
		void RemoveElement(u32 pageId, u32 nodeId);

		/** Grows the atlas without changing existing allocations. New size cannot be smaller than existing size. */
		void Grow(const Size2UI& newSize);

		/** Removes all entries from the layout. */
		void Clear();

		/** Checks have any elements been added to the layout. */
		bool IsEmpty() const { return mPages.size() == 0; }

		/** Checks is a specific page empty. */
		bool IsPageEmpty(u32 pageId) const;

		/** Returns the size of the atlas texture, in pixels. */
		const Size2UI& GetSize() const { return mSettings.Size; }

	private:
		/** Finds a bucket that stores nodes of the provided size. */
		u32 GetFreeNodeBucketForSize(Page& page, const Size2UI& size) const;

		/** Aligns the provided size to the alignment requested in layout settings. */
		Size2UI AlignSize(const Size2UI& size) const;

		/** Applies padding to the size as requested in layout settings. */
		Size2UI PadSize(const Size2UI& size) const;

		/** Splits a node and returns leftover rectangles, if there are any. */
		NodeSplitResult Split(const Node& nodeToSplit, const Size2UI& requiredSize) const;

		/** Finds the index of the best fitting node that can be the provided area. Returns ~0u if no such node can be found. */
		u32 FindBestFreeNode(Page& page, const Size2UI& size);

		/** Registers the node in the appropriate free node bucket. */
		void RegisterFreeNode(Page& page, u32 nodeId, const Size2UI& size);

		/** Allocates a new mode in the nodes array and returns the allocated index. This method will attempt to recycle unused nodes before allocating new nodes. */
		u32 AllocateNode();

		/** Marks the node as unused and frees it for re-allocation. */
		void FreeNode(u32 nodeId);

		/** Allocates a new atlas page. */
		Page AllocatePage();

		/** Frees a page. Assumes the page contains only the root node. */
		void FreePage(u32 pageId);

		/** Attempts to merge the next sibling of the provided node into the node, if the sibling exists and is free. */
		void MergeWithNextSibling(u32 nodeId);

		TreeTextureAtlasLayoutSettings mSettings;

		Vector<Node> mNodes;
		TInlineArray<Page, 2> mPages;
		u32 mUnusedNodeListHead = ~0u; /**< Index into mNodex of the first unused node, stored as a linked list. ~0u if no unused nodes. */
	};

	/** Utility class used for texture atlas layouts. */
	class B3D_EXPORT TextureAtlasUtility
	{
	public:
		/**
		 * Represents a single element used as in input to TextureAtlasUtility. Usually represents a single texture.
		 *
		 * @note	input is required to be filled in before passing it to TextureAtlasUtility.
		 * @note	output will be filled after a call to TextureAtlasUtility::createAtlasLayout().
		 */
		struct Element
		{
			struct
			{
				u32 Width, Height;
			} Input;

			struct
			{
				u32 X, Y;
				u32 Idx;
				i32 Page;
			} Output;
		};

		/** Describes a single page of the texture atlas. */
		struct Page
		{
			u32 Width, Height;
		};

		/**
		 * Creates an optimal texture layout by packing texture elements in order to end up with as little empty space
		 * as possible. Algorithm will split elements over multiple textures if they don't fit in a single texture.
		 *
		 * @param	elements	Elements to process. They need to have their input structures filled in,
		 * 						and this method will fill output when it returns.
		 * @param	width 		Initial width of the atlas texture.
		 * @param	height		Initial height of the atlas texture.
		 * @param	maxWidth	Maximum width the atlas texture is allowed to grow to, when elements don't fit.
		 * @param	maxHeight	Maximum height the atlas texture is allowed to grow to, when elements don't fit.
		 * @param	pow2		When true the resulting atlas size will always be a power of two.
		 * @return				One or more descriptors that determine the size of the final atlas textures.
		 *						Texture elements will reference these pages with their output.page parameter.
		 */
		static Vector<Page> CreateAtlasLayout(Vector<Element>& elements, u32 width, u32 height, u32 maxWidth, u32 maxHeight, bool pow2 = false);
	};

	/** @} */
} // namespace b3d
