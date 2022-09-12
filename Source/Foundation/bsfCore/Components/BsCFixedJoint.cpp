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

	SPtr<Joint> CFixedJoint::CreateInternal()
	{
		const SPtr<SceneInstance>& scene = SO()->GetScene();
		SPtr<Joint> joint = FixedJoint::create(*scene->GetPhysicsScene(), mDesc);

		joint->_setOwner(PhysicsOwnerType::Component, this);
		return joint;
	}

	void CFixedJoint::GetLocalTransform(JointBody body, Vector3& position, Quaternion& rotation)
	{
		position = mPositions[(UINT32)body];
		rotation = mRotations[(UINT32)body];

		HRigidbody rigidbody = mBodies[(UINT32)body];
		const Transform& tfrm = SO()->GetTransform();
		if (rigidbody == nullptr) // Get world space transform if no relative to any body
		{
			Quaternion worldRot = tfrm.GetRotation();

			rotation = worldRot*rotation;
			position = worldRot.Rotate(position) + tfrm.getPosition();
		}
		else
		{
			const Transform& rigidbodyTfrm = rigidbody->SO()->GetTransform();

			// Find world space transform
			Quaternion worldRot = rigidbodyTfrm.GetRotation();

			rotation = worldRot * rotation;
			position = worldRot.Rotate(position) + rigidbodyTfrm.getPosition();

			// Get transform of the joint local to the object
			Quaternion invRotation = rotation.Inverse();

			position = invRotation.Rotate(tfrm.getPosition() - position);
			rotation = invRotation * tfrm.GetRotation();
		}
	}

	RTTITypeBase* CFixedJoint::getRTTIStatic()
	{
		return CFixedJointRTTI::Instance();
	}

	RTTITypeBase* CFixedJoint::getRTTI() const
	{
		return CFixedJoint::GetRTTIStatic();
	}
}
