//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DSize2.h"
#include "Math/B3DVector2.h"
#include "Math/B3DVector4.h"
#include "B3DMatrix4.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Represents a 2D area. Area is represented with an origin in top left and width/height. */
	template<typename PositionType, typename SizeType = PositionType>
	struct TArea2
	{
		constexpr TArea2() = default;

		constexpr TArea2(PositionType x, PositionType y, SizeType width, SizeType height)
			: X(x), Y(y), Width(width), Height(height)
		{}

		TArea2(const TVector2<PositionType>& position, const TSize2<SizeType>& size);

		PositionType X = (PositionType)0;
		PositionType Y = (PositionType)0;
		SizeType Width = (SizeType)0;
		SizeType Height = (SizeType)0;

		/** Converts a unit with one underlying type to another. */
		template<typename PositionType2, typename SizeType2 = PositionType2>
		TArea2<PositionType2, SizeType2> To() const { return TArea2<PositionType2, SizeType2>((PositionType2)X, (PositionType2)Y, (SizeType2)Width, (SizeType2)Height); }

		/** Returns true if the area covered is 0. */
		bool IsEmpty() const { return Width == 0 || Height == 0; }

		/** Returns the top left corner of the area. */
		TVector2<PositionType> GetPosition() const { return TVector2<PositionType>(X, Y); }

		/** Returns the size of the area. */
		TSize2<SizeType> GetSize() const { return TSize2<SizeType>(Width, Height); }

		/** Center of the rectangle. */
		TVector2<PositionType> GetCenter() const { return TVector2<PositionType>(X + Width / (PositionType)2, Y + Height / (PositionType)2); }

		/** Returns true if the area contains the provided point. */
		bool Contains(const TVector2<PositionType>& point) const;

		/** Returns true if the area fully contains the provided rectangle. */
		bool Contains(const TArea2& other) const;

		/**
		 * Returns true if the area overlaps the provided area. Also returns true if the areas are contained
		 * within each other completely (no intersecting edges).
		 */
		bool Overlaps(const TArea2& other) const;

		/** Extends this area so that the provided area is completely contained within it. */
		void Encapsulate(const TArea2& other);

		/** Sets the X/Y coordinates of the area. */
		void SetPosition(const TVector2<PositionType>& position);

		/** Sets the width/height of the area. */
		void SetSize(const TSize2<SizeType>& size);

		/** Clips current area so that it does not overlap the provided area. */
		void Clip(const TArea2& clipRect);

		/**
		 * Cuts the current area with the provided area and outputs the pieces. The pieces will contain all area
		 * of the current area without including the cut area area.
		 */
		u32 Cut(const TArea2& rectangleToCutWith, Array<TArea2, 4>& outPieces) const;

		/**
		 * Cuts the current area with the provided areas and outputs the pieces. The pieces will contain all area
		 * of the current area without including the cut area.
		 */
		void Cut(const Vector<TArea2>& rectanglesToCutWith, Vector<TArea2>& outPieces) const;

		/**
		 * Transforms the bounds by the given matrix. Resulting value is an axis aligned rectangle encompassing the
		 * transformed points.
		 *
		 * @note
		 * Since the resulting value is an AA rectangle of the original transformed rectangle, the bounds will be larger
		 * than needed. Oriented rectangle would provide a much tighter fit.
		 */
		template<typename Condition = PositionType, std::enable_if_t<std::is_same_v<Condition, i32>, int> = 0>
		void Transform(const Matrix4& matrix);

		/**
		 * Adds a unique area to the provided @p outAreas list. The list will be modified by adding, removing or cutting areas in the list to ensure
		 * only non-overlapping areas are present in the list.
		 *
		 * @param	area		Area to try to add to the list.
		 * @param	inOutAreas	List of unique areas to append the result to. Must be Vector<T> or FrameVector<T>.
		 */
		template<class T>
		static void AddUnique(const TArea2& area, T& inOutAreas);

		bool operator==(const TArea2& rhs) const
		{
			return X == rhs.X && Y == rhs.Y && Width == rhs.Width && Height == rhs.Height;
		}

		bool operator!=(const TArea2& rhs) const
		{
			return !(*this == rhs);
		}

		static const TArea2 kEmpty;
	};

	template<typename PositionType, typename SizeType>
	TArea2<PositionType, SizeType>::TArea2(const TVector2<PositionType>& position, const TSize2<SizeType>& size)
		: X(position.X), Y(position.Y), Width(size.Width), Height(size.Height)
	{ }

	template<typename PositionType, typename SizeType>
	bool TArea2<PositionType, SizeType>::Contains(const TVector2<PositionType>& point) const
	{
		if(point.X >= X && point.X < (X + (PositionType)Width))
		{
			if(point.Y >= Y && point.Y < (Y + (PositionType)Height))
				return true;
		}

		return false;
	}

	template<typename PositionType, typename SizeType>
	bool TArea2<PositionType, SizeType>::Contains(const TArea2& other) const
	{
		const PositionType right = X + (PositionType)Width;
		const PositionType bottom = Y + (PositionType)Height;

		const PositionType otherRight = other.X + (PositionType)other.Width;
		const PositionType otherBottom = other.Y + (PositionType)other.Height;

		return other.X >= X && otherRight <= right && other.Y >= Y && otherBottom <= bottom;
	}

	template<typename PositionType, typename SizeType>
	bool TArea2<PositionType, SizeType>::Overlaps(const TArea2& other) const
	{
		PositionType otherRight = other.X + (PositionType)other.Width;
		PositionType myRight = X + (PositionType)Width;

		PositionType otherBottom = other.Y + (PositionType)other.Height;
		PositionType myBottom = Y + (PositionType)Height;

		if(X < otherRight && myRight > other.X && Y < otherBottom && myBottom > other.Y)
			return true;

		return false;
	}

	template<typename PositionType, typename SizeType>
	void TArea2<PositionType, SizeType>::SetPosition(const TVector2<PositionType>& position)
	{
		X = position.X;
		Y = position.Y;
	}

	template<typename PositionType, typename SizeType>
	void TArea2<PositionType, SizeType>::SetSize(const TSize2<SizeType>& size)
	{
		Width = size.Width;
		Height = size.Height;
	}

	template<typename PositionType, typename SizeType>
	void TArea2<PositionType, SizeType>::Encapsulate(const TArea2& other)
	{
		PositionType myRight = X + (PositionType)Width;
		PositionType myBottom = Y + (PositionType)Height;
		PositionType otherRight = other.X + (PositionType)other.Width;
		PositionType otherBottom = other.Y + (PositionType)other.Height;

		if(other.X < X)
			X = other.X;

		if(other.Y < Y)
			Y = other.Y;

		if(otherRight > myRight)
			Width = (SizeType)(otherRight - X);
		else
			Width = (SizeType)(myRight - X);

		if(otherBottom > myBottom)
			Height = SizeType(otherBottom - Y);
		else
			Height = SizeType(myBottom - Y);
	}

	template<typename PositionType, typename SizeType>
	void TArea2<PositionType, SizeType>::Clip(const TArea2& clipRect)
	{
		PositionType newLeft = std::max(X, clipRect.X);
		PositionType newTop = std::max(Y, clipRect.Y);

		PositionType newRight = Math::Clamp(X + (PositionType)Width, clipRect.X, clipRect.X + (PositionType)clipRect.Width);
		PositionType newBottom = Math::Clamp(Y + (PositionType)Height, clipRect.Y, clipRect.Y + (PositionType)clipRect.Height);

		X = std::min(newLeft, newRight);
		Y = std::min(newTop, newBottom);
		Width = (SizeType)std::max((PositionType)0, newRight - newLeft);
		Height = (SizeType)std::max((PositionType)0, newBottom - newTop);
	}

	template<typename PositionType, typename SizeType>
	u32 TArea2<PositionType, SizeType>::Cut(const TArea2& rectangleToCutWith, Array<TArea2, 4>& outPieces) const
	{
		u32 cutPieceCount = 0;

		// Cut horizontal
		if(rectangleToCutWith.X > X && rectangleToCutWith.X < (X + (PositionType)Width))
		{
			TArea2 leftPiece;
			leftPiece.X = X;
			leftPiece.Width = (SizeType)(rectangleToCutWith.X - X);
			leftPiece.Y = Y;
			leftPiece.Height = Height;

			outPieces[cutPieceCount++] = leftPiece;
		}

		if((rectangleToCutWith.X + (PositionType)rectangleToCutWith.Width) > X && (rectangleToCutWith.X + (PositionType)rectangleToCutWith.Width) < (X + (PositionType)Width))
		{
			TArea2 rightPiece;
			rightPiece.X = rectangleToCutWith.X + rectangleToCutWith.Width;
			rightPiece.Width = (SizeType)((X + Width) - (rectangleToCutWith.X + rectangleToCutWith.Width));
			rightPiece.Y = Y;
			rightPiece.Height = Height;

			outPieces[cutPieceCount++] = rightPiece;
		}

		// Cut vertical
		PositionType cutLeft = std::min(std::max(X, rectangleToCutWith.X), X + (PositionType)Width);
		PositionType cutRight = std::max(std::min(X + (PositionType)Width, rectangleToCutWith.X + (PositionType)rectangleToCutWith.Width), X);

		if(cutLeft != cutRight)
		{
			if(rectangleToCutWith.Y > Y && rectangleToCutWith.Y < (Y + (PositionType)Height))
			{
				TArea2 topPiece;
				topPiece.Y = Y;
				topPiece.Height = (SizeType)(rectangleToCutWith.Y - Y);
				topPiece.X = cutLeft;
				topPiece.Width = (SizeType)(cutRight - cutLeft);

				outPieces[cutPieceCount++] = topPiece;
			}

			if((rectangleToCutWith.Y + (PositionType)rectangleToCutWith.Height) > Y && (rectangleToCutWith.Y + (PositionType)rectangleToCutWith.Height) < (Y + (PositionType)Height))
			{
				TArea2 bottomPiece;
				bottomPiece.Y = rectangleToCutWith.Y + rectangleToCutWith.Height;
				bottomPiece.Height = (SizeType)((Y + Height) - (rectangleToCutWith.Y + rectangleToCutWith.Height));
				bottomPiece.X = cutLeft;
				bottomPiece.Width = (SizeType)(cutRight - cutLeft);

				outPieces[cutPieceCount++] = bottomPiece;
			}
		}

		// No cut
		if(cutPieceCount == 0)
		{
			if(rectangleToCutWith.X <= X && (rectangleToCutWith.X + (PositionType)rectangleToCutWith.Width) >= (X + (PositionType)Width) &&
			   rectangleToCutWith.Y <= Y && (rectangleToCutWith.Y + (PositionType)rectangleToCutWith.Height) >= (Y + (PositionType)Height))
			{
				// Cut rectangle completely encompasses this one
			}
			else
				outPieces[cutPieceCount++] = *this; // Cut rectangle doesn't even touch this one
		}

		return cutPieceCount;
	}

	template<typename PositionType, typename SizeType>
	void TArea2<PositionType, SizeType>::Cut(const Vector<TArea2>& rectanglesToCutWith, Vector<TArea2>& pieces) const
	{
		FrameAllocatorScope frameScope;
		FrameVector<TArea2> temporaryPieceBuffers[2];
		u32 outputBufferIndex = 0;

		temporaryPieceBuffers[0].push_back(*this);

		for(auto& rectangleToCutWith : rectanglesToCutWith)
		{
			const u32 inputBufferIndex = outputBufferIndex;

			outputBufferIndex = (outputBufferIndex + 1) % 2;
			temporaryPieceBuffers[outputBufferIndex].clear();

			for(auto& rectangleToCut : temporaryPieceBuffers[inputBufferIndex])
			{
				Array<TArea2, 4> cutPieces;
				const u32 pieceCount = rectangleToCut.Cut(rectangleToCutWith, cutPieces);

				temporaryPieceBuffers[outputBufferIndex].insert(temporaryPieceBuffers[outputBufferIndex].end(), cutPieces.data(), cutPieces.data() + pieceCount);
			}
		}

		pieces = Vector<TArea2>(temporaryPieceBuffers[outputBufferIndex].begin(), temporaryPieceBuffers[outputBufferIndex].end());
	}

	template<typename PositionType, typename SizeType>
	template <typename T>
	void TArea2<PositionType, SizeType>::AddUnique(const TArea2& area, T& inOutAreaList)
	{
		if(area.Width == (SizeType)0 || area.Height == (SizeType)0)
			return;

		bool shouldAddArea = true;
		for(auto it = inOutAreaList.begin(); it != inOutAreaList.end();)
		{
			const TArea2& existingArea = *it;

			if(existingArea.Contains(area))
			{
				shouldAddArea = false;
				break;
			}

			if(area.Contains(existingArea))
			{
				it = inOutAreaList.erase(it);
				continue;
			}

			if(area.Overlaps(existingArea))
			{
				Array<TArea2, 4> cutImageAreas;
				const u32 cutImageAreaCount = area.Cut(existingArea, cutImageAreas);

				for(u32 cutImageAreaIndex = 0; cutImageAreaIndex < cutImageAreaCount; ++cutImageAreaIndex)
					AddUnique(cutImageAreas[cutImageAreaIndex], inOutAreaList);

				shouldAddArea = false;
				break;
			}

			++it;
		}

		if(shouldAddArea)
			inOutAreaList.push_back(area);
	}

	template<> const TArea2<i32> TArea2<i32>::kEmpty;
	template<> const TArea2<i32, u32> TArea2<i32, u32>::kEmpty;
	template<> const TArea2<float> TArea2<float>::kEmpty;

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true)) TArea2<i32>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true)) TArea2<i32, u32>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true)) TArea2<float>;

	/** @} */
}

/** @cond STDLIB */

namespace std
{
	/** Hash value generator for TArea2<T>. */
	template<typename PositionType, typename SizeType>
	struct hash<b3d::TArea2<PositionType, SizeType>>
	{
		size_t operator()(const b3d::TArea2<PositionType, SizeType>& value) const
		{
			size_t hash = 0;
			b3d::B3DCombineHash(hash, value.X);
			b3d::B3DCombineHash(hash, value.Y);
			b3d::B3DCombineHash(hash, value.Width);
			b3d::B3DCombineHash(hash, value.Height);

			return hash;
		}
	};
} // namespace std

/** @endcond */
