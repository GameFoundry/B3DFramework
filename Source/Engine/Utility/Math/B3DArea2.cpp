//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DArea2.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DMath.h"
#include "Math/B3DVector2.h"

using namespace b3d;

template<>
template<>
B3D_EXPORT void TArea2<i32, u32>::Transform(const Matrix4& matrix)
{
	Vector4 verts[4];
	verts[0] = Vector4((float)X, (float)Y, 0.0f, 1.0f);
	verts[1] = Vector4((float)X + Width, (float)Y, 0.0f, 1.0f);
	verts[2] = Vector4((float)X, (float)Y + Height, 0.0f, 1.0f);
	verts[3] = Vector4((float)X + Width, (float)Y + Height, 0.0f, 1.0f);

	for(u32 vertexIndex = 0; vertexIndex < 4; vertexIndex++)
		verts[vertexIndex] = matrix.Multiply(verts[vertexIndex]);

	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::min();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::min();

	for(u32 vertexIndex = 0; vertexIndex < 4; vertexIndex++)
	{
		if(verts[vertexIndex].X < minX)
			minX = verts[vertexIndex].X;

		if(verts[vertexIndex].Y < minY)
			minY = verts[vertexIndex].Y;

		if(verts[vertexIndex].X > maxX)
			maxX = verts[vertexIndex].X;

		if(verts[vertexIndex].Y > maxY)
			maxY = verts[vertexIndex].Y;
	}

	X = Math::FloorToInt(minX);
	Y = Math::FloorToInt(minY);
	Width = (u32)Math::CeilToInt(maxX) - X;
	Height = (u32)Math::CeilToInt(maxY) - Y;
}

namespace b3d
{
	template B3D_EXPORT void TArea2<i32>::AddUnique(const TArea2&, Vector<TArea2>&);
	template B3D_EXPORT void TArea2<i32, u32>::AddUnique(const TArea2&, Vector<TArea2>&);
	template B3D_EXPORT void TArea2<float>::AddUnique(const TArea2&, Vector<TArea2>&);

	template B3D_EXPORT void TArea2<i32>::AddUnique(const TArea2&, FrameVector<TArea2>&);
	template B3D_EXPORT void TArea2<i32, u32>::AddUnique(const TArea2&, FrameVector<TArea2>&);
	template B3D_EXPORT void TArea2<float>::AddUnique(const TArea2&, FrameVector<TArea2>&);

	template struct B3D_EXPORT TArea2<i32>;
	template struct B3D_EXPORT TArea2<i32, u32>;
	template struct B3D_EXPORT TArea2<float>;
} // namespace b3d
