//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCFixedJoint.h"
#include "BsSceneObject.h"
#include "BsCRigidbody.h"
#include "BsCFixedJointRTTI.h"

namespace bs
{
	CFixedJoint::CFixedJoint() 
		:CJoint(mDesc)
	{ }

	CFixedJoint::CFixedJoint(const HSceneObject& parent)
		: CJoint(parent, mDesc)
	{
		setName("FixedJoint");
	}

	SPtr<Joint> CFixedJoint::createInternal()
	{
		SPtr<Joint> joint = FixedJoint::create(mDesc);

		joint->_setOwner(PhysicsOwnerType::Component, this);
		return joint;
	}

	void CFixedJoint::getLocalTransform(JointBody body, Vector3& position, Quaternion& rotation)
	{
		position = mPositions[(int)body];
		rotation = mRotations[(int)body];

		HRigidbody rigidbody = mBodies[(int)body];
		if (rigidbody == nullptr) // Get world space transform if no relative to any body
		{
			Quaternion worldRot = SO()->getWorldRotation();

			rotation = worldRot*rotation;
			position = worldRot.rotate(position) + SO()->getWorldPosition();
		}
		else
		{
			// Find world space transform
			Quaternion worldRot = rigidbody->SO()->getWorldRotation();

			rotation = worldRot * rotation;
			position = worldRot.rotate(position) + rigidbody->SO()->getWorldPosition();

			// Get transform of the joint local to the object
			Quaternion invRotation = rotation.inverse();

			position = invRotation.rotate(SO()->getWorldPosition() - position);
			rotation = invRotation * SO()->getWorldRotation();
		}
	}

	RTTITypeBase* CFixedJoint::getRTTIStatic()
	{
		return CFixedJointRTTI::instance();
	}

	RTTITypeBase* CFixedJoint::getRTTI() const
	{
		return CFixedJoint::getRTTIStatic();
	}
}