//************************************ bs::framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsArea2.h"
#include "Math/BsMatrix4.h"
#include "Math/BsMath.h"
#include "Math/BsVector2.h"

using namespace bs;

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
	FrameScope frameScope;
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

template<>
template<>
void B3D_UTILITY_EXPORT TArea2<i32, u32>::Transform(const Matrix4& matrix)
{
	Vector4 verts[4];
	verts[0] = Vector4((float)X, (float)Y, 0.0f, 1.0f);
	verts[1] = Vector4((float)X + Width, (float)Y, 0.0f, 1.0f);
	verts[2] = Vector4((float)X, (float)Y + Height, 0.0f, 1.0f);
	verts[3] = Vector4((float)X + Width, (float)Y + Height, 0.0f, 1.0f);

	for(u32 i = 0; i < 4; i++)
		verts[i] = matrix.Multiply(verts[i]);

	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::min();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::min();

	for(u32 i = 0; i < 4; i++)
	{
		if(verts[i].X < minX)
			minX = verts[i].X;

		if(verts[i].Y < minY)
			minY = verts[i].Y;

		if(verts[i].X > maxX)
			maxX = verts[i].X;

		if(verts[i].Y > maxY)
			maxY = verts[i].Y;
	}

	X = Math::FloorToInt(minX);
	Y = Math::FloorToInt(minY);
	Width = (u32)Math::CeilToInt(maxX) - X;
	Height = (u32)Math::CeilToInt(maxY) - Y;
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

template B3D_UTILITY_EXPORT void TArea2<i32>::AddUnique(const TArea2&, Vector<TArea2>&);
template B3D_UTILITY_EXPORT void TArea2<i32, u32>::AddUnique(const TArea2&, Vector<TArea2>&);
template B3D_UTILITY_EXPORT void TArea2<float>::AddUnique(const TArea2&, Vector<TArea2>&);

template B3D_UTILITY_EXPORT void TArea2<i32>::AddUnique(const TArea2&, FrameVector<TArea2>&);
template B3D_UTILITY_EXPORT void TArea2<i32, u32>::AddUnique(const TArea2&, FrameVector<TArea2>&);
template B3D_UTILITY_EXPORT void TArea2<float>::AddUnique(const TArea2&, FrameVector<TArea2>&);

template B3D_UTILITY_EXPORT TArea2<i32>;
template B3D_UTILITY_EXPORT TArea2<i32, u32>;
template B3D_UTILITY_EXPORT TArea2<float>;
