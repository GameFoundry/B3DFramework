//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DRigidbody.h"
#include "Scene/B3DSceneObject.h"
#include "Components/B3DCollider.h"
#include "Components/B3DJoint.h"
#include "RTTI/B3DRigidbodyRTTI.h"
#include "Physics/B3DPhysics.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

Rigidbody::Rigidbody(const HSceneObject& parent)
	: Component(parent)
{
	SetName("Rigidbody");

	mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
}

Rigidbody::Rigidbody()
	: Rigidbody(nullptr)
{ }

void Rigidbody::Move(const Vector3& position)
{
	if(GetIsKinematic())
		mImplementation->Move(position);
	else
		SO()->SetWorldPosition(position);
}

void Rigidbody::Rotate(const Quaternion& rotation)
{
	if(GetIsKinematic())
		mImplementation->Rotate(rotation);
	else
		SO()->SetWorldRotation(rotation);
}

void Rigidbody::SetMass(float mass)
{
	if(mFlags.IsSet(RigidbodyFlag::AutoMass))
	{
		B3D_LOG(Warning, LogPhysics, "Attempting to set Rigidbody mass, but it has automatic mass calculation turned on.");
		return;
	}

	mMass = mass;
	mImplementation->SetMass(mass);
}

void Rigidbody::SetIsKinematic(bool kinematic)
{
	if(mIsKinematic == kinematic)
		return;

	mIsKinematic = kinematic;
	mImplementation->SetIsKinematic(kinematic);

	ClearColliders();
	UpdateColliders();
}

bool Rigidbody::IsSleeping() const
{
	return mImplementation->IsSleeping();
}

void Rigidbody::Sleep()
{
	mImplementation->Sleep();
}

void Rigidbody::WakeUp()
{
	mImplementation->WakeUp();
}

void Rigidbody::SetSleepThreshold(float threshold)
{
	mSleepThreshold = threshold;
	mImplementation->SetSleepThreshold(threshold);
}

void Rigidbody::SetUseGravity(bool gravity)
{
	mUseGravity = gravity;
	mImplementation->SetUseGravity(gravity);
}

void Rigidbody::SetVelocity(const Vector3& velocity)
{
	mImplementation->SetVelocity(velocity);
}

Vector3 Rigidbody::GetVelocity() const
{
	return mImplementation->GetVelocity();
}

void Rigidbody::SetAngularVelocity(const Vector3& velocity)
{
	mImplementation->SetAngularVelocity(velocity);
}

Vector3 Rigidbody::GetAngularVelocity() const
{
	return mImplementation->GetAngularVelocity();
}

void Rigidbody::SetDrag(float drag)
{
	mLinearDrag = drag;
	mImplementation->SetDrag(drag);
}

void Rigidbody::SetAngularDrag(float drag)
{
	mAngularDrag = drag;
	mImplementation->SetAngularDrag(drag);
}

void Rigidbody::SetInertiaTensor(const Vector3& tensor)
{
	if(mFlags.IsSet(RigidbodyFlag::AutoTensors))
	{
		B3D_LOG(Warning, LogPhysics, "Attempting to set Rigidbody inertia tensor, but it has automatic tensor calculation turned on.");
		return;
	}

	mInertiaTensor = tensor;
	mImplementation->SetInertiaTensor(tensor);
}

Vector3 Rigidbody::GetInertiaTensor() const
{
	return mImplementation->GetInertiaTensor();
}

void Rigidbody::SetMaxAngularVelocity(float velocity)
{
	mMaxAngularVelocity = velocity;
	mImplementation->SetMaxAngularVelocity(velocity);
}

void Rigidbody::SetCenterOfMassPosition(const Vector3& position)
{
	if(mFlags.IsSet(RigidbodyFlag::AutoTensors))
	{
		B3D_LOG(Warning, LogPhysics, "Attempting to set Rigidbody center of mass, but it has automatic tensor calculation turned on.");
		return;
	}

	mCenterOfMassPosition = position;
	mImplementation->SetCenterOfMass(position, mCenterOfMassRotation);
}

