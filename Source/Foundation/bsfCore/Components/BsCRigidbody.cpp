//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCRigidbody.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCCollider.h"
#include "Components/BsCJoint.h"
#include "Private/RTTI/BsCRigidbodyRTTI.h"
#include "Physics/BsPhysics.h"

using namespace std::placeholders;

namespace bs
{
	CRigidbody::CRigidbody()
	{
		setName("Rigidbody");

		mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
	}

	CRigidbody::CRigidbody(const HSceneObject& parent)
		: Component(parent)
	{
		setName("Rigidbody");

		mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
	}

	void CRigidbody::Move(const Vector3& position)
	{
		if (mInternal != nullptr)
			mInternal->Move(position);

		mNotifyFlags = (TransformChangedFlags)0;
		SO()->SetWorldPosition(position);
		mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
	}

	void CRigidbody::Rotate(const Quaternion& rotation)
	{
		if (mInternal != nullptr)
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
		if (mIsKinematic == kinematic)
			return;

		mIsKinematic = kinematic;
		
		if (mInternal != nullptr)
		{
			mInternal->SetIsKinematic(kinematic);

			clearColliders();
			updateColliders();
		}
	}

	bool CRigidbody::IsSleeping() const
	{
		if (mInternal != nullptr)
			return mInternal->IsSleeping();

		return true;
	}

	void CRigidbody::Sleep()
	{
		if (mInternal != nullptr)
			return mInternal->Sleep();
	}

	void CRigidbody::WakeUp()
	{
		if (mInternal != nullptr)
			return mInternal->WakeUp();
	}

	void CRigidbody::SetSleepThreshold(float threshold)
	{
		mSleepThreshold = threshold;

		if (mInternal != nullptr)
			mInternal->SetSleepThreshold(threshold);
	}

	void CRigidbody::SetUseGravity(bool gravity)
	{
		mUseGravity = gravity;

		if (mInternal != nullptr)
			mInternal->SetUseGravity(gravity);
	}

	void CRigidbody::SetVelocity(const Vector3& velocity)
	{
		if (mInternal != nullptr)
			mInternal->SetVelocity(velocity);
	}

	Vector3 CRigidbody::GetVelocity() const
	{
		if (mInternal != nullptr)
			return mInternal->GetVelocity();

		return Vector3::ZERO;
	}

	void CRigidbody::SetAngularVelocity(const Vector3& velocity)
	{
		if (mInternal != nullptr)
			mInternal->SetAngularVelocity(velocity);
	}

	Vector3 CRigidbody::GetAngularVelocity() const
	{
		if (mInternal != nullptr)
			return mInternal->GetAngularVelocity();

		return Vector3::ZERO;
	}

	void CRigidbody::SetDrag(float drag)
	{
		mLinearDrag = drag;

		if (mInternal != nullptr)
			mInternal->SetDrag(drag);
	}

	void CRigidbody::SetAngularDrag(float drag)
	{
		mAngularDrag = drag;

		if (mInternal != nullptr)
			mInternal->SetAngularDrag(drag);
	}

	void CRigidbody::SetInertiaTensor(const Vector3& tensor)
	{
		mInertiaTensor = tensor;

		if (mInternal != nullptr)
			mInternal->SetInertiaTensor(tensor);
	}

	Vector3 CRigidbody::GetInertiaTensor() const
	{
		if (mInternal != nullptr)
			return mInternal->GetInertiaTensor();

		return Vector3::ZERO;
	}

	void CRigidbody::SetMaxAngularVelocity(float maxVelocity)
	{
		mMaxAngularVelocity = maxVelocity;

		if (mInternal != nullptr)
			mInternal->SetMaxAngularVelocity(maxVelocity);
	}

	void CRigidbody::SetCenterOfMassPosition(const Vector3& position)
	{
		mCMassPosition = position;

		if (mInternal != nullptr)
			mInternal->SetCenterOfMass(position, mCMassRotation);
	}

	void CRigidbody::SetCenterOfMassRotation(const Quaternion& rotation)
	{
		mCMassRotation = rotation;

		if (mInternal != nullptr)
			mInternal->SetCenterOfMass(mCMassPosition, rotation);
	}

