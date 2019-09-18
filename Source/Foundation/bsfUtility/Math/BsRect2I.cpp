//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsRect2I.h"
#include "Math/BsVector2I.h"
#include "Math/BsMatrix4.h"
#include "Math/BsMath.h"

namespace bs
{
	const Rect2I Rect2I::EMPTY;

	bool Rect2I::contains(const Vector2I& point) const
	{
		if(point.x >= x && point.x < (x + (INT32)width))
		{
			if(point.y >= y && point.y < (y + (INT32)height))
				return true;
		}

		return false;
	}

	bool Rect2I::overlaps(const Rect2I& other) const
	{
		INT32 otherRight = other.x + (INT32)other.width;
		INT32 myRight = x + (INT32)width;

		INT32 otherBottom = other.y + (INT32)other.height;
		INT32 myBottom = y + (INT32)height;

		if(x < otherRight && myRight > other.x &&
			y < otherBottom && myBottom > other.y)
			return true;

		return false;
	}

	void Rect2I::encapsulate(const Rect2I& other)
	{
		int myRight = x + (INT32)width;
		int myBottom = y + (INT32)height;
		int otherRight = other.x + (INT32)other.width;
		int otherBottom = other.y + (INT32)other.height;

		if(other.x < x)
			x = other.x;

		if(other.y < y)
			y = other.y;

		if(otherRight > myRight)
			width = otherRight - x;
		else
			width = myRight - x;

		if(otherBottom > myBottom)
			height = otherBottom - y;
		else
			height = myBottom - y;
	}

	void Rect2I::clip(const Rect2I& clipRect)
	{
		int newLeft = std::max(x, clipRect.x);
		int newTop = std::max(y, clipRect.y);

		int newRight = Math::clamp(x + (INT32)width, clipRect.x, clipRect.x + (INT32)clipRect.width);
		int newBottom = Math::clamp(y + (INT32)height, clipRect.y, clipRect.y + (INT32)clipRect.height);

		x = std::min(newLeft, newRight);
		y = std::min(newTop, newBottom);
		width = std::max(0, newRight - newLeft);
		height = std::max(0, newBottom - newTop);
	}

	void Rect2I::cut(const Rect2I& cutRect, Vector<Rect2I>& pieces)
	{
		UINT32 initialPieces = (UINT32)pieces.size();

		// Cut horizontal
		if (cutRect.x > x && cutRect.x < (x + (INT32)width))
		{
			Rect2I leftPiece;
			leftPiece.x = x;
			leftPiece.width = cutRect.x - x;
			leftPiece.y = y;
			leftPiece.height = height;

			pieces.push_back(leftPiece);
		}

		if ((cutRect.x + (INT32)cutRect.width) > x && (cutRect.x + (INT32)cutRect.width) < (x + (INT32)width))
		{
			Rect2I rightPiece;
			rightPiece.x = cutRect.x + cutRect.width;
			rightPiece.width = (x + width) - (cutRect.x + cutRect.width);
			rightPiece.y = y;
			rightPiece.height = height;

			pieces.push_back(rightPiece);
		}

		// Cut vertical
		INT32 cutLeft = std::min(std::max(x, cutRect.x), x + (INT32)width);
		INT32 cutRight = std::max(std::min(x + (INT32)width, cutRect.x + (INT32)cutRect.width), x);

		if (cutLeft != cutRight)
		{
			if (cutRect.y > y && cutRect.y < (y + (INT32)height))
			{
				Rect2I topPiece;
				topPiece.y = y;
				topPiece.height = cutRect.y - y;
				topPiece.x = cutLeft;
				topPiece.width = cutRight - cutLeft;

				pieces.push_back(topPiece);
			}

			if ((cutRect.y + (INT32)cutRect.height) > y && (cutRect.y + (INT32)cutRect.height) < (y + (INT32)height))
			{
				Rect2I bottomPiece;
				bottomPiece.y = cutRect.y + cutRect.height;
				bottomPiece.height = (y + height) - (cutRect.y + cutRect.height);
				bottomPiece.x = cutLeft;
				bottomPiece.width = cutRight - cutLeft;

				pieces.push_back(bottomPiece);
			}
		}

		// No cut
		if (initialPieces == (UINT32)pieces.size())
		{
			if (cutRect.x <= x && (cutRect.x + (INT32)cutRect.width) >= (x + (INT32)width) &&
				cutRect.y <= y && (cutRect.y + (INT32)cutRect.height) >= (y + (INT32)height))
			{
				// Cut rectangle completely encompasses this one
			}
			else
				pieces.push_back(*this); // Cut rectangle doesn't even touch this one
		}
	}

	void Rect2I::cut(const Vector<Rect2I>& cutRects, Vector<Rect2I>& pieces)
	{
		Vector<Rect2I> tempPieces[2];
		UINT32 bufferIdx = 0;

		tempPieces[0].push_back(*this);

		for (auto& cutRect : cutRects)
		{
			UINT32 currentBufferIdx = bufferIdx;

			bufferIdx = (bufferIdx + 1) % 2;
			tempPieces[bufferIdx].clear();

			for (auto& rect : tempPieces[currentBufferIdx])
				rect.cut(cutRect, tempPieces[bufferIdx]);
		}

		pieces = tempPieces[bufferIdx];
	}

	void Rect2I::transform(const Matrix4& matrix)
	{
		Vector4 verts[4];
		verts[0] = Vector4((float)x, (float)y, 0.0f, 1.0f);
		verts[1] = Vector4((float)x + width, (float)y, 0.0f, 1.0f);
		verts[2] = Vector4((float)x, (float)y + height, 0.0f, 1.0f);
		verts[3] = Vector4((float)x + width, (float)y + height, 0.0f, 1.0f);

		for(UINT32 i = 0; i < 4; i++)
			verts[i] = matrix.multiply(verts[i]);

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::min();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::min();

		for(UINT32 i = 0; i < 4; i++)
		{
			if(verts[i].x < minX)
				minX = verts[i].x;

			if(verts[i].y < minY)
				minY = verts[i].y;

			if(verts[i].x > maxX)
				maxX = verts[i].x;

			if(verts[i].y > maxY)
				maxY = verts[i].y;
		}

		x = Math::floorToInt(minX);
		y = Math::floorToInt(minY);
		width = (UINT32)Math::ceilToInt(maxX) - x;
		height = (UINT32)Math::ceilToInt(maxY) - y;
	}
}
