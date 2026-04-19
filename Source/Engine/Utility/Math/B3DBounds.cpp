//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DBounds.h"
#include "Math/B3DMatrix3.h"
#include "Math/B3DRay.h"
#include "Math/B3DSphere.h"

using namespace b3d;

template<typename T>
TBounds<T>::TBounds(const TAABox<T>& box, const TSphere<T>& sphere)
{
	mCenter = box.GetCenter();
	mBoxExtents = box.GetExtents();
	mSphereRadius = sphere.Radius;
}

template<typename T>
TBounds<T>::TBounds(const TAABox<T>& box)
{
	mCenter = box.GetCenter();
	mBoxExtents = box.GetExtents();
	mSphereRadius = mBoxExtents.Length();
}

template<typename T>
TBounds<T>::TBounds(const TSphere<T>& sphere)
{
	mCenter = sphere.Center;
	mBoxExtents = TVector3<T>(sphere.Radius, sphere.Radius, sphere.Radius);
	mSphereRadius = sphere.Radius;
}

template<typename T>
void TBounds<T>::Merge(const TBounds<T>& rhs)
{
	TAABox<T> box;
	box.Merge(mCenter - mBoxExtents);
	box.Merge(mCenter + mBoxExtents);
	box.Merge(rhs.mCenter - rhs.mBoxExtents);
	box.Merge(rhs.mCenter + rhs.mBoxExtents);

	mCenter = box.GetCenter();
	mBoxExtents = box.GetExtents();

	const T sphereCenterDistance = (mCenter - rhs.mCenter).Length();
	mSphereRadius = Math::Min(mBoxExtents.Length(), Math::Max(sphereCenterDistance + mSphereRadius, sphereCenterDistance + rhs.mSphereRadius));
}

template<typename T>
void TBounds<T>::Merge(const TVector3<T>& point)
{
	TAABox<T> box;
	box.Merge(mCenter - mBoxExtents);
	box.Merge(mCenter + mBoxExtents);
	box.Merge(point);

	mCenter = box.GetCenter();
	mBoxExtents = box.GetExtents();

	mSphereRadius = Math::Min(mBoxExtents.Length(), (mCenter - point).Length() + mSphereRadius);
}

template<typename T>
void TBounds<T>::TransformAffine(const TMatrix4<T>& matrix)
{
	mCenter = matrix.MultiplyAffine(mCenter);
	mBoxExtents = TVector3<T>(
		Math::Abs(matrix[0][0]) * mBoxExtents.X + Math::Abs(matrix[0][1]) * mBoxExtents.Y + Math::Abs(matrix[0][2]) * mBoxExtents.Z,
		Math::Abs(matrix[1][0]) * mBoxExtents.X + Math::Abs(matrix[1][1]) * mBoxExtents.Y + Math::Abs(matrix[1][2]) * mBoxExtents.Z,
		Math::Abs(matrix[2][0]) * mBoxExtents.X + Math::Abs(matrix[2][1]) * mBoxExtents.Y + Math::Abs(matrix[2][2]) * mBoxExtents.Z);

	T lengthSquared[3];
	for(u32 columnIndex = 0; columnIndex < 3; columnIndex++)
	{
		TVector3<T> column = matrix.GetColumn(columnIndex);
		lengthSquared[columnIndex] = column.Dot(column);
	}

	T maximumLengthSquared = std::max(lengthSquared[0], std::max(lengthSquared[1], lengthSquared[2]));
	mSphereRadius *= Math::SquareRoot(maximumLengthSquared);
}

template<typename T>
void TBounds<T>::TransformAffine(const TTransform<T>& transform)
{
	const TVector3<T>& position = transform.GetPosition();
	const TQuaternion<T>& rotation = transform.GetRotation();
	const TVector3<T>& scale = transform.GetScale();

	// Transform center: rotate scaled center, then translate
	mCenter = rotation.Rotate(scale * mCenter) + position;

	// Build rotation matrix to compute new axis-aligned extents.
	// The affine 3x3 part is R * S, so entry [i][j] = R[i][j] * S[j].
	const TMatrix3<T> rotationMatrix(rotation);

	mBoxExtents = TVector3<T>(
		Math::Abs(rotationMatrix[0][0] * scale.X) * mBoxExtents.X + Math::Abs(rotationMatrix[0][1] * scale.Y) * mBoxExtents.Y + Math::Abs(rotationMatrix[0][2] * scale.Z) * mBoxExtents.Z,
		Math::Abs(rotationMatrix[1][0] * scale.X) * mBoxExtents.X + Math::Abs(rotationMatrix[1][1] * scale.Y) * mBoxExtents.Y + Math::Abs(rotationMatrix[1][2] * scale.Z) * mBoxExtents.Z,
		Math::Abs(rotationMatrix[2][0] * scale.X) * mBoxExtents.X + Math::Abs(rotationMatrix[2][1] * scale.Y) * mBoxExtents.Y + Math::Abs(rotationMatrix[2][2] * scale.Z) * mBoxExtents.Z);

	// Rotation columns are unit length, so column length equals |scale| per axis — no sqrt needed
	mSphereRadius *= Math::Max(Math::Abs(scale.X), Math::Max(Math::Abs(scale.Y), Math::Abs(scale.Z)));
}

template struct B3D_EXPORT TBounds<float>;
template struct B3D_EXPORT TBounds<double>;
