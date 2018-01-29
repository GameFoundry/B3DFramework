//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Components/BsCFixedJoint.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCFixedJointRTTI.h"

namespace bs
{
	CFixedJoint::CFixedJoint() 
		:CJoint(mDesc)
	{
		setName("FixedJoint");
	}

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
		const Transform& tfrm = SO()->getTransform();
		if (rigidbody == nullptr) // Get world space transform if no relative to any body
		{
			Quaternion worldRot = tfrm.getRotation();

			rotation = worldRot*rotation;
			position = worldRot.rotate(position) + tfrm.getPosition();
		}
		else
		{
			const Transform& rigidbodyTfrm = rigidbody->SO()->getTransform();

			// Find world space transform
			Quaternion worldRot = rigidbodyTfrm.getRotation();

			rotation = worldRot * rotation;
			position = worldRot.rotate(position) + rigidbodyTfrm.getPosition();

			// Get transform of the joint local to the object
			Quaternion invRotation = rotation.inverse();

			position = invRotation.rotate(tfrm.getPosition() - position);
			rotation = invRotation * tfrm.getRotation();
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