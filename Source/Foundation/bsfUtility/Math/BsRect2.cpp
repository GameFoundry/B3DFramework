//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsRect2.h"
#include "Math/BsVector2.h"
#include "Math/BsMatrix4.h"
#include "Math/BsMath.h"

namespace bs
{
	const Rect2 Rect2::EMPTY;

	bool Rect2::contains(const Vector2& point) const
	{
		if(point.x >= x && point.x <= (x + width))
		{
			if(point.y >= y && point.y <= (y + height))
				return true;
		}

		return false;
	}

	bool Rect2::overlaps(const Rect2& other) const
	{
		float otherRight = other.x + other.width;
		float myRight = x + width;

		float otherBottom = other.y + other.height;
		float myBottom = y + height;

		if(x < otherRight && myRight > other.x &&
			y < otherBottom && myBottom > other.y)
			return true;

		return false;
	}

	void Rect2::encapsulate(const Rect2& other)
	{
		float myRight = x + width;
		float myBottom = y + height;
		float otherRight = other.x + other.width;
		float otherBottom = other.y + other.height;

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

	void Rect2::clip(const Rect2& clipRect)
	{
		float newLeft = std::max(x, clipRect.x);
		float newTop = std::max(y, clipRect.y);

		float newRight = std::min(x + width, clipRect.x + clipRect.width);
		float newBottom = std::min(y + height, clipRect.y + clipRect.height);

		x = newLeft;
		y = newTop;
		width = newRight - newLeft;
		height = newBottom - newTop;
	}

	void Rect2::transform(const Matrix4& matrix)
	{
		Vector4 verts[4];
		verts[0] = Vector4(x, y, 0.0f, 1.0f);
		verts[1] = Vector4(x + width, y, 0.0f, 1.0f);
		verts[2] = Vector4(x, y + height, 0.0f, 1.0f);
		verts[3] = Vector4(x + width, y + height, 0.0f, 1.0f);

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

		x = minX;
		y = minY;
		width = maxX - x;
		height = maxY - y;
	}

	Vector2 Rect2::getCenter() const
	{
		return Vector2(x + width * 0.5, y + height * 0.5);
	}

	Vector2 Rect2::getHalfSize() const
	{
		return Vector2(width, height) * 0.5;
	}
}
