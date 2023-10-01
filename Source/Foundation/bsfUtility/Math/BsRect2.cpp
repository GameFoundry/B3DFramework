//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsRect2.h"
#include "Math/BsVector2.h"
#include "Math/BsMatrix4.h"
#include "Math/BsMath.h"

using namespace bs;

const Rect2 Rect2::kEmpty;

bool Rect2::Contains(const Vector2& point) const
{
	if(point.X >= X && point.X <= (X + Width))
	{
		if(point.Y >= Y && point.Y <= (Y + Height))
			return true;
	}

	return false;
}

bool Rect2::Contains(const Rect2& other) const
{
	const float right = X + Width;
	const float bottom = Y + Height;

	const float otherRight = other.X + other.Width;
	const float otherBottom = other.Y + other.Height;

	return other.X >= X && otherRight <= right &&
		other.Y >= Y && otherBottom <= bottom;
}

bool Rect2::Overlaps(const Rect2& other) const
{
	float otherRight = other.X + other.Width;
	float myRight = X + Width;

	float otherBottom = other.Y + other.Height;
	float myBottom = Y + Height;

	if(X < otherRight && myRight > other.X &&
	   Y < otherBottom && myBottom > other.Y)
		return true;

	return false;
}

void Rect2::Encapsulate(const Rect2& other)
{
	float myRight = X + Width;
	float myBottom = Y + Height;
	float otherRight = other.X + other.Width;
	float otherBottom = other.Y + other.Height;

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

void Rect2::Inset(float amount)
{
	Width = Math::Max(0.0f, Width - amount * 2.0f);
	Height = Math::Max(0.0f, Height - amount * 2.0f);

	X += amount;
	Y += amount;
}

void Rect2::Clip(const Rect2& clipRect)
{
	float newLeft = std::max(X, clipRect.X);
	float newTop = std::max(Y, clipRect.Y);

	float newRight = std::min(X + Width, clipRect.X + clipRect.Width);
	float newBottom = std::min(Y + Height, clipRect.Y + clipRect.Height);

	X = newLeft;
	Y = newTop;
	Width = newRight - newLeft;
	Height = newBottom - newTop;
}

void Rect2::Transform(const Matrix4& matrix)
{
	Vector4 verts[4];
	verts[0] = Vector4(X, Y, 0.0f, 1.0f);
	verts[1] = Vector4(X + Width, Y, 0.0f, 1.0f);
	verts[2] = Vector4(X, Y + Height, 0.0f, 1.0f);
	verts[3] = Vector4(X + Width, Y + Height, 0.0f, 1.0f);

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

	X = minX;
	Y = minY;
	Width = maxX - X;
	Height = maxY - Y;
}

Vector2 Rect2::GetCenter() const
{
	return Vector2(X + Width * 0.5f, Y + Height * 0.5f);
}

Vector2 Rect2::GetHalfSize() const
{
	return Vector2(Width, Height) * 0.5f;
}
