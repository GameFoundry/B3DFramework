//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsRect2I.h"
#include "Math/BsMatrix4.h"
#include "Math/BsMath.h"

using namespace bs;

const Rect2I Rect2I::kEmpty;

Rect2I::Rect2I(const Vector2I& position, const Size2UI& size)
	: X(position.X), Y(position.Y), Width(size.Width), Height(size.Height)
{ }

bool Rect2I::Contains(const Vector2I& point) const
{
	if(point.X >= X && point.X < (X + (i32)Width))
	{
		if(point.Y >= Y && point.Y < (Y + (i32)Height))
			return true;
	}

	return false;
}

bool Rect2I::Contains(const Rect2I& other) const
{
	const i32 right = X + (i32)Width;
	const i32 bottom = Y + (i32)Height;

	const i32 otherRight = other.X + (i32)other.Width;
	const i32 otherBottom = other.Y + (i32)other.Height;

	return other.X >= X && otherRight <= right && other.Y >= Y && otherBottom <= bottom;
}

bool Rect2I::Overlaps(const Rect2I& other) const
{
	i32 otherRight = other.X + (i32)other.Width;
	i32 myRight = X + (i32)Width;

	i32 otherBottom = other.Y + (i32)other.Height;
	i32 myBottom = Y + (i32)Height;

	if(X < otherRight && myRight > other.X &&
	   Y < otherBottom && myBottom > other.Y)
		return true;

	return false;
}

void Rect2I::SetPosition(const Vector2I& position)
{
	X = position.X;
	Y = position.Y;
}

void Rect2I::SetSize(const Size2UI& size)
{
	Width = size.Width;
	Height = size.Height;
}

void Rect2I::Encapsulate(const Rect2I& other)
{
	int myRight = X + (i32)Width;
	int myBottom = Y + (i32)Height;
	int otherRight = other.X + (i32)other.Width;
	int otherBottom = other.Y + (i32)other.Height;

	if(other.X < X)
		X = other.X;

	if(other.Y < Y)
		Y = other.Y;

	if(otherRight > myRight)
		Width = otherRight - X;
	else
		Width = myRight - X;

	if(otherBottom > myBottom)
		Height = otherBottom - Y;
	else
		Height = myBottom - Y;
}

void Rect2I::Clip(const Rect2I& clipRect)
{
	int newLeft = std::max(X, clipRect.X);
	int newTop = std::max(Y, clipRect.Y);

	int newRight = Math::Clamp(X + (i32)Width, clipRect.X, clipRect.X + (i32)clipRect.Width);
	int newBottom = Math::Clamp(Y + (i32)Height, clipRect.Y, clipRect.Y + (i32)clipRect.Height);

	X = std::min(newLeft, newRight);
	Y = std::min(newTop, newBottom);
	Width = std::max(0, newRight - newLeft);
	Height = std::max(0, newBottom - newTop);
}

