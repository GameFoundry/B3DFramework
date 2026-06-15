//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DTransform.h"
#include "Math/B3DMatrix3.h"

using namespace b3d;

template<typename T>
TTransform<T>::TTransform(const TVector3<T>& position, const TQuaternion<T>& rotation, const TVector3<T>& scale)
	: mPosition(position), mRotation(rotation), mScale(scale)
{}

template<typename T>
TMatrix4<T> TTransform<T>::GetMatrix() const
{
	return TMatrix4<T>::TRS(mPosition, mRotation, mScale);
}

template<typename T>
TMatrix4<T> TTransform<T>::GetInvMatrix() const
{
	return TMatrix4<T>::InverseTrs(mPosition, mRotation, mScale);
}

template<typename T>
void TTransform<T>::MakeLocal(const TTransform& parent)
{
	SetWorldPosition(mPosition, parent);
	SetWorldRotation(mRotation, parent);
	SetWorldScale(mScale, parent);
}

template<typename T>
void TTransform<T>::MakeWorld(const TTransform& parent)
{
	// Update orientation
	const TQuaternion<T>& parentOrientation = parent.GetRotation();
	mRotation = parentOrientation * mRotation;

	// Update scale
	const TVector3<T>& parentScale = parent.GetScale();

	// Scale own position by parent scale, just combine as equivalent axes, no shearing
	mScale = parentScale * mScale;

	// Change position vector based on parent's orientation & scale
	mPosition = parentOrientation.Rotate(parentScale * mPosition);

	// Add altered position vector to parents
	mPosition += parent.GetPosition();
}

template<typename T>
void TTransform<T>::SetWorldPosition(const TVector3<T>& position, const TTransform& parent)
{
	TVector3<T> invScale = parent.GetScale();
	if(invScale.X != (T)0.0) invScale.X = (T)1.0 / invScale.X;
	if(invScale.Y != (T)0.0) invScale.Y = (T)1.0 / invScale.Y;
	if(invScale.Z != (T)0.0) invScale.Z = (T)1.0 / invScale.Z;

	TQuaternion<T> invRotation = parent.GetRotation().Inverse();

	mPosition = invRotation.Rotate(position - parent.GetPosition()) * invScale;
}

template<typename T>
void TTransform<T>::SetWorldRotation(const TQuaternion<T>& rotation, const TTransform& parent)
{
	TQuaternion<T> invRotation = parent.GetRotation().Inverse();
	mRotation = invRotation * rotation;
}

template<typename T>
void TTransform<T>::SetWorldScale(const TVector3<T>& scale, const TTransform& parent)
{
	TMatrix4<T> parentMatrix = parent.GetMatrix();
	TMatrix3<T> rotScale = parentMatrix.Get3x3();
	rotScale = rotScale.Inverse();

	TMatrix3<T> scaleMat = TMatrix3<T>(TQuaternion<T>::kIdentity, scale);
	scaleMat = rotScale * scaleMat;

	TQuaternion<T> rotation;
	TVector3<T> localScale;
	scaleMat.Decomposition(rotation, localScale);

	mScale = localScale;
}

template<typename T>
void TTransform<T>::LookAt(const TVector3<T>& location, const TVector3<T>& up)
{
	TVector3<T> forward = location - GetPosition();

	TQuaternion<T> rotation = GetRotation();
	rotation.LookRotation(forward, up);
	SetRotation(rotation);
}

template<typename T>
void TTransform<T>::Move(const TVector3<T>& vec)
{
	SetPosition(mPosition + vec);
}

template<typename T>
void TTransform<T>::MoveRelative(const TVector3<T>& vec)
{
	// Transform the axes of the relative vector by camera's local axes
	TVector3<T> trans = mRotation.Rotate(vec);

	SetPosition(mPosition + trans);
}

template<typename T>
void TTransform<T>::Rotate(const TVector3<T>& axis, const TRadian<T>& angle)
{
	TQuaternion<T> q;
	q.FromAxisAngle(axis, angle);
	Rotate(q);
}

template<typename T>
void TTransform<T>::Rotate(const TQuaternion<T>& q)
{
	// Note the order of the mult, i.e. q comes after

	// Normalize the quat to avoid cumulative problems with precision
	TQuaternion<T> qnorm = q;
	qnorm.Normalize();
	SetRotation(qnorm * mRotation);
}

template<typename T>
void TTransform<T>::Roll(const TRadian<T>& angle)
{
	// Rotate around local Z axis
	TVector3<T> zAxis = mRotation.Rotate(TVector3<T>::kUnitZ);
	Rotate(zAxis, angle);
}

template<typename T>
void TTransform<T>::Yaw(const TRadian<T>& angle)
{
	TVector3<T> yAxis = mRotation.Rotate(TVector3<T>::kUnitY);
	Rotate(yAxis, angle);
}

template<typename T>
void TTransform<T>::Pitch(const TRadian<T>& angle)
{
	// Rotate around local X axis
	TVector3<T> xAxis = mRotation.Rotate(TVector3<T>::kUnitX);
	Rotate(xAxis, angle);
}

template<typename T>
void TTransform<T>::SetForward(const TVector3<T>& forwardDir)
{
	TQuaternion<T> currentRotation = GetRotation();
	currentRotation.LookRotation(forwardDir);
	SetRotation(currentRotation);
}

namespace b3d
{
	template class B3D_EXPORT TTransform<float>;
	template class B3D_EXPORT TTransform<double>;
} // namespace b3d
