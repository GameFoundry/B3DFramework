//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCFixedJoint.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCFixedJointRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

CFixedJoint::CFixedJoint()
	: CJoint(mDesc)
{
	SetName("FixedJoint");
}

CFixedJoint::CFixedJoint(const HSceneObject& parent)
	: CJoint(parent, mDesc)
{
	SetName("FixedJoint");
}

SPtr<Joint> CFixedJoint::CreateInternal()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	SPtr<Joint> joint = FixedJoint::Create(*scene->GetPhysicsScene(), mDesc);

	joint->SetOwnerInternal(PhysicsOwnerType::Component, this);
	return joint;
}

void CFixedJoint::GetLocalTransform(JointBody body, Vector3& position, Quaternion& rotation)
{
	position = mPositions[(u32)body];
	rotation = mRotations[(u32)body];

	HRigidbody rigidbody = mBodies[(u32)body];
	const Transform& tfrm = SO()->GetTransform();
	if(rigidbody == nullptr) // Get world space transform if no relative to any body
	{
		Quaternion worldRot = tfrm.GetRotation();

		rotation = worldRot * rotation;
		position = worldRot.Rotate(position) + tfrm.GetPosition();
	}
	else
	{
		const Transform& rigidbodyTfrm = rigidbody->SO()->GetTransform();

		// Find world space transform
		Quaternion worldRot = rigidbodyTfrm.GetRotation();

		rotation = worldRot * rotation;
		position = worldRot.Rotate(position) + rigidbodyTfrm.GetPosition();

		// Get transform of the joint local to the object
		Quaternion invRotation = rotation.Inverse();

		position = invRotation.Rotate(tfrm.GetPosition() - position);
		rotation = invRotation * tfrm.GetRotation();
	}
}

RTTIType* CFixedJoint::GetRttiStatic()
{
	return CFixedJointRTTI::Instance();
}

RTTIType* CFixedJoint::GetRtti() const
{
	return CFixedJoint::GetRttiStatic();
}
