//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsRect2I.h"
#include "Math/BsVector2I.h"
#include "Math/BsMatrix4.h"
#include "Math/BsMath.h"

using namespace bs;

const Rect2I Rect2I::kEmpty;

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

void Rect2I::Cut(const Rect2I& cutRect, Vector<Rect2I>& pieces)
{
	u32 initialPieces = (u32)pieces.size();

	// Cut horizontal
	if(cutRect.X > X && cutRect.X < (X + (i32)Width))
	{
		Rect2I leftPiece;
		leftPiece.X = X;
		leftPiece.Width = cutRect.X - X;
		leftPiece.Y = Y;
		leftPiece.Height = Height;

		pieces.push_back(leftPiece);
	}

	if((cutRect.X + (i32)cutRect.Width) > X && (cutRect.X + (i32)cutRect.Width) < (X + (i32)Width))
	{
		Rect2I rightPiece;
		rightPiece.X = cutRect.X + cutRect.Width;
		rightPiece.Width = (X + Width) - (cutRect.X + cutRect.Width);
		rightPiece.Y = Y;
		rightPiece.Height = Height;

		pieces.push_back(rightPiece);
	}

	// Cut vertical
	i32 cutLeft = std::min(std::max(X, cutRect.X), X + (i32)Width);
	i32 cutRight = std::max(std::min(X + (i32)Width, cutRect.X + (i32)cutRect.Width), X);

	if(cutLeft != cutRight)
	{
		if(cutRect.Y > Y && cutRect.Y < (Y + (i32)Height))
		{
			Rect2I topPiece;
			topPiece.Y = Y;
			topPiece.Height = cutRect.Y - Y;
			topPiece.X = cutLeft;
			topPiece.Width = cutRight - cutLeft;

			pieces.push_back(topPiece);
		}

		if((cutRect.Y + (i32)cutRect.Height) > Y && (cutRect.Y + (i32)cutRect.Height) < (Y + (i32)Height))
		{
			Rect2I bottomPiece;
			bottomPiece.Y = cutRect.Y + cutRect.Height;
			bottomPiece.Height = (Y + Height) - (cutRect.Y + cutRect.Height);
			bottomPiece.X = cutLeft;
			bottomPiece.Width = cutRight - cutLeft;

			pieces.push_back(bottomPiece);
		}
	}

	// No cut
	if(initialPieces == (u32)pieces.size())
	{
		if(cutRect.X <= X && (cutRect.X + (i32)cutRect.Width) >= (X + (i32)Width) &&
		   cutRect.Y <= Y && (cutRect.Y + (i32)cutRect.Height) >= (Y + (i32)Height))
		{
			// Cut rectangle completely encompasses this one
		}
		else
			pieces.push_back(*this); // Cut rectangle doesn't even touch this one
	}
}

void Rect2I::Cut(const Vector<Rect2I>& cutRects, Vector<Rect2I>& pieces)
{
	Vector<Rect2I> tempPieces[2];
	u32 bufferIdx = 0;

	tempPieces[0].push_back(*this);

	for(auto& cutRect : cutRects)
	{
		u32 currentBufferIdx = bufferIdx;

		bufferIdx = (bufferIdx + 1) % 2;
		tempPieces[bufferIdx].clear();

		for(auto& rect : tempPieces[currentBufferIdx])
			rect.Cut(cutRect, tempPieces[bufferIdx]);
	}

	pieces = tempPieces[bufferIdx];
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
