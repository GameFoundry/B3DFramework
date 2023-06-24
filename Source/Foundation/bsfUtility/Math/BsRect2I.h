//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Represents a 2D rectangle using integer values. Rectangle is represented with an origin in top left and width/height. */
	class B3D_UTILITY_EXPORT Rect2I
	{
	public:
		constexpr Rect2I() = default;

		constexpr Rect2I(i32 x, i32 y, u32 width, u32 height)
			: X(x), Y(y), Width(width), Height(height)
		{}

		i32 X = 0;
		i32 Y = 0;
		u32 Width = 0;
		u32 Height = 0;

		/** Returns true if the rectangle contains the provided point. */
		bool Contains(const Vector2I& point) const;

		/** Returns true if the rectangle fully contains the provided rectangle. */
		bool Contains(const Rect2I& other) const;

		/**
		 * Returns true if the rectangle overlaps the provided rectangle. Also returns true if the rectangles are contained
		 * within each other completely (no intersecting edges).
		 */
		bool Overlaps(const Rect2I& other) const;

		/** Extends this rectangle so that the provided rectangle is completely contained within it. */
		void Encapsulate(const Rect2I& other);

		/** Clips current rectangle so that it does not overlap the provided rectangle. */
		void Clip(const Rect2I& clipRect);

		/**
		 * Cuts the current rectangle with the provided rectangle and outputs the pieces. The pieces will contain all area
		 * of the current rectangle without including the cut rectangle area.
		 */
		u32 Cut(const Rect2I& rectangleToCutWith, Array<Rect2I, 4>& outPieces) const;

		/**
		 * Cuts the current rectangle with the provided rectangles and outputs the pieces. The pieces will contain all area
		 * of the current rectangle without including the cut rectangles area.
		 */
		void Cut(const Vector<Rect2I>& rectanglesToCutWith, Vector<Rect2I>& outPieces) const;

		/**
		 * Transforms the bounds by the given matrix. Resulting value is an axis aligned rectangle encompassing the
		 * transformed points.
		 *
		 * @note
		 * Since the resulting value is an AA rectangle of the original transformed rectangle, the bounds will be larger
		 * than needed. Oriented rectangle would provide a much tighter fit.
		 */
		void Transform(const Matrix4& matrix);

		/**
		 * Adds a unique area to the provided @p outAreas list. The list will be modified by adding, removing or cutting areas in the list to ensure
		 * only non-overlapping areas are present in the list.
		 *
		 * @param	area		Area to try to add to the list.
		 * @param	inOutAreas	List of unique areas to append the result to. Must be Vector<T> or FrameVector<T>.
		 */
		template<class T>
		static void AddUnique(const Rect2I& area, T& inOutAreas);

		bool operator==(const Rect2I& rhs) const
		{
			return X == rhs.X && Y == rhs.Y && Width == rhs.Width && Height == rhs.Height;
		}

		bool operator!=(const Rect2I& rhs) const
		{
			return !(*this == rhs);
		}

		static const Rect2I kEmpty;
	};

	/** @} */
} // namespace bs
