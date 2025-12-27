//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/B3DTransform.h"
#include "RTTI/B3DTransformRTTI.h"

using namespace b3d;

Transform Transform::kIdentity;

Transform::Transform(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
	: mPosition(position), mRotation(rotation), mScale(scale)
{}

Matrix4 Transform::GetMatrix() const
{
	return Matrix4::TRS(mPosition, mRotation, mScale);
}

Matrix4 Transform::GetInvMatrix() const
{
	return Matrix4::InverseTrs(mPosition, mRotation, mScale);
}

void Transform::MakeLocal(const Transform& parent)
{
	SetWorldPosition(mPosition, parent);
	SetWorldRotation(mRotation, parent);
	SetWorldScale(mScale, parent);
}

void Transform::MakeWorld(const Transform& parent)
{
	// Update orientation
	const Quaternion& parentOrientation = parent.GetRotation();
	mRotation = parentOrientation * mRotation;

	// Update scale
	const Vector3& parentScale = parent.GetScale();

	// Scale own position by parent scale, just combine as equivalent axes, no shearing
	mScale = parentScale * mScale;

	// Change position vector based on parent's orientation & scale
	mPosition = parentOrientation.Rotate(parentScale * mPosition);

	// Add altered position vector to parents
	mPosition += parent.GetPosition();
}

void Transform::SetWorldPosition(const Vector3& position, const Transform& parent)
{
	Vector3 invScale = parent.GetScale();
	if(invScale.X != 0) invScale.X = 1.0f / invScale.X;
	if(invScale.Y != 0) invScale.Y = 1.0f / invScale.Y;
	if(invScale.Z != 0) invScale.Z = 1.0f / invScale.Z;

	Quaternion invRotation = parent.GetRotation().Inverse();

	mPosition = invRotation.Rotate(position - parent.GetPosition()) * invScale;
}

void Transform::SetWorldRotation(const Quaternion& rotation, const Transform& parent)
{
	Quaternion invRotation = parent.GetRotation().Inverse();
	mRotation = invRotation * rotation;
}

void Transform::SetWorldScale(const Vector3& scale, const Transform& parent)
{
	Matrix4 parentMatrix = parent.GetMatrix();
	Matrix3 rotScale = parentMatrix.Get3x3();
	rotScale = rotScale.Inverse();

	Matrix3 scaleMat = Matrix3(Quaternion::kIdentity, scale);
	scaleMat = rotScale * scaleMat;

	Quaternion rotation;
	Vector3 localScale;
	scaleMat.Decomposition(rotation, localScale);

	mScale = localScale;
}

void Transform::LookAt(const Vector3& location, const Vector3& up)
{
	Vector3 forward = location - GetPosition();

	Quaternion rotation = GetRotation();
	rotation.LookRotation(forward, up);
	SetRotation(rotation);
}

void Transform::Move(const Vector3& vec)
{
	SetPosition(mPosition + vec);
}

void Transform::MoveRelative(const Vector3& vec)
{
	// Transform the axes of the relative vector by camera's local axes
	Vector3 trans = mRotation.Rotate(vec);

	SetPosition(mPosition + trans);
}

void Transform::Rotate(const Vector3& axis, const Radian& angle)
{
	Quaternion q;
	q.FromAxisAngle(axis, angle);
	Rotate(q);
}

void Transform::Rotate(const Quaternion& q)
{
	// Note the order of the mult, i.e. q comes after

	// Normalize the quat to avoid cumulative problems with precision
	Quaternion qnorm = q;
	qnorm.Normalize();
	SetRotation(qnorm * mRotation);
}

void Transform::Roll(const Radian& angle)
{
	// Rotate around local Z axis
	Vector3 zAxis = mRotation.Rotate(Vector3::kUnitZ);
	Rotate(zAxis, angle);
}

void Transform::Yaw(const Radian& angle)
{
	Vector3 yAxis = mRotation.Rotate(Vector3::kUnitY);
	Rotate(yAxis, angle);
}

void Transform::Pitch(const Radian& angle)
{
	// Rotate around local X axis
	Vector3 xAxis = mRotation.Rotate(Vector3::kUnitX);
	Rotate(xAxis, angle);
}

void Transform::SetForward(const Vector3& forwardDir)
{
	Quaternion currentRotation = GetRotation();
	currentRotation.LookRotation(forwardDir);
	SetRotation(currentRotation);
}

RTTIType* Transform::GetRttiStatic()
{
	return TransformRTTI::Instance();
}

RTTIType* Transform::GetRtti() const
{
	return Transform::GetRttiStatic();
}
