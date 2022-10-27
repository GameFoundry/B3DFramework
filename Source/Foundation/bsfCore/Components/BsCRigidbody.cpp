//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCRigidbody.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCCollider.h"
#include "Components/BsCJoint.h"
#include "Private/RTTI/BsCRigidbodyRTTI.h"
#include "Physics/BsPhysics.h"

using namespace std::placeholders;

using namespace bs;

CRigidbody::CRigidbody()
{
	SetName("Rigidbody");

	mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
}

CRigidbody::CRigidbody(const HSceneObject& parent)
	: Component(parent)
{
	SetName("Rigidbody");

	mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
}

void CRigidbody::Move(const Vector3& position)
{
	if(mInternal != nullptr)
		mInternal->Move(position);

	mNotifyFlags = (TransformChangedFlags)0;
	SO()->SetWorldPosition(position);
	mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
}

void CRigidbody::Rotate(const Quaternion& rotation)
{
	if(mInternal != nullptr)
		mInternal->Rotate(rotation);

	mNotifyFlags = (TransformChangedFlags)0;
	SO()->SetWorldRotation(rotation);
	mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
}

void CRigidbody::SetMass(float mass)
{
	mMass = mass;

	if(mInternal != nullptr)
		mInternal->SetMass(mass);
}

void CRigidbody::SetIsKinematic(bool kinematic)
{
	if(mIsKinematic == kinematic)
		return;

	mIsKinematic = kinematic;

	if(mInternal != nullptr)
	{
		mInternal->SetIsKinematic(kinematic);

		ClearColliders();
		UpdateColliders();
	}
}

bool CRigidbody::IsSleeping() const
{
	if(mInternal != nullptr)
		return mInternal->IsSleeping();

	return true;
}

void CRigidbody::Sleep()
{
	if(mInternal != nullptr)
		return mInternal->Sleep();
}

void CRigidbody::WakeUp()
{
	if(mInternal != nullptr)
		return mInternal->WakeUp();
}

void CRigidbody::SetSleepThreshold(float threshold)
{
	mSleepThreshold = threshold;

	if(mInternal != nullptr)
		mInternal->SetSleepThreshold(threshold);
}

void CRigidbody::SetUseGravity(bool gravity)
{
	mUseGravity = gravity;

	if(mInternal != nullptr)
		mInternal->SetUseGravity(gravity);
}

void CRigidbody::SetVelocity(const Vector3& velocity)
{
	if(mInternal != nullptr)
		mInternal->SetVelocity(velocity);
}

Vector3 CRigidbody::GetVelocity() const
{
	if(mInternal != nullptr)
		return mInternal->GetVelocity();

	return Vector3::kZero;
}

void CRigidbody::SetAngularVelocity(const Vector3& velocity)
{
	if(mInternal != nullptr)
		mInternal->SetAngularVelocity(velocity);
}

Vector3 CRigidbody::GetAngularVelocity() const
{
	if(mInternal != nullptr)
		return mInternal->GetAngularVelocity();

	return Vector3::kZero;
}

void CRigidbody::SetDrag(float drag)
{
	mLinearDrag = drag;

	if(mInternal != nullptr)
		mInternal->SetDrag(drag);
}

void CRigidbody::SetAngularDrag(float drag)
{
	mAngularDrag = drag;

	if(mInternal != nullptr)
		mInternal->SetAngularDrag(drag);
}

void CRigidbody::SetInertiaTensor(const Vector3& tensor)
{
	mInertiaTensor = tensor;

	if(mInternal != nullptr)
		mInternal->SetInertiaTensor(tensor);
}

Vector3 CRigidbody::GetInertiaTensor() const
{
	if(mInternal != nullptr)
		return mInternal->GetInertiaTensor();

	return Vector3::kZero;
}

void CRigidbody::SetMaxAngularVelocity(float maxVelocity)
{
	mMaxAngularVelocity = maxVelocity;

	if(mInternal != nullptr)
		mInternal->SetMaxAngularVelocity(maxVelocity);
}

void CRigidbody::SetCenterOfMassPosition(const Vector3& position)
{
	mCMassPosition = position;

	if(mInternal != nullptr)
		mInternal->SetCenterOfMass(position, mCMassRotation);
}

void CRigidbody::SetCenterOfMassRotation(const Quaternion& rotation)
{
	mCMassRotation = rotation;

	if(mInternal != nullptr)
		mInternal->SetCenterOfMass(mCMassPosition, rotation);
}

