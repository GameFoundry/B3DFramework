//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Scene/BsTransform.h"
#include "Private/RTTI/BsTransformRTTI.h"

namespace bs
{
	Transform::Transform()
		: mPosition(Vector3::ZERO), mRotation(Quaternion::IDENTITY), mScale(Vector3::ONE)
	{ }

	Matrix4 Transform::getMatrix() const
	{
		return Matrix4::TRS(mPosition, mRotation, mScale);
	}

	Matrix4 Transform::getInvMatrix() const
	{
		return Matrix4::inverseTRS(mPosition, mRotation, mScale);
	}

	void Transform::makeLocal(const Transform& parent)
	{
		setWorldPosition(mPosition, parent);
		setWorldRotation(mRotation, parent);
		setWorldScale(mScale, parent);
	}

	void Transform::makeWorld(const Transform& parent)
	{
		// Update orientation
		const Quaternion& parentOrientation = parent.getRotation();
		mRotation = parentOrientation * mRotation;

		// Update scale
		const Vector3& parentScale = parent.getScale();

		// Scale own position by parent scale, just combine as equivalent axes, no shearing
		mScale = parentScale * mScale;

		// Change position vector based on parent's orientation & scale
		mPosition = parentOrientation.rotate(parentScale * mPosition);

		// Add altered position vector to parents
		mPosition += parent.getPosition();
	}

	void Transform::setWorldPosition(const Vector3& position, const Transform& parent)
	{
		Vector3 invScale = parent.getScale();
		if (invScale.x != 0) invScale.x = 1.0f / invScale.x;
		if (invScale.y != 0) invScale.y = 1.0f / invScale.y;
		if (invScale.z != 0) invScale.z = 1.0f / invScale.z;

		Quaternion invRotation = parent.getRotation().inverse();

		mPosition = invRotation.rotate(position - parent.getPosition()) *  invScale;
	}

	void Transform::setWorldRotation(const Quaternion& rotation, const Transform& parent)
	{
		Quaternion invRotation = parent.getRotation().inverse();
		mRotation = invRotation * rotation;
	}

	void Transform::setWorldScale(const Vector3& scale, const Transform& parent)
	{
		Matrix3 rotScale;
		Matrix4 parentMatrix = parent.getMatrix();
		parentMatrix.extract3x3Matrix(rotScale);
		rotScale.inverse();

		Matrix3 scaleMat = Matrix3(Quaternion::IDENTITY, scale);
		scaleMat = rotScale * scaleMat;

		Quaternion rotation;
		Vector3 localScale;
		scaleMat.decomposition(rotation, localScale);

		mScale = localScale;
	}

	void Transform::lookAt(const Vector3& location, const Vector3& up)
	{
		Vector3 forward = location - getPosition();
		
		Quaternion rotation = getRotation();
		rotation.lookRotation(forward, up);
		setRotation(rotation);
	}

	void Transform::move(const Vector3& vec)
	{
		setPosition(mPosition + vec);
	}

	void Transform::moveRelative(const Vector3& vec)
	{
		// Transform the axes of the relative vector by camera's local axes
		Vector3 trans = mRotation.rotate(vec);

		setPosition(mPosition + trans);
	}

	void Transform::rotate(const Vector3& axis, const Radian& angle)
	{
		Quaternion q;
		q.fromAxisAngle(axis, angle);
		rotate(q);
	}

	void Transform::rotate(const Quaternion& q)
	{
		// Note the order of the mult, i.e. q comes after

		// Normalize the quat to avoid cumulative problems with precision
		Quaternion qnorm = q;
		qnorm.normalize();
		setRotation(qnorm * mRotation);
	}

	void Transform::roll(const Radian& angle)
	{
		// Rotate around local Z axis
		Vector3 zAxis = mRotation.rotate(Vector3::UNIT_Z);
		rotate(zAxis, angle);
	}

	void Transform::yaw(const Radian& angle)
	{
		Vector3 yAxis = mRotation.rotate(Vector3::UNIT_Y);
		rotate(yAxis, angle);
	}

	void Transform::pitch(const Radian& angle)
	{
		// Rotate around local X axis
		Vector3 xAxis = mRotation.rotate(Vector3::UNIT_X);
		rotate(xAxis, angle);
	}

	void Transform::setForward(const Vector3& forwardDir)
	{
		Quaternion currentRotation = getRotation();
		currentRotation.lookRotation(forwardDir);
		setRotation(currentRotation);
	}

	RTTITypeBase* Transform::getRTTIStatic()
	{
		return TransformRTTI::instance();
	}

	RTTITypeBase* Transform::getRTTI() const
	{
		return Transform::getRTTIStatic();
	}
}
