//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DArea2.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DMath.h"
#include "Math/B3DVector2.h"

using namespace b3d;

template<>
template<>
B3D_UTILITY_EXPORT void TArea2<i32, u32>::Transform(const Matrix4& matrix)
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

template B3D_UTILITY_EXPORT void TArea2<i32>::AddUnique(const TArea2&, Vector<TArea2>&);
template B3D_UTILITY_EXPORT void TArea2<i32, u32>::AddUnique(const TArea2&, Vector<TArea2>&);
template B3D_UTILITY_EXPORT void TArea2<float>::AddUnique(const TArea2&, Vector<TArea2>&);

template B3D_UTILITY_EXPORT void TArea2<i32>::AddUnique(const TArea2&, FrameVector<TArea2>&);
template B3D_UTILITY_EXPORT void TArea2<i32, u32>::AddUnique(const TArea2&, FrameVector<TArea2>&);
template B3D_UTILITY_EXPORT void TArea2<float>::AddUnique(const TArea2&, FrameVector<TArea2>&);

template struct B3D_UTILITY_EXPORT TArea2<i32>;
template struct B3D_UTILITY_EXPORT TArea2<i32, u32>;
template struct B3D_UTILITY_EXPORT TArea2<float>;
