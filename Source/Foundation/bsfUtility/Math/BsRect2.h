//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsVector2.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Represents a 2D rectangle using real values. Rectangle is represented with an origin in top left and width/height. */
	class BS_UTILITY_EXPORT Rect2
	{
	public:
		Rect2() = default;

		Rect2(float x, float y, float width, float height)
			: X(x), Y(y), Width(width), Height(height)
		{}

		Rect2(const Vector2& topLeft, const Vector2& size)
			: X(topLeft.X), Y(topLeft.Y), Width(size.X), Height(size.Y)
		{}

		float X = 0.0f;
		float Y = 0.0f;
		float Width = 0.0f;
		float Height = 0.0f;

		/** Returns true if the rectangle contains the provided point. */
		bool Contains(const Vector2& point) const;

		/**
		 * Returns true if the rectangle overlaps the provided rectangle. Also returns true if the rectangles are contained
		 * within each other completely (no intersecting edges).
		 */
		bool Overlaps(const Rect2& other) const;

		/** Extends this rectangle so that the provided rectangle is completely contained within it. */
		void Encapsulate(const Rect2& other);

		/** Clips current rectangle so that it does not overlap the provided rectangle. */
		void Clip(const Rect2& clipRect);

		/**
		 * Transforms the bounds by the given matrix. Resulting value is an axis aligned rectangle encompassing the
		 * transformed points.
		 *
		 * @note	Since the resulting value is an AA rectangle of the original transformed rectangle, the bounds
		 * 			will be larger than needed. Oriented rectangle would provide a much tighter fit.
		 */
		void Transform(const Matrix4& matrix);

		/** Center of the rectangle. */
		Vector2 GetCenter() const;

		/** Extents of the rectangle (distance from center to one of the corners) */
		Vector2 GetHalfSize() const;

		bool operator==(const Rect2& rhs) const
		{
			return X == rhs.X && Y == rhs.Y && Width == rhs.Width && Height == rhs.Height;
		}

		bool operator!=(const Rect2& rhs) const
		{
			return !(*this == rhs);
		}

		static const Rect2 kEmpty;
	};

	/** @} */
} // namespace bs