Vector3 CRigidbody::GetCenterOfMassPosition() const
{
	if(mInternal != nullptr)
		return mInternal->GetCenterOfMassPosition();

	return Vector3::kZero;
}

Quaternion CRigidbody::GetCenterOfMassRotation() const
{
	if(mInternal != nullptr)
		return mInternal->GetCenterOfMassRotation();

	return Quaternion::kIdentity;
}

void CRigidbody::SetPositionSolverCount(u32 count)
{
	mPositionSolverCount = count;

	if(mInternal != nullptr)
		mInternal->SetPositionSolverCount(count);
}

void CRigidbody::SetVelocitySolverCount(u32 count)
{
	mVelocitySolverCount = count;

	if(mInternal != nullptr)
		mInternal->SetVelocitySolverCount(count);
}

void CRigidbody::SetCollisionReportMode(CollisionReportMode mode)
{
	if(mCollisionReportMode == mode)
		return;

	mCollisionReportMode = mode;

	for(auto& entry : mChildren)
		entry->UpdateCollisionReportMode();
}

void CRigidbody::SetFlags(RigidbodyFlag flags)
{
	mFlags = flags;

	if(mInternal != nullptr)
	{
		mInternal->SetFlags(flags);
		mInternal->UpdateMassDistribution();
	}
}

void CRigidbody::AddForce(const Vector3& force, ForceMode mode)
{
	if(mInternal != nullptr)
		mInternal->AddForce(force, mode);
}

void CRigidbody::AddTorque(const Vector3& torque, ForceMode mode)
{
	if(mInternal != nullptr)
		mInternal->AddTorque(torque, mode);
}

void CRigidbody::AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode)
{
	if(mInternal != nullptr)
		mInternal->AddForceAtPoint(force, position, mode);
}

Vector3 CRigidbody::GetVelocityAtPoint(const Vector3& point) const
{
	if(mInternal != nullptr)
		return mInternal->GetVelocityAtPoint(point);

	return Vector3::kZero;
}

void CRigidbody::UpdateMassDistributionInternal()
{
	if(mInternal != nullptr)
		return mInternal->UpdateMassDistribution();
}

