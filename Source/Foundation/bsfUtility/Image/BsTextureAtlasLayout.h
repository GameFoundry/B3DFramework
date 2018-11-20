//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsVector2.h"

namespace bs
{
	/** @addtogroup Image
	 *  @{
	 */

	/** Organizes a set of textures into a single larger texture (an atlas) by minimizing empty space. */
	class BS_UTILITY_EXPORT TextureAtlasLayout
	{
		/** Represent a single node in the texture atlas binary tree. */
		class TexAtlasNode
		{
		public:
			constexpr TexAtlasNode() = default;
			constexpr TexAtlasNode(UINT32 x, UINT32 y, UINT32 width, UINT32 height)
				: x(x), y(y), width(width), height(height)
			{ }

			UINT32 x = 0;
			UINT32 y = 0;
			UINT32 width = 0;
			UINT32 height = 0;
			UINT32 children[2] { std::numeric_limits<UINT32>::max(), std::numeric_limits<UINT32>::max() };
			bool nodeFull = false;
		};

	public:
		TextureAtlasLayout() = default;

		/**
		 * Constructs a new texture atlas layout with the provided parameters.
		 *
		 * @param[in]	width 			Initial width of the atlas texture. 
		 * @param[in]	height			Initial height of the atlas texture.
		 * @param[in]	maxWidth		Maximum width the atlas texture is allowed to grow to, when elements don't fit.
		 * @param[in]	maxHeight		Maximum height the atlas texture is allowed to grow to, when elements don't fit.
		 * @param[in]	pow2			When true the resulting atlas size will always be a power of two.
		 */
		TextureAtlasLayout(UINT32 width, UINT32 height, UINT32 maxWidth, UINT32 maxHeight, bool pow2 = false)
			: mInitialWidth(width), mInitialHeight(height), mWidth(width), mHeight(height), mPow2(pow2)
		{
			mNodes.push_back(TexAtlasNode(0, 0, maxWidth, maxHeight));
		}

		/**
		 * Attempts to add a new element in the layout. Elements should be added to the atlas from largest to smallest,
		 * otherwise a non-optimal layout is likely to be generated.
		 * 
		 * @param[in]	width	Width of the new element, in pixels.
		 * @param[in]	height	Height of the new element, in pixels.
		 * @param[out]	x		Horizontal position of the new element within the atlas. Only valid if method returns true.
		 * @param[out]	y		Vertical position of the new element within the atlas. Only valid if method returns true.
		 * @return				True if the element was added to the atlas, false if the element doesn't fit.
		 */
		bool addElement(UINT32 width, UINT32 height, UINT32& x, UINT32& y);

		/** Removes all entries from the layout. */
		void clear();

		/** Checks have any elements been added to the layout. */
		bool isEmpty() const { return mNodes.size() == 1; }

		/** Returns the width of the atlas texture, in pixels. */
		UINT32 getWidth() const { return mWidth; }

		/** Returns the height of the atlas texture, in pixels. */
		UINT32 getHeight() const { return mHeight; }

	private:
		/* 
		 * Attempts to add a new element to the specified layout node. 
		 * 
		 * @param[in]	nodeIdx			Index of the node to which to add the element.
		 * @param[in]	width			Width of the new element, in pixels.
		 * @param[in]	height			Height of the new element, in pixels.
		 * @param[out]	x				Horizontal position of the new element within the atlas. Only valid if method
		 *								returns true.
		 * @param[out]	y				Vertical position of the new element within the atlas. Only valid if method returns
		 *								true.
		 * @param[in]	allowGrowth		When true, the width/height of the atlas will be allowed to grow to fit the element.
		 * @return						True if the element was added to the atlas, false if the element doesn't fit.
		 */
		bool addToNode(UINT32 nodeIdx, UINT32 width, UINT32 height, UINT32& x, UINT32& y, bool allowGrowth);

		UINT32 mInitialWidth = 0;
		UINT32 mInitialHeight = 0;
		UINT32 mWidth = 0;
		UINT32 mHeight = 0;
		bool mPow2 = false;

		Vector<TexAtlasNode> mNodes;
	};

	/** Utility class used for texture atlas layouts. */
	class BS_UTILITY_EXPORT TextureAtlasUtility
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
				UINT32 width, height;
			} input;
		
			struct
			{
				UINT32 x, y;
				UINT32 idx;
				INT32 page;
			} output;
		};

		/** Describes a single page of the texture atlas. */
		struct Page
		{
			UINT32 width, height;
		};

		/**
		 * Creates an optimal texture layout by packing texture elements in order to end up with as little empty space 
		 * as possible. Algorithm will split elements over multiple textures if they don't fit in a single texture.
		 *
		 * @param[in]	elements	Elements to process. They need to have their input structures filled in,
		 * 							and this method will fill output when it returns.
		 * @param[in]	width 		Initial width of the atlas texture.
		 * @param[in]	height		Initial height of the atlas texture.
		 * @param[in]	maxWidth	Maximum width the atlas texture is allowed to grow to, when elements don't fit.
		 * @param[in]	maxHeight	Maximum height the atlas texture is allowed to grow to, when elements don't fit.
		 * @param[in]	pow2		When true the resulting atlas size will always be a power of two.
		 * @return					One or more descriptors that determine the size of the final atlas textures. 
		 *							Texture elements will reference these pages with their output.page parameter.
		 */
		static Vector<Page> createAtlasLayout(Vector<Element>& elements, UINT32 width, UINT32 height, UINT32 maxWidth, 
			UINT32 maxHeight, bool pow2 = false);
	};

	/** @} */
}
