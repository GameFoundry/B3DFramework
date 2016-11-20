//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsVector2.h"

namespace bs
{
	/** @addtogroup Image
	 *  @{
	 */

	/**
	 * Represents a single element used as in input to TexAtlasGenerator. Usually represents a single texture.
	 * 			
	 * @note	input is required to be filled in before passing it to TexAtlasGenerator.
	 * @note	output will be filled in by TexAtlasGenerator after a call to TexAtlasGenerator::createAtlasLayout().
	 */
	struct TexAtlasElementDesc
	{
		struct
		{
			UINT32 width, height;
		} input;
		
		struct
		{
			UINT32 x, y;
			INT32 page;
		} output;
	};

	/** A single page of the texture atlas. */
	struct TexAtlasPageDesc
	{
		UINT32 width, height;
	};

	class TexAtlasNode;

	/** Organizes a set of textures into a single larger texture (an atlas) by minimizing empty space. */
	class BS_UTILITY_EXPORT TexAtlasGenerator
	{
	public:
		/**
		 * Constructs a new texture atlas generator with the provided parameters.
		 *
		 * @param[in]	square			(optional) Should the returned texture always be square. (width == height)
		 * 								This option is only used if @p fixedSize parameter is set to false.
		 * @param[in]	maxTexWidth 	(optional) Maximum width of the texture. 
		 * @param[in]	maxTexHeight	(optional) Maximum height of the texture. 
		 * @param[in]	fixedSize   	(optional) If this field is false, algorithm will try to reduce the size of the texture
		 * 								if possible. If it is true, the algorithm will always produce textures of the specified
		 * 								@p maxTexWidth, @p maxTexHeight size.
		 */
		TexAtlasGenerator(bool square = false, UINT32 maxTexWidth = 2048, UINT32 maxTexHeight = 2048, bool fixedSize = false);

		/**
		 * Creates an optimal texture layout by packing texture elements in order to end up with as little empty space 
		 * as possible.
		 *
		 * @param[in]	elements	Elements to process. They need to have their input structures filled in,
		 * 							and this method will fill output when it returns.
		 * @return					One or more descriptors that determine the size of the final atlas textures. 
		 *							Texture elements will reference these pages with their output.page parameter.
		 *
		 * @note	
		 * Algorithm will split elements over multiple textures if they don't fit in a single texture (Determined by 
		 * maximum texture size).
		 */
		Vector<TexAtlasPageDesc> createAtlasLayout(Vector<TexAtlasElementDesc>& elements) const;

	private:
		bool mSquare;
		bool mFixedSize;
		UINT32 mMaxTexWidth;
		UINT32 mMaxTexHeight;

		/**
		 * Organize all of the provide elements and place them into minimum number of pages with the specified width and height.
		 * 			
		 * Caller must ensure @p elements array has the page indexes reset to -1 before calling, otherwise it will be assumed
		 * those elements already have assigned pages.
		 * 			
		 * Using @p startPage parameter you may add an offset to the generated page indexes.
		 *
		 * @return	Number of pages generated.
		 */
		int generatePagesForSize(Vector<TexAtlasElementDesc>& elements, UINT32 width, UINT32 height, UINT32 startPage = 0) const;

		/**
		 * Finds the largest element without a page that fits within the provided node.
		 *
		 * @return	Array index of the found page, or -1 if all textures have a page.
		 */
		int addLargestTextureWithoutPageThatFits(Vector<TexAtlasElementDesc>& elements, TexAtlasNode& node) const;

		/**
		 * Scan all of the provided elements and find the largest one that still doesn't have a page assigned.
		 * 			
		 * @return	Array index of the found page, or -1 if all textures have a page.
		 */
		int findLargestTextureWithoutPage(const Vector<TexAtlasElementDesc>& elements) const;

		/** Sorts all the texture elements so that larget elements come first. */
		void sortBySize(Vector<TexAtlasElementDesc>& elements) const;
	};

	/** @} */
}