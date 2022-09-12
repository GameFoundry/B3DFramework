//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsJoint.h"

namespace bs
{
	Rigidbody* Joint::getBody(JointBody body) const
	{
		return mInternal->getBody(body);
	}

	void Joint::SetBody(JointBody body, Rigidbody* value)
	{
		mInternal->setBody(body, value);
	}

	Vector3 Joint::GetPosition(JointBody body) const
	{
		return mInternal->getPosition(body);
	}

	Quaternion Joint::GetRotation(JointBody body) const
	{
		return mInternal->getRotation(body);
	}

	void Joint::SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation)
	{
		mInternal->setTransform(body, position, rotation);
	}

	float Joint::GetBreakForce() const
	{
		return mInternal->getBreakForce();
	}

	void Joint::SetBreakForce(float force)
	{
		mInternal->setBreakForce(force);
	}

	float Joint::GetBreakTorque() const
	{
		return mInternal->getBreakTorque();
	}

	void Joint::SetBreakTorque(float torque)
	{
		mInternal->setBreakTorque(torque);
	}

	bool Joint::GetEnableCollision() const
	{
		return mInternal->getEnableCollision();
	}

	void Joint::SetEnableCollision(bool value)
	{
		mInternal->setEnableCollision(value);
	}		
}
