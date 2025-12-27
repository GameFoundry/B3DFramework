//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DFixedJoint.h"
#include "Scene/B3DSceneObject.h"
#include "Components/B3DRigidbody.h"
#include "Physics/B3DPhysics.h"
#include "RTTI/B3DFixedJointRTTI.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

FixedJoint::FixedJoint(const HSceneObject& parent)
	: Joint(parent, mInformation)
{
	SetName("FixedJoint");
}

FixedJoint::FixedJoint()
	: FixedJoint(nullptr)
{ }

UPtr<IJointImplementation> FixedJoint::CreateImplementation()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	return scene->GetPhysicsScene()->CreateFixedJoint(*this, mInformation);
}

void FixedJoint::CalculateLocalBodyTransform(JointBody body, Vector3& position, Quaternion& rotation)
{
	position = mInformation.Bodies[(u32)body].Position;
	rotation = mInformation.Bodies[(u32)body].Rotation;

	HRigidbody rigidbody = mInformation.Bodies[(u32)body].Body;
	const Transform& transform = SO()->GetTransform();
	if(rigidbody == nullptr) // Get world space transform if no relative to any body
	{
		Quaternion worldRotation = transform.GetRotation();

		rotation = worldRotation * rotation;
		position = worldRotation.Rotate(position) + transform.GetPosition();
	}
	else
	{
		const Transform& rigidbodyTransform = rigidbody->SO()->GetTransform();

		// Find world space transform
		Quaternion worldRotation = rigidbodyTransform.GetRotation();

		rotation = worldRotation * rotation;
		position = worldRotation.Rotate(position) + rigidbodyTransform.GetPosition();

		// Get transform of the joint local to the object
		Quaternion invRotation = rotation.Inverse();

		position = invRotation.Rotate(transform.GetPosition() - position);
		rotation = invRotation * transform.GetRotation();
	}
}

IFixedJointImplementation& FixedJoint::GetImplementation() const
{
	return static_cast<IFixedJointImplementation&>(*mImplementation);
}

RTTIType* FixedJoint::GetRttiStatic()
{
	return FixedJointRTTI::Instance();
}

RTTIType* FixedJoint::GetRtti() const
{
	return FixedJoint::GetRttiStatic();
}