	Vector3 CRigidbody::GetCenterOfMassPosition() const
	{
		if (mInternal != nullptr)
			return mInternal->GetCenterOfMassPosition();

		return Vector3::ZERO;
	}

	Quaternion CRigidbody::GetCenterOfMassRotation() const
	{
		if (mInternal != nullptr)
			return mInternal->GetCenterOfMassRotation();

		return Quaternion::IDENTITY;
	}

	void CRigidbody::SetPositionSolverCount(UINT32 count)
	{
		mPositionSolverCount = count;

		if (mInternal != nullptr)
			mInternal->SetPositionSolverCount(count);
	}

	void CRigidbody::SetVelocitySolverCount(UINT32 count)
	{
		mVelocitySolverCount = count;

		if (mInternal != nullptr)
			mInternal->SetVelocitySolverCount(count);
	}

	void CRigidbody::SetCollisionReportMode(CollisionReportMode mode)
	{
		if (mCollisionReportMode == mode)
			return;

		mCollisionReportMode = mode;

		for (auto& entry : mChildren)
			entry->UpdateCollisionReportMode();
	}

	void CRigidbody::SetFlags(RigidbodyFlag flags)
	{
		mFlags = flags;

		if (mInternal != nullptr)
		{
			mInternal->SetFlags(flags);
			mInternal->UpdateMassDistribution();
		}
	}

	void CRigidbody::AddForce(const Vector3& force, ForceMode mode)
	{
		if (mInternal != nullptr)
			mInternal->AddForce(force, mode);
	}

	void CRigidbody::AddTorque(const Vector3& torque, ForceMode mode)
	{
		if (mInternal != nullptr)
			mInternal->AddTorque(torque, mode);
	}

