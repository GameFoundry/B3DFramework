//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsJoint.h"

namespace bs
{
	Rigidbody* Joint::getBody(JointBody body) const
	{
		return mInternal->GetBody(body);
	}

	void Joint::SetBody(JointBody body, Rigidbody* value)
	{
		mInternal->SetBody(body, value);
	}

	Vector3 Joint::GetPosition(JointBody body) const
	{
		return mInternal->GetPosition(body);
	}

	Quaternion Joint::GetRotation(JointBody body) const
	{
		return mInternal->GetRotation(body);
	}

	void Joint::SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation)
	{
		mInternal->SetTransform(body, position, rotation);
	}

	float Joint::GetBreakForce() const
	{
		return mInternal->GetBreakForce();
	}

	void Joint::SetBreakForce(float force)
	{
		mInternal->SetBreakForce(force);
	}

	float Joint::GetBreakTorque() const
	{
		return mInternal->GetBreakTorque();
	}

	void Joint::SetBreakTorque(float torque)
	{
		mInternal->SetBreakTorque(torque);
	}

	bool Joint::GetEnableCollision() const
	{
		return mInternal->GetEnableCollision();
	}

	void Joint::SetEnableCollision(bool value)
	{
		mInternal->SetEnableCollision(value);
	}		
}
