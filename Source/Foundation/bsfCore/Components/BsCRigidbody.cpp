//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCRigidbody.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCollider.h"
#include "Components/BsCJoint.h"
#include "Private/RTTI/BsCRigidbodyRTTI.h"
#include "Physics/BsPhysics.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

CRigidbody::CRigidbody(const HSceneObject& parent)
	: Component(parent)
{
	SetName("Rigidbody");

	mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
}

CRigidbody::CRigidbody()
	: CRigidbody(nullptr)
{ }

void CRigidbody::Move(const Vector3& position)
{
	if(GetIsKinematic())
		mImplementation->Move(position);
	else
		SO()->SetWorldPosition(position);
}

void CRigidbody::Rotate(const Quaternion& rotation)
{
	if(GetIsKinematic())
		mImplementation->Rotate(rotation);
	else
		SO()->SetWorldRotation(rotation);
}

void CRigidbody::SetMass(float mass)
{
	if(((u32)mFlags & (u32)RigidbodyFlag::AutoMass) != 0)
	{
		B3D_LOG(Warning, Physics, "Attempting to set Rigidbody mass, but it has automatic mass calculation turned on.");
		return;
	}

	mMass = mass;
	mImplementation->SetMass(mass);
}

void CRigidbody::SetIsKinematic(bool kinematic)
{
	if(mIsKinematic == kinematic)
		return;

	mIsKinematic = kinematic;
	mImplementation->SetIsKinematic(kinematic);

	ClearColliders();
	UpdateColliders();
}

bool CRigidbody::IsSleeping() const
{
	return mImplementation->IsSleeping();
}

void CRigidbody::Sleep()
{
	mImplementation->Sleep();
}

void CRigidbody::WakeUp()
{
	mImplementation->WakeUp();
}

void CRigidbody::SetSleepThreshold(float threshold)
{
	mSleepThreshold = threshold;
	mImplementation->SetSleepThreshold(threshold);
}

void CRigidbody::SetUseGravity(bool gravity)
{
	mUseGravity = gravity;
	mImplementation->SetUseGravity(gravity);
}

void CRigidbody::SetVelocity(const Vector3& velocity)
{
	mImplementation->SetVelocity(velocity);
}

Vector3 CRigidbody::GetVelocity() const
{
	return mImplementation->GetVelocity();
}

void CRigidbody::SetAngularVelocity(const Vector3& velocity)
{
	mImplementation->SetAngularVelocity(velocity);
}

Vector3 CRigidbody::GetAngularVelocity() const
{
	return mImplementation->GetAngularVelocity();
}

void CRigidbody::SetDrag(float drag)
{
	mLinearDrag = drag;
	mImplementation->SetDrag(drag);
}

void CRigidbody::SetAngularDrag(float drag)
{
	mAngularDrag = drag;
	mImplementation->SetAngularDrag(drag);
}

void CRigidbody::SetInertiaTensor(const Vector3& tensor)
{
	if(((u32)mFlags & (u32)RigidbodyFlag::AutoTensors) != 0)
	{
		B3D_LOG(Warning, Physics, "Attempting to set Rigidbody inertia tensor, but it has automatic tensor calculation turned on.");
		return;
	}

	mInertiaTensor = tensor;
	mImplementation->SetInertiaTensor(tensor);
}

Vector3 CRigidbody::GetInertiaTensor() const
{
	return mImplementation->GetInertiaTensor();
}

void CRigidbody::SetMaxAngularVelocity(float velocity)
{
	mMaxAngularVelocity = velocity;
	mImplementation->SetMaxAngularVelocity(velocity);
}

void CRigidbody::SetCenterOfMassPosition(const Vector3& position)
{
	if(((u32)mFlags & (u32)RigidbodyFlag::AutoTensors) != 0)
	{
		B3D_LOG(Warning, Physics, "Attempting to set Rigidbody center of mass, but it has automatic tensor calculation turned on.");
		return;
	}

	mCenterOfMassPosition = position;
	mImplementation->SetCenterOfMass(position, mCenterOfMassRotation);
}

Vector3 CRigidbody::GetCenterOfMassPosition() const
{
	Vector3 position;
	Quaternion rotation;
	mImplementation->GetCenterOfMass(position, rotation);

	return position;
}

void CRigidbody::SetCenterOfMassRotation(const Quaternion& rotation)
{
	if(((u32)mFlags & (u32)RigidbodyFlag::AutoTensors) != 0)
	{
		B3D_LOG(Warning, Physics, "Attempting to set Rigidbody center of mass, but it has automatic tensor calculation turned on.");
		return;
	}

	mCenterOfMassRotation = rotation;
	mImplementation->SetCenterOfMass(mCenterOfMassPosition, rotation);
}

Quaternion CRigidbody::GetCenterOfMassRotation() const
{
	Vector3 position;
	Quaternion rotation;
	mImplementation->GetCenterOfMass(position, rotation);

	return rotation;
}

void CRigidbody::SetPositionSolverCount(u32 count)
{
	mPositionSolverCount = count;
	mImplementation->SetSolverIterationCounts(mPositionSolverCount, mVelocitySolverCount);
}

void CRigidbody::SetVelocitySolverCount(u32 count)
{
	mVelocitySolverCount = count;
	mImplementation->SetSolverIterationCounts(mPositionSolverCount, mVelocitySolverCount);
}

