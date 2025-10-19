//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DJoint.h"
#include "Components/B3DRigidbody.h"
#include "Scene/B3DSceneObject.h"
#include "Physics/B3DPhysics.h"
#include "RTTI/B3DJointRTTI.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

Joint::Joint(const HSceneObject& parent, JointCreateInformation& createInformation)
	: Component(parent), mInformation(createInformation)
{
	SetName("Joint");

	mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
}

Joint::Joint(JointCreateInformation& createInformation)
	: Joint(nullptr, createInformation)
{ }

void Joint::SetBody(JointBody body, const HRigidbody& value)
{
	if(mInformation.Bodies[(int)body].Body == value)
		return;

	if(mInformation.Bodies[(int)body].Body != nullptr)
		mInformation.Bodies[(int)body].Body->SetParentJoint(nullptr);

	mInformation.Bodies[(int)body].Body = value;

	if(value != nullptr)
		mInformation.Bodies[(int)body].Body->SetParentJoint(B3DStaticGameObjectCast<Joint>(mThisHandle));

	// If joint already exists, destroy it if we removed all bodies, otherwise update its transform
	if(mImplementation != nullptr)
	{
		if(!mInformation.Bodies[0].Body.IsValid()&& !mInformation.Bodies[1].Body.IsValid())
			DestroyImplementation();
		else
		{
			mImplementation->SetBody(body, value.IsValid() ? value.Get() : nullptr);
			UpdateRelativeBodyTransforms(body);
		}
	}
	else // If joint doesn't exist, check if we can create it
	{
		// Must be an active component and at least one of the bodies must be non-null
		if(GetEnabled() && (mInformation.Bodies[0].Body.IsValid() || mInformation.Bodies[1].Body.IsValid()))
			mImplementation = CreateImplementation();
	}
}

void Joint::SetRelativeBodyTransform(JointBody body, const Vector3& position, const Quaternion& rotation)
{
	if(mInformation.Bodies[(int)body].Position == position && mInformation.Bodies[(int)body].Rotation == rotation)
		return;

	mInformation.Bodies[(int)body].Position = position;
	mInformation.Bodies[(int)body].Rotation = rotation;

	if(mImplementation != nullptr)
		UpdateRelativeBodyTransforms(body);
}

void Joint::SetBreakForce(float force)
{
	if(mInformation.BreakForce == force)
		return;

	mInformation.BreakForce = force;

	if(mImplementation != nullptr)
		mImplementation->SetBreakForce(force);
}

void Joint::SetBreakTorque(float torque)
{
	if(mInformation.BreakTorque == torque)
		return;

	mInformation.BreakTorque = torque;

	if(mImplementation != nullptr)
		mImplementation->SetBreakTorque(torque);
}

void Joint::SetEnableCollision(bool value)
{
	if(mInformation.EnableCollision == value)
		return;

	mInformation.EnableCollision = value;

	if(mImplementation != nullptr)
		mImplementation->SetEnableCollision(value);
}

void Joint::OnDestroyed()
{
	if(mInformation.Bodies[0].Body.IsValid())
		mInformation.Bodies[0].Body->SetParentJoint(HJoint());

	if(mInformation.Bodies[1].Body.IsValid())
		mInformation.Bodies[1].Body->SetParentJoint(HJoint());

	DestroyImplementation();
}

void Joint::OnDisabled()
{
	DestroyImplementation();
}

void Joint::OnEnabled()
{
	if(mInformation.Bodies[0].Body.IsValid() || mInformation.Bodies[1].Body.IsValid())
		mImplementation = CreateImplementation();
}

void Joint::OnTransformChanged(TransformChangedFlags flags)
{
	if(mImplementation == nullptr)
		return;

	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	const SPtr<PhysicsScene>& physicsScene = scene->GetPhysicsScene();

	// We're ignoring this during physics update because it would cause problems if the joint itself was moved by physics
	// Note: This isn't particularily correct because if the joint is being moved by physics but the rigidbodies
	// themselves are not parented to the joint, the transform will need updating. However I'm leaving it up to the
	// user to ensure rigidbodies are always parented to the joint in such a case (It's an unlikely situation that
	// I can't think of an use for - joint transform will almost always be set as an initialization step and not a
	// physics response).
	if(physicsScene->IsUpdateInProgress())
		return;

	UpdateRelativeBodyTransforms(JointBody::Target);
	UpdateRelativeBodyTransforms(JointBody::Anchor);
}

void Joint::DestroyImplementation()
{
	mImplementation = nullptr;
}

void Joint::NotifyRigidbodyMoved(const HRigidbody& body)
{
	if(mImplementation == nullptr)
		return;

	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	const SPtr<PhysicsScene>& physicsScene = scene->GetPhysicsScene();

	// If physics update is in progress do nothing, as its the joint itself that's probably moving the body
	if(physicsScene->IsUpdateInProgress())
		return;

	if(mInformation.Bodies[0].Body == body)
		UpdateRelativeBodyTransforms(JointBody::Target);
	else if(mInformation.Bodies[1].Body == body)
		UpdateRelativeBodyTransforms(JointBody::Anchor);
	else
		B3D_ASSERT(false); // Not allowed to happen
}

void Joint::UpdateRelativeBodyTransforms(JointBody body)
{
	Vector3 localPos;
	Quaternion localRot;
	CalculateLocalBodyTransform(body, localPos, localRot);

	mImplementation->SetTransform(body, localPos, localRot);
}

void Joint::CalculateLocalBodyTransform(JointBody body, Vector3& position, Quaternion& rotation)
{
	position = mInformation.Bodies[(u32)body].Position;
	rotation = mInformation.Bodies[(u32)body].Rotation;

	HRigidbody rigidbody = mInformation.Bodies[(u32)body].Body;
	if(rigidbody == nullptr) // Get world space transform if no relative to any body
	{
		const Transform& transform = SO()->GetTransform();
		Quaternion worldRotation = transform.GetRotation();

		rotation = worldRotation * rotation;
		position = worldRotation.Rotate(position) + transform.GetPosition();
	}
	else
		position = rotation.Rotate(position);
}

RTTIType* Joint::GetRttiStatic()
{
	return JointRTTI::Instance();
}

RTTIType* Joint::GetRtti() const
{
	return Joint::GetRttiStatic();
}
