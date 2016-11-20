//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Represents a 2D rectangle using real values. Rectangle is represented with an origin in top left and width/height. */
	class BS_UTILITY_EXPORT Rect2
	{
	public:
		Rect2();
		Rect2(float _x, float _y, float _width, float _height);

		float x, y, width, height;

		/** Returns true if the rectangle contains the provided point. */
		bool contains(const Vector2& point) const;

		/**
		 * Returns true if the rectangle overlaps the provided rectangle. Also returns true if the rectangles are contained 
		 * within each other completely (no intersecting edges).
		 */
		bool overlaps(const Rect2& other) const;

		/** Extends this rectangle so that the provided rectangle is completely contained within it. */
		void encapsulate(const Rect2& other);

		/** Clips current rectangle so that it does not overlap the provided rectangle. */
		void clip(const Rect2& clipRect);

		/**
		 * Transforms the bounds by the given matrix. Resulting value is an axis aligned rectangle encompassing the 
		 * transformed points.
		 * 			
		 * @note	Since the resulting value is an AA rectangle of the original transformed rectangle, the bounds
		 * 			will be larger than needed. Oriented rectangle would provide a much tighter fit.
		 */
		void transform(const Matrix4& matrix);

		bool operator== (const Rect2& rhs) const
		{
			return x == rhs.x && y == rhs.y && width == rhs.width && height == rhs.height;
		}

		bool operator!= (const Rect2& rhs) const
		{
			return !(*this == rhs);
		}

		static const Rect2 EMPTY;
	};

	/** @} */

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(Rect2);
	/** @endcond */
}