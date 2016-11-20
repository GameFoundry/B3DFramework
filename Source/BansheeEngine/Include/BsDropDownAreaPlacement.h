//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsVector2I.h"
#include "BsRect2I.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**
	 * Determines how will the drop down box be positioned. Usually the system will attempt to position the drop box in a 
	 * way so all elements can fit, and this class allows you to specify some limitations on how that works. 
	 * 			
	 * @note	For example, list boxes usually want drop down boxes to be placed above or below them, while
	 * 			context menus may want to have them placed around a single point in any direction.
	 */
	class BS_EXPORT DropDownAreaPlacement
	{
	public:
		/**	Determines how will the drop down box be positioned. */
		enum class Type
		{
			Position,
			BoundsVert,
			BoundsHorz,
			BoundsAll
		};

		/**
		 * Preferred horizontal direction of the placement bounds, either to the left or to the right of the wanted 
		 * position/bounds.
		 */
		enum class HorzDir
		{
			Left, Right
		};

		/**
		 * Preferred horizontal direction of the placement bounds, either upward or downward of the wanted position/bounds.
		 */
		enum class VertDir
		{
			Up, Down
		};

		DropDownAreaPlacement() { }

		/**
		 * Drop down box will be placed at the specified position. By default the system prefers the top left corner of the
		 * box to correspond to the position, but if other corners offer more space for the contents, those will be used 
		 * instead.
		 */
		static DropDownAreaPlacement aroundPosition(const Vector2I& position);

		/**
		 * Drop down box will be placed at the specified bounds. Box will be horizontally aligned to the left of the 
		 * provided bounds. Vertically system prefers placing the box at the bottom of the bounds, but may choose to align
		 * it with the top of the bounds if it offers more space for the contents.
		 */
		static DropDownAreaPlacement aroundBoundsVert(const Rect2I& bounds);
		
		/**
		 * Drop down box will be placed at the specified bounds. Box will be vertically aligned to the top of the provided
		 * bounds. Horizontally system prefers placing the box at the right of the bounds, but may choose to align it with
		 * the left of the bounds if it offers more space for the contents.
		 */
		static DropDownAreaPlacement aroundBoundsHorz(const Rect2I& bounds);

		/**
		 * Drop down box will be placed at the specified bounds. Box will be vertically aligned to the top or bottom of the
		 * provided bounds, with bottom being preferred. Horizontally system prefers placing the box at the right of the
		 * bounds, but may choose to align it with the left of the bounds if it offers more space for the contents.
		 */
		static DropDownAreaPlacement aroundBounds(const Rect2I& bounds);

		/**	Returns drop down box positioning type. */
		Type getType() const { return mType; }

		/** Returns bounds around which to position the drop down box if one of the bounds positioning types is used. */
		const Rect2I& getBounds() const { return mBounds; }

		/**	Returns position around which to position the drop down box if position positioning type is used. */
		const Vector2I& getPosition() const { return mPosition; }

		/**
		 * Calculates the optimal bounds to place an element of the specified size, within the available area using the 
		 * internal data as a guide.
		 *
		 * @param[in]	width			Width of the element to try to position, in pixels.
		 * @param[in]	height			Height of the element to try to position, in pixels.
		 * @param[in]	availableArea	Available area to try to position the element in, in pixels.
		 * @param[in]	horzDir			Output parameter that signals the preferred horizontal direction of the bounds 
		 *								(left or right).
		 * @param[in]	vertDir			Output parameter that signals the preferred vertical direction of the bounds 
		 *								(up or down).
		 */
		Rect2I getOptimalBounds(UINT32 width, UINT32 height, const Rect2I& availableArea, HorzDir& horzDir, 
			VertDir& vertDir) const;

	private:
		Type mType;
		Rect2I mBounds;
		Vector2I mPosition;
	};

	/** @} */
}