u32 Rect2I::Cut(const Rect2I& rectangleToCutWith, Array<Rect2I, 4>& outPieces) const
{
	u32 cutPieceCount = 0;

	// Cut horizontal
	if(rectangleToCutWith.X > X && rectangleToCutWith.X < (X + (i32)Width))
	{
		Rect2I leftPiece;
		leftPiece.X = X;
		leftPiece.Width = rectangleToCutWith.X - X;
		leftPiece.Y = Y;
		leftPiece.Height = Height;

		outPieces[cutPieceCount++] = leftPiece;
	}

	if((rectangleToCutWith.X + (i32)rectangleToCutWith.Width) > X && (rectangleToCutWith.X + (i32)rectangleToCutWith.Width) < (X + (i32)Width))
	{
		Rect2I rightPiece;
		rightPiece.X = rectangleToCutWith.X + rectangleToCutWith.Width;
		rightPiece.Width = (X + Width) - (rectangleToCutWith.X + rectangleToCutWith.Width);
		rightPiece.Y = Y;
		rightPiece.Height = Height;

		outPieces[cutPieceCount++] = rightPiece;
	}

	// Cut vertical
	i32 cutLeft = std::min(std::max(X, rectangleToCutWith.X), X + (i32)Width);
	i32 cutRight = std::max(std::min(X + (i32)Width, rectangleToCutWith.X + (i32)rectangleToCutWith.Width), X);

	if(cutLeft != cutRight)
	{
		if(rectangleToCutWith.Y > Y && rectangleToCutWith.Y < (Y + (i32)Height))
		{
			Rect2I topPiece;
			topPiece.Y = Y;
			topPiece.Height = rectangleToCutWith.Y - Y;
			topPiece.X = cutLeft;
			topPiece.Width = cutRight - cutLeft;

			outPieces[cutPieceCount++] = topPiece;
		}

		if((rectangleToCutWith.Y + (i32)rectangleToCutWith.Height) > Y && (rectangleToCutWith.Y + (i32)rectangleToCutWith.Height) < (Y + (i32)Height))
		{
			Rect2I bottomPiece;
			bottomPiece.Y = rectangleToCutWith.Y + rectangleToCutWith.Height;
			bottomPiece.Height = (Y + Height) - (rectangleToCutWith.Y + rectangleToCutWith.Height);
			bottomPiece.X = cutLeft;
			bottomPiece.Width = cutRight - cutLeft;

			outPieces[cutPieceCount++] = bottomPiece;
		}
	}

	// No cut
	if(cutPieceCount == 0)
	{
		if(rectangleToCutWith.X <= X && (rectangleToCutWith.X + (i32)rectangleToCutWith.Width) >= (X + (i32)Width) &&
		   rectangleToCutWith.Y <= Y && (rectangleToCutWith.Y + (i32)rectangleToCutWith.Height) >= (Y + (i32)Height))
		{
			// Cut rectangle completely encompasses this one
		}
		else
			outPieces[cutPieceCount++] = *this; // Cut rectangle doesn't even touch this one
	}

	return cutPieceCount;
}

void Rect2I::Cut(const Vector<Rect2I>& rectanglesToCutWith, Vector<Rect2I>& pieces) const
{
	FrameScope frameScope;
	FrameVector<Rect2I> temporaryPieceBuffers[2];
	u32 outputBufferIndex = 0;

	temporaryPieceBuffers[0].push_back(*this);

	for(auto& rectangleToCutWith : rectanglesToCutWith)
	{
		const u32 inputBufferIndex = outputBufferIndex;

		outputBufferIndex = (outputBufferIndex + 1) % 2;
		temporaryPieceBuffers[outputBufferIndex].clear();

		for(auto& rectangleToCut : temporaryPieceBuffers[inputBufferIndex])
		{
			Array<Rect2I, 4> cutPieces;
			const u32 pieceCount = rectangleToCut.Cut(rectangleToCutWith, cutPieces);

			temporaryPieceBuffers[outputBufferIndex].insert(temporaryPieceBuffers[outputBufferIndex].end(), cutPieces.data(), cutPieces.data() + pieceCount);
		}
	}

	pieces = Vector<Rect2I>(temporaryPieceBuffers[outputBufferIndex].begin(), temporaryPieceBuffers[outputBufferIndex].end());
}

void Rect2I::Transform(const Matrix4& matrix)
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

template <typename T>
void Rect2I::AddUnique(const Rect2I& area, T& inOutAreaList)
{
	if(area.Width == 0 || area.Height == 0)
		return;

	bool shouldAddArea = true;
	for(auto it = inOutAreaList.begin(); it != inOutAreaList.end();)
	{
		const Rect2I& existingArea = *it;

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
			Array<Rect2I, 4> cutImageAreas;
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

template B3D_UTILITY_EXPORT void Rect2I::AddUnique(const Rect2I&, Vector<Rect2I>&);
template B3D_UTILITY_EXPORT void Rect2I::AddUnique(const Rect2I&, FrameVector<Rect2I>&);