Vector3 Rigidbody::GetCenterOfMassPosition() const
{
	Vector3 position;
	Quaternion rotation;
	mImplementation->GetCenterOfMass(position, rotation);

	return position;
}

void Rigidbody::SetCenterOfMassRotation(const Quaternion& rotation)
{
	if(mFlags.IsSet(RigidbodyFlag::AutoTensors))
	{
		B3D_LOG(Warning, LogPhysics, "Attempting to set Rigidbody center of mass, but it has automatic tensor calculation turned on.");
		return;
	}

	mCenterOfMassRotation = rotation;
	mImplementation->SetCenterOfMass(mCenterOfMassPosition, rotation);
}

Quaternion Rigidbody::GetCenterOfMassRotation() const
{
	Vector3 position;
	Quaternion rotation;
	mImplementation->GetCenterOfMass(position, rotation);

	return rotation;
}

void Rigidbody::SetPositionSolverCount(u32 count)
{
	mPositionSolverCount = count;
	mImplementation->SetSolverIterationCounts(mPositionSolverCount, mVelocitySolverCount);
}

void Rigidbody::SetVelocitySolverCount(u32 count)
{
	mVelocitySolverCount = count;
	mImplementation->SetSolverIterationCounts(mPositionSolverCount, mVelocitySolverCount);
}

void Rigidbody::SetCollisionReportMode(CollisionReportMode mode)
{
	if(mCollisionReportMode == mode)
		return;

	mCollisionReportMode = mode;

	for(auto& entry : mChildColliders)
		entry->UpdateCollisionReportMode();
}

void Rigidbody::SetFlags(RigidbodyFlags flags)
{
	mFlags = flags;

	mImplementation->SetFlags(flags);
	UpdateMassDistribution();
}

void Rigidbody::AddForce(const Vector3& force, ForceMode mode)
{
	mImplementation->AddForce(force, mode);
}

void Rigidbody::AddTorque(const Vector3& torque, ForceMode mode)
{
	mImplementation->AddTorque(torque, mode);
}

void Rigidbody::AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode)
{
	mImplementation->AddForceAtPoint(force, position, mode);
}

Vector3 Rigidbody::GetVelocityAtPoint(const Vector3& point) const
{
	return mImplementation->GetVelocityAtPoint(point);
}

void Rigidbody::UpdateColliders()
{
	// Note: Instead of clearing all, then re-registering below, detect changes between arrays and add/remove only changed
	ClearColliders();

	Stack<HSceneObject> todo;
	todo.push(SO());

	while(!todo.empty())
	{
		HSceneObject currentSO = todo.top();
		todo.pop();

		if(currentSO->HasComponent<Collider>())
		{
			Vector<HCollider> colliders = currentSO->GetComponents<Collider>();

			for(auto& entry : colliders)
			{
				if(!entry->IsValidParent(B3DStaticGameObjectCast<Rigidbody>(mThisHandle)))
					continue;

				entry->SetRigidbody(B3DStaticGameObjectCast<Rigidbody>(mThisHandle));
				mChildColliders.push_back(entry);
			}
		}

		u32 childCount = currentSO->GetChildCount();
		for(u32 childIndex = 0; childIndex < childCount; childIndex++)
		{
			HSceneObject child = currentSO->GetChild(childIndex);

			if(child->HasComponent<Rigidbody>())
				continue;

			todo.push(child);
		}
	}
}

void Rigidbody::ClearColliders()
{
	Vector<HCollider> children = std::move(mChildColliders);
	mChildColliders.clear();

	for(auto& collider : children)
		collider->SetRigidbody(HRigidbody());
}

void Rigidbody::AddCollider(const HCollider& collider)
{
	mChildColliders.push_back(collider);
}

