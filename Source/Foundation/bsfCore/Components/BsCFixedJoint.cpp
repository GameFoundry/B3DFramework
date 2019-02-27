//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCFixedJoint.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCFixedJointRTTI.h"
#include "Scene/BsSceneManager.h"

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
		const SPtr<SceneInstance>& scene = SO()->getScene();
		SPtr<Joint> joint = FixedJoint::create(*scene->getPhysicsScene(), mDesc);

		joint->_setOwner(PhysicsOwnerType::Component, this);
		return joint;
	}

	void CFixedJoint::getLocalTransform(JointBody body, Vector3& position, Quaternion& rotation)
	{
		position = mPositions[(UINT32)body];
		rotation = mRotations[(UINT32)body];

		HRigidbody rigidbody = mBodies[(UINT32)body];
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