void CRigidbody::UpdateColliders()
{
	Stack<HSceneObject> todo;
	todo.push(SO());

	while(!todo.empty())
	{
		HSceneObject currentSO = todo.top();
		todo.pop();

		if(currentSO->HasComponent<CCollider>())
		{
			Vector<HCollider> colliders = currentSO->GetComponents<CCollider>();

			for(auto& entry : colliders)
			{
				if(!entry->IsValidParent(static_object_cast<CRigidbody>(mThisHandle)))
					continue;

				Collider* collider = entry->GetInternalInternal();
				if(collider == nullptr)
					continue;

				entry->SetRigidbody(static_object_cast<CRigidbody>(mThisHandle), true);
				mChildren.push_back(entry);

				collider->SetRigidbody(mInternal.get());
				mInternal->AddCollider(collider);
			}
		}

		u32 childCount = currentSO->GetNumChildren();
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
	for(auto& collider : mChildren)
		collider->SetRigidbody(HRigidbody(), true);

	mChildren.clear();

	if(mInternal != nullptr)
		mInternal->RemoveColliders();
}

void CRigidbody::AddCollider(const HCollider& collider)
{
	if(mInternal == nullptr)
		return;

	mChildren.push_back(collider);
	mInternal->AddCollider(collider->GetInternalInternal());
}

void CRigidbody::RemoveCollider(const HCollider& collider)
{
	if(mInternal == nullptr)
		return;

	auto iterFind = std::find(mChildren.begin(), mChildren.end(), collider);

	if(iterFind != mChildren.end())
	{
		mInternal->RemoveCollider(collider->GetInternalInternal());
		mChildren.erase(iterFind);
	}
}

void CRigidbody::CheckForNestedRigibody()
{
	HSceneObject currentSO = SO()->GetParent();

	while(currentSO != nullptr)
	{
		if(currentSO->HasComponent<CRigidbody>())
		{
			BS_LOG(Warning, Physics, "Nested Rigidbodies detected. This will result in inconsistent transformations. "
									 "To parent one Rigidbody to another move its colliders to the new parent, but remove the Rigidbody "
									 "component.");
			return;
		}

		currentSO = currentSO->GetParent();
	}
}

void CRigidbody::ProcessCollisionData(const CollisionDataRaw& data, CollisionData& output)
{
	output.ContactPoints = std::move(data.ContactPoints);

	if(data.Colliders[0] != nullptr)
	{
		CCollider* other = (CCollider*)data.Colliders[0]->GetOwnerInternal(PhysicsOwnerType::Component);
		output.Collider[0] = static_object_cast<CCollider>(other->GetHandle());
	}

	if(data.Colliders[1] != nullptr)
	{
		CCollider* other = (CCollider*)data.Colliders[1]->GetOwnerInternal(PhysicsOwnerType::Component);
		output.Collider[1] = static_object_cast<CCollider>(other->GetHandle());
	}
}

void CRigidbody::DestroyInternal()
{
	ClearColliders();

	if(mInternal)
	{
		mInternal->SetOwnerInternal(PhysicsOwnerType::None, nullptr);
		mInternal = nullptr;
	}
}

void CRigidbody::TriggerOnCollisionBegin(const CollisionDataRaw& data)
{
	CollisionData hit;
	ProcessCollisionData(data, hit);

	OnCollisionBegin(hit);
}

void CRigidbody::TriggerOnCollisionStay(const CollisionDataRaw& data)
{
	CollisionData hit;
	ProcessCollisionData(data, hit);

	OnCollisionStay(hit);
}

void CRigidbody::TriggerOnCollisionEnd(const CollisionDataRaw& data)
{
	CollisionData hit;
	ProcessCollisionData(data, hit);

	OnCollisionEnd(hit);
}

void CRigidbody::OnInitialized()
{
}

void CRigidbody::OnDestroyed()
{
	DestroyInternal();
}

void CRigidbody::OnDisabled()
{
	DestroyInternal();
}

void CRigidbody::OnEnabled()
{
	mInternal = Rigidbody::Create(SO());
	mInternal->SetOwnerInternal(PhysicsOwnerType::Component, this);

	UpdateColliders();

#if BS_DEBUG_MODE
	CheckForNestedRigibody();
#endif

	mInternal->OnCollisionBegin.Connect(std::bind(&CRigidbody::TriggerOnCollisionBegin, this, _1));
	mInternal->OnCollisionStay.Connect(std::bind(&CRigidbody::TriggerOnCollisionStay, this, _1));
	mInternal->OnCollisionEnd.Connect(std::bind(&CRigidbody::TriggerOnCollisionEnd, this, _1));

	const Transform& tfrm = SO()->GetTransform();
	mInternal->SetTransform(tfrm.GetPosition(), tfrm.GetRotation());

	// Note: Merge into one call to avoid many virtual function calls
	mInternal->SetPositionSolverCount(mPositionSolverCount);
	mInternal->SetVelocitySolverCount(mVelocitySolverCount);
	mInternal->SetMaxAngularVelocity(mMaxAngularVelocity);
	mInternal->SetDrag(mLinearDrag);
	mInternal->SetAngularDrag(mAngularDrag);
	mInternal->SetSleepThreshold(mSleepThreshold);
	mInternal->SetUseGravity(mUseGravity);
	mInternal->SetIsKinematic(mIsKinematic);
	mInternal->SetFlags(mFlags);

	if(((u32)mFlags & (u32)RigidbodyFlag::AutoTensors) == 0)
	{
		mInternal->SetCenterOfMass(mCMassPosition, mCMassRotation);
		mInternal->SetInertiaTensor(mInertiaTensor);
		mInternal->SetMass(mMass);
	}
	else
	{
		if(((u32)mFlags & (u32)RigidbodyFlag::AutoMass) == 0)
			mInternal->SetMass(mMass);

		mInternal->UpdateMassDistribution();
	}
}

void CRigidbody::OnTransformChanged(TransformChangedFlags flags)
{
	if(!SO()->GetActive())
		return;

	if((flags & TCF_Parent) != 0)
	{
		ClearColliders();
		UpdateColliders();

		if(((u32)mFlags & (u32)RigidbodyFlag::AutoTensors) != 0)
			mInternal->UpdateMassDistribution();

#if BS_DEBUG_MODE
		CheckForNestedRigibody();
#endif
	}

	if(gPhysics().IsUpdateInProgressInternal())
		return;

	const Transform& tfrm = SO()->GetTransform();
	mInternal->SetTransform(tfrm.GetPosition(), tfrm.GetRotation());

	if(mParentJoint != nullptr)
		mParentJoint->NotifyRigidbodyMoved(static_object_cast<CRigidbody>(mThisHandle));
}

RTTITypeBase* CRigidbody::GetRttiStatic()
{
	return CRigidbodyRTTI::Instance();
}

RTTITypeBase* CRigidbody::GetRtti() const
{
	return CRigidbody::GetRttiStatic();
}