void Rigidbody::RemoveCollider(const HCollider& collider)
{
	auto found = std::find(mChildColliders.begin(), mChildColliders.end(), collider);

	if(found != mChildColliders.end())
		mChildColliders.erase(found);
}

void Rigidbody::CheckForNestedRigibody()
{
	HSceneObject currentSO = SO()->GetParent();

	while(currentSO != nullptr)
	{
		if(currentSO->HasComponent<Rigidbody>())
		{
			B3D_LOG(Warning, LogPhysics, "Nested Rigidbodies detected. This will result in inconsistent transformations. "
									 "To parent one Rigidbody to another move its colliders to the new parent, but remove the Rigidbody "
									 "component.");
			return;
		}

		currentSO = currentSO->GetParent();
	}
}

void Rigidbody::UpdateMassDistribution()
{
	if(!mFlags.IsSet(RigidbodyFlag::AutoTensors))
		return;

	mImplementation->UpdateMassDistribution(mFlags.IsSet(RigidbodyFlag::AutoMass));
}

void Rigidbody::OnCreated()
{
	B3D_ASSERT(mImplementation == nullptr);

	const Transform& transform = SO()->GetTransform();

	mImplementation = GetPhysics().CreateRigidbodyImplementation(*this);
	mImplementation->SetTransform(transform.GetPosition(), transform.GetRotation());
	mImplementation->SetSolverIterationCounts(mPositionSolverCount, mVelocitySolverCount);
	mImplementation->SetMaxAngularVelocity(mMaxAngularVelocity);
	mImplementation->SetDrag(mLinearDrag);
	mImplementation->SetAngularDrag(mAngularDrag);
	mImplementation->SetSleepThreshold(mSleepThreshold);
	mImplementation->SetUseGravity(mUseGravity);
	mImplementation->SetIsKinematic(mIsKinematic);
	mImplementation->SetFlags(mFlags);

	if(!mFlags.IsSet(RigidbodyFlag::AutoTensors))
	{
		mImplementation->SetCenterOfMass(mCenterOfMassPosition, mCenterOfMassRotation);
		mImplementation->SetInertiaTensor(mInertiaTensor);
		mImplementation->SetMass(mMass);
	}
	else
	{
		if(!mFlags.IsSet(RigidbodyFlag::AutoMass))
			mImplementation->SetMass(mMass);

		UpdateMassDistribution();
	}
}

void Rigidbody::OnDestroyed()
{
	mImplementation = nullptr;
}

void Rigidbody::OnEnabled()
{
	UpdateColliders();

#if B3D_DEBUG
	CheckForNestedRigibody();
#endif

	const TShared<SceneInstance>& sceneInstance = SceneObject()->GetScene();
	mImplementation->AddToScene(*sceneInstance->GetPhysicsScene());
}

void Rigidbody::OnDisabled()
{
	ClearColliders();
	mImplementation->RemoveFromScene();
}

void Rigidbody::OnTransformChanged(TransformChangedFlags flags)
{
	if((flags & TCF_Parent) != 0)
	{
		ClearColliders();
		UpdateColliders();

		if(mFlags.IsSet(RigidbodyFlag::AutoTensors))
			UpdateMassDistribution();

#if B3D_DEBUG
		CheckForNestedRigibody();
#endif
	}

	const TShared<SceneInstance>& scene = SceneObject()->GetScene();
	const TShared<PhysicsScene>& physicsScene = scene->GetPhysicsScene();

	if(physicsScene->IsUpdateInProgress())
		return;

	const Transform& transform = SO()->GetTransform();
	mImplementation->SetTransform(transform.GetPosition(), transform.GetRotation());

	if(mParentJoint != nullptr)
		mParentJoint->NotifyRigidbodyMoved(B3DStaticGameObjectCast<Rigidbody>(mThisHandle));
}

RTTIType* Rigidbody::GetRttiStatic()
{
	return RigidbodyRTTI::Instance();
}

RTTIType* Rigidbody::GetRtti() const
{
	return GetRttiStatic();
}