void CRigidbody::SetCollisionReportMode(CollisionReportMode mode)
{
	if(mCollisionReportMode == mode)
		return;

	mCollisionReportMode = mode;

	for(auto& entry : mChildColliders)
		entry->UpdateCollisionReportMode();
}

void CRigidbody::SetFlags(RigidbodyFlag flags)
{
	mFlags = flags;

	mImplementation->SetFlags(flags);
	UpdateMassDistribution();
}

void CRigidbody::AddForce(const Vector3& force, ForceMode mode)
{
	mImplementation->AddForce(force, mode);
}

void CRigidbody::AddTorque(const Vector3& torque, ForceMode mode)
{
	mImplementation->AddTorque(torque, mode);
}

void CRigidbody::AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode)
{
	mImplementation->AddForceAtPoint(force, position, mode);
}

Vector3 CRigidbody::GetVelocityAtPoint(const Vector3& point) const
{
	return mImplementation->GetVelocityAtPoint(point);
}

void CRigidbody::UpdateColliders()
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
				if(!entry->IsValidParent(B3DStaticGameObjectCast<CRigidbody>(mThisHandle)))
					continue;

				entry->SetRigidbody(B3DStaticGameObjectCast<CRigidbody>(mThisHandle));
				mChildColliders.push_back(entry);
			}
		}

		u32 childCount = currentSO->GetChildCount();
		for(u32 i = 0; i < childCount; i++)
		{
			HSceneObject child = currentSO->GetChild(i);

			if(child->HasComponent<CRigidbody>())
				continue;

			todo.push(child);
		}
	}
}

void CRigidbody::ClearColliders()
{
	Vector<HCollider> children = std::move(mChildColliders);
	mChildColliders.clear();

	for(auto& collider : children)
		collider->SetRigidbody(HRigidbody());
}

void CRigidbody::AddCollider(const HCollider& collider)
{
	mChildColliders.push_back(collider);
}

void CRigidbody::RemoveCollider(const HCollider& collider)
{
	auto iterFind = std::find(mChildColliders.begin(), mChildColliders.end(), collider);

	if(iterFind != mChildColliders.end())
		mChildColliders.erase(iterFind);
}

void CRigidbody::CheckForNestedRigibody()
{
	HSceneObject currentSO = SO()->GetParent();

	while(currentSO != nullptr)
	{
		if(currentSO->HasComponent<CRigidbody>())
		{
			B3D_LOG(Warning, Physics, "Nested Rigidbodies detected. This will result in inconsistent transformations. "
									 "To parent one Rigidbody to another move its colliders to the new parent, but remove the Rigidbody "
									 "component.");
			return;
		}

		currentSO = currentSO->GetParent();
	}
}

void CRigidbody::UpdateMassDistribution()
{
	if(((u32)mFlags & (u32)RigidbodyFlag::AutoTensors) == 0)
		return;

	mImplementation->UpdateMassDistribution((((u32)mFlags & (u32)RigidbodyFlag::AutoMass)) != 0);
}

void CRigidbody::OnCreated()
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

	if(((u32)mFlags & (u32)RigidbodyFlag::AutoTensors) == 0)
	{
		mImplementation->SetCenterOfMass(mCenterOfMassPosition, mCenterOfMassRotation);
		mImplementation->SetInertiaTensor(mInertiaTensor);
		mImplementation->SetMass(mMass);
	}
	else
	{
		if(((u32)mFlags & (u32)RigidbodyFlag::AutoMass) == 0)
			mImplementation->SetMass(mMass);

		UpdateMassDistribution();
	}
}

void CRigidbody::OnDestroyed()
{
	mImplementation = nullptr;
}

void CRigidbody::OnEnabled()
{
	UpdateColliders();

#if B3D_DEBUG
	CheckForNestedRigibody();
#endif

	const SPtr<SceneInstance>& sceneInstance = SceneObject()->GetScene();
	mImplementation->AddToScene(*sceneInstance->GetPhysicsScene());
}

void CRigidbody::OnDisabled()
{
	ClearColliders();
	mImplementation->RemoveFromScene();
}

void CRigidbody::OnTransformChanged(TransformChangedFlags flags)
{
	if((flags & TCF_Parent) != 0)
	{
		ClearColliders();
		UpdateColliders();

		if(((u32)mFlags & (u32)RigidbodyFlag::AutoTensors) != 0)
			UpdateMassDistribution();

#if B3D_DEBUG
		CheckForNestedRigibody();
#endif
	}

	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	const SPtr<PhysicsScene>& physicsScene = scene->GetPhysicsScene();

	if(physicsScene->IsUpdateInProgress())
		return;

	const Transform& transform = SO()->GetTransform();
	mImplementation->SetTransform(transform.GetPosition(), transform.GetRotation());

	if(mParentJoint != nullptr)
		mParentJoint->NotifyRigidbodyMoved(B3DStaticGameObjectCast<CRigidbody>(mThisHandle));
}

RTTIType* CRigidbody::GetRttiStatic()
{
	return CRigidbodyRTTI::Instance();
}

RTTIType* CRigidbody::GetRtti() const
{
	return CRigidbody::GetRttiStatic();
}