	void CRigidbody::AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode)
	{
		if (mInternal != nullptr)
			mInternal->AddForceAtPoint(force, position, mode);
	}

	Vector3 CRigidbody::GetVelocityAtPoint(const Vector3& point) const
	{
		if (mInternal != nullptr)
			return mInternal->GetVelocityAtPoint(point);

		return Vector3::ZERO;
	}

	void CRigidbody::_updateMassDistribution()
	{
		if (mInternal != nullptr)
			return mInternal->UpdateMassDistribution();
	}

	void CRigidbody::UpdateColliders()
	{
		Stack<HSceneObject> todo;
		todo.Push(SO());

		while(!todo.Empty())
		{
			HSceneObject currentSO = todo.Top();
			todo.Pop();

			if(currentSO->hasComponent<CCollider>())
			{
				Vector<HCollider> colliders = currentSO->getComponents<CCollider>();
				
				for (auto& entry : colliders)
				{
					if (!entry->IsValidParent(static_object_cast<CRigidbody>(mThisHandle)))
						continue;

					Collider* collider = entry->_getInternal();
					if (collider == nullptr)
						continue;

					entry->SetRigidbody(static_object_cast<CRigidbody>(mThisHandle), true);
					mChildren.push_back(entry);

					collider->SetRigidbody(mInternal.Get());
					mInternal->AddCollider(collider);
				}
			}

			UINT32 childCount = currentSO->GetNumChildren();
			for (UINT32 i = 0; i < childCount; i++)
			{
				HSceneObject child = currentSO->GetChild(i);

				if (child->hasComponent<CRigidbody>())
					continue;

				todo.Push(child);
			}
		}
	}

	void CRigidbody::ClearColliders()
	{
		for (auto& collider : mChildren)
			collider->SetRigidbody(HRigidbody(), true);

		mChildren.Clear();

		if (mInternal != nullptr)
			mInternal->RemoveColliders();
	}

	void CRigidbody::AddCollider(const HCollider& collider)
	{
		if (mInternal == nullptr)
			return;

		mChildren.push_back(collider);
		mInternal->AddCollider(collider->_getInternal());
	}

	void CRigidbody::RemoveCollider(const HCollider& collider)
	{
		if (mInternal == nullptr)
			return;

		auto iterFind = std::find(mChildren.Begin(), mChildren.end(), collider);

		if(iterFind != mChildren.End())
		{
			mInternal->RemoveCollider(collider->_getInternal());
			mChildren.Erase(iterFind);
		}
	}

	void CRigidbody::CheckForNestedRigibody()
	{
		HSceneObject currentSO = SO()->GetParent();

		while(currentSO != nullptr)
		{
			if(currentSO->hasComponent<CRigidbody>())
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
		output.contactPoints = std::move(data.contactPoints);

		if (data.colliders[0] != nullptr)
		{
			CCollider* other = (CCollider*)data.colliders[0]->_getOwner(PhysicsOwnerType::Component);
			output.collider[0] = static_object_cast<CCollider>(other->GetHandle());
		}

		if (data.colliders[1] != nullptr)
		{
			CCollider* other = (CCollider*)data.colliders[1]->_getOwner(PhysicsOwnerType::Component);
			output.collider[1] = static_object_cast<CCollider>(other->GetHandle());
		}
	}

	void CRigidbody::DestroyInternal()
	{
		clearColliders();

		if(mInternal)
		{
			mInternal->_setOwner(PhysicsOwnerType::None, nullptr);
			mInternal = nullptr;
		}
	}

	void CRigidbody::TriggerOnCollisionBegin(const CollisionDataRaw& data)
	{
		CollisionData hit;
		processCollisionData(data, hit);

		onCollisionBegin(hit);
	}

	void CRigidbody::TriggerOnCollisionStay(const CollisionDataRaw& data)
	{
		CollisionData hit;
		processCollisionData(data, hit);

		onCollisionStay(hit);
	}

	void CRigidbody::TriggerOnCollisionEnd(const CollisionDataRaw& data)
	{
		CollisionData hit;
		processCollisionData(data, hit);

		onCollisionEnd(hit);
	}

	void CRigidbody::OnInitialized()
	{

	}

	void CRigidbody::OnDestroyed()
	{
		destroyInternal();
	}

	void CRigidbody::OnDisabled()
	{
		destroyInternal();
	}

	void CRigidbody::OnEnabled()
	{
		mInternal = Rigidbody::create(SO());
		mInternal->_setOwner(PhysicsOwnerType::Component, this);

		updateColliders();

#if BS_DEBUG_MODE
		checkForNestedRigibody();
#endif

		mInternal->onCollisionBegin.Connect(std::bind(&CRigidbody::triggerOnCollisionBegin, this, _1));
		mInternal->onCollisionStay.Connect(std::bind(&CRigidbody::triggerOnCollisionStay, this, _1));
		mInternal->onCollisionEnd.Connect(std::bind(&CRigidbody::triggerOnCollisionEnd, this, _1));

		const Transform& tfrm = SO()->GetTransform();
		mInternal->SetTransform(tfrm.GetPosition(), tfrm.getRotation());

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

		if(((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoTensors) == 0)
		{
			mInternal->SetCenterOfMass(mCMassPosition, mCMassRotation);
			mInternal->SetInertiaTensor(mInertiaTensor);
			mInternal->SetMass(mMass);
		}
		else
		{
			if (((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoMass) == 0)
				mInternal->SetMass(mMass);

			mInternal->UpdateMassDistribution();
		}
	}

	void CRigidbody::OnTransformChanged(TransformChangedFlags flags)
	{
		if (!SO()->GetActive())
			return;

		if((flags & TCF_Parent) != 0)
		{
			clearColliders();
			updateColliders();

			if (((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoTensors) != 0)
				mInternal->UpdateMassDistribution();

#if BS_DEBUG_MODE
			checkForNestedRigibody();
#endif
		}
		
		if(gPhysics()._isUpdateInProgress())
			return;

		const Transform& tfrm = SO()->GetTransform();
		mInternal->SetTransform(tfrm.GetPosition(), tfrm.getRotation());

		if (mParentJoint != nullptr)
			mParentJoint->NotifyRigidbodyMoved(static_object_cast<CRigidbody>(mThisHandle));
	}

	RTTITypeBase* CRigidbody::getRTTIStatic()
	{
		return CRigidbodyRTTI::Instance();
	}

	RTTITypeBase* CRigidbody::getRTTI() const
	{
		return CRigidbody::GetRTTIStatic();
	}
}
