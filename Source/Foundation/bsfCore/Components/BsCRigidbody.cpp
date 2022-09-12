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
			mInternal->move(position);

		mNotifyFlags = (TransformChangedFlags)0;
		SO()->setWorldPosition(position);
		mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
	}

	void CRigidbody::Rotate(const Quaternion& rotation)
	{
		if (mInternal != nullptr)
			mInternal->rotate(rotation);

		mNotifyFlags = (TransformChangedFlags)0;
		SO()->setWorldRotation(rotation);
		mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
	}

	void CRigidbody::SetMass(float mass)
	{
		mMass = mass;

		if(mInternal != nullptr)
			mInternal->setMass(mass);
	}

	void CRigidbody::SetIsKinematic(bool kinematic)
	{
		if (mIsKinematic == kinematic)
			return;

		mIsKinematic = kinematic;
		
		if (mInternal != nullptr)
		{
			mInternal->setIsKinematic(kinematic);

			clearColliders();
			updateColliders();
		}
	}

	bool CRigidbody::IsSleeping() const
	{
		if (mInternal != nullptr)
			return mInternal->isSleeping();

		return true;
	}

	void CRigidbody::Sleep()
	{
		if (mInternal != nullptr)
			return mInternal->sleep();
	}

	void CRigidbody::WakeUp()
	{
		if (mInternal != nullptr)
			return mInternal->wakeUp();
	}

	void CRigidbody::SetSleepThreshold(float threshold)
	{
		mSleepThreshold = threshold;

		if (mInternal != nullptr)
			mInternal->setSleepThreshold(threshold);
	}

	void CRigidbody::SetUseGravity(bool gravity)
	{
		mUseGravity = gravity;

		if (mInternal != nullptr)
			mInternal->setUseGravity(gravity);
	}

	void CRigidbody::SetVelocity(const Vector3& velocity)
	{
		if (mInternal != nullptr)
			mInternal->setVelocity(velocity);
	}

	Vector3 CRigidbody::GetVelocity() const
	{
		if (mInternal != nullptr)
			return mInternal->getVelocity();

		return Vector3::ZERO;
	}

	void CRigidbody::SetAngularVelocity(const Vector3& velocity)
	{
		if (mInternal != nullptr)
			mInternal->setAngularVelocity(velocity);
	}

	Vector3 CRigidbody::GetAngularVelocity() const
	{
		if (mInternal != nullptr)
			return mInternal->getAngularVelocity();

		return Vector3::ZERO;
	}

	void CRigidbody::SetDrag(float drag)
	{
		mLinearDrag = drag;

		if (mInternal != nullptr)
			mInternal->setDrag(drag);
	}

	void CRigidbody::SetAngularDrag(float drag)
	{
		mAngularDrag = drag;

		if (mInternal != nullptr)
			mInternal->setAngularDrag(drag);
	}

	void CRigidbody::SetInertiaTensor(const Vector3& tensor)
	{
		mInertiaTensor = tensor;

		if (mInternal != nullptr)
			mInternal->setInertiaTensor(tensor);
	}

	Vector3 CRigidbody::GetInertiaTensor() const
	{
		if (mInternal != nullptr)
			return mInternal->getInertiaTensor();

		return Vector3::ZERO;
	}

	void CRigidbody::SetMaxAngularVelocity(float maxVelocity)
	{
		mMaxAngularVelocity = maxVelocity;

		if (mInternal != nullptr)
			mInternal->setMaxAngularVelocity(maxVelocity);
	}

	void CRigidbody::SetCenterOfMassPosition(const Vector3& position)
	{
		mCMassPosition = position;

		if (mInternal != nullptr)
			mInternal->setCenterOfMass(position, mCMassRotation);
	}

	void CRigidbody::SetCenterOfMassRotation(const Quaternion& rotation)
	{
		mCMassRotation = rotation;

		if (mInternal != nullptr)
			mInternal->setCenterOfMass(mCMassPosition, rotation);
	}

	Vector3 CRigidbody::GetCenterOfMassPosition() const
	{
		if (mInternal != nullptr)
			return mInternal->getCenterOfMassPosition();

		return Vector3::ZERO;
	}

	Quaternion CRigidbody::GetCenterOfMassRotation() const
	{
		if (mInternal != nullptr)
			return mInternal->getCenterOfMassRotation();

		return Quaternion::IDENTITY;
	}

	void CRigidbody::SetPositionSolverCount(UINT32 count)
	{
		mPositionSolverCount = count;

		if (mInternal != nullptr)
			mInternal->setPositionSolverCount(count);
	}

	void CRigidbody::SetVelocitySolverCount(UINT32 count)
	{
		mVelocitySolverCount = count;

		if (mInternal != nullptr)
			mInternal->setVelocitySolverCount(count);
	}

	void CRigidbody::SetCollisionReportMode(CollisionReportMode mode)
	{
		if (mCollisionReportMode == mode)
			return;

		mCollisionReportMode = mode;

		for (auto& entry : mChildren)
			entry->updateCollisionReportMode();
	}

	void CRigidbody::SetFlags(RigidbodyFlag flags)
	{
		mFlags = flags;

		if (mInternal != nullptr)
		{
			mInternal->setFlags(flags);
			mInternal->updateMassDistribution();
		}
	}

	void CRigidbody::AddForce(const Vector3& force, ForceMode mode)
	{
		if (mInternal != nullptr)
			mInternal->addForce(force, mode);
	}

	void CRigidbody::AddTorque(const Vector3& torque, ForceMode mode)
	{
		if (mInternal != nullptr)
			mInternal->addTorque(torque, mode);
	}

	void CRigidbody::AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode)
	{
		if (mInternal != nullptr)
			mInternal->addForceAtPoint(force, position, mode);
	}

	Vector3 CRigidbody::GetVelocityAtPoint(const Vector3& point) const
	{
		if (mInternal != nullptr)
			return mInternal->getVelocityAtPoint(point);

		return Vector3::ZERO;
	}

	void CRigidbody::_updateMassDistribution()
	{
		if (mInternal != nullptr)
			return mInternal->updateMassDistribution();
	}

	void CRigidbody::UpdateColliders()
	{
		Stack<HSceneObject> todo;
		todo.push(SO());

		while(!todo.empty())
		{
			HSceneObject currentSO = todo.top();
			todo.pop();

			if(currentSO->hasComponent<CCollider>())
			{
				Vector<HCollider> colliders = currentSO->getComponents<CCollider>();
				
				for (auto& entry : colliders)
				{
					if (!entry->isValidParent(static_object_cast<CRigidbody>(mThisHandle)))
						continue;

					Collider* collider = entry->_getInternal();
					if (collider == nullptr)
						continue;

					entry->setRigidbody(static_object_cast<CRigidbody>(mThisHandle), true);
					mChildren.push_back(entry);

					collider->setRigidbody(mInternal.get());
					mInternal->addCollider(collider);
				}
			}

			UINT32 childCount = currentSO->getNumChildren();
			for (UINT32 i = 0; i < childCount; i++)
			{
				HSceneObject child = currentSO->getChild(i);

				if (child->hasComponent<CRigidbody>())
					continue;

				todo.push(child);
			}
		}
	}

	void CRigidbody::ClearColliders()
	{
		for (auto& collider : mChildren)
			collider->setRigidbody(HRigidbody(), true);

		mChildren.clear();

		if (mInternal != nullptr)
			mInternal->removeColliders();
	}

	void CRigidbody::AddCollider(const HCollider& collider)
	{
		if (mInternal == nullptr)
			return;

		mChildren.push_back(collider);
		mInternal->addCollider(collider->_getInternal());
	}

	void CRigidbody::RemoveCollider(const HCollider& collider)
	{
		if (mInternal == nullptr)
			return;

		auto iterFind = std::find(mChildren.begin(), mChildren.end(), collider);

		if(iterFind != mChildren.end())
		{
			mInternal->removeCollider(collider->_getInternal());
			mChildren.erase(iterFind);
		}
	}

	void CRigidbody::CheckForNestedRigibody()
	{
		HSceneObject currentSO = SO()->getParent();

		while(currentSO != nullptr)
		{
			if(currentSO->hasComponent<CRigidbody>())
			{
				BS_LOG(Warning, Physics, "Nested Rigidbodies detected. This will result in inconsistent transformations. "
					"To parent one Rigidbody to another move its colliders to the new parent, but remove the Rigidbody "
					"component.");
				return;
			}

			currentSO = currentSO->getParent();
		}
	}

	void CRigidbody::ProcessCollisionData(const CollisionDataRaw& data, CollisionData& output)
	{
		output.contactPoints = std::move(data.contactPoints);

		if (data.colliders[0] != nullptr)
		{
			CCollider* other = (CCollider*)data.colliders[0]->_getOwner(PhysicsOwnerType::Component);
			output.collider[0] = static_object_cast<CCollider>(other->getHandle());
		}

		if (data.colliders[1] != nullptr)
		{
			CCollider* other = (CCollider*)data.colliders[1]->_getOwner(PhysicsOwnerType::Component);
			output.collider[1] = static_object_cast<CCollider>(other->getHandle());
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

		mInternal->onCollisionBegin.connect(std::bind(&CRigidbody::triggerOnCollisionBegin, this, _1));
		mInternal->onCollisionStay.connect(std::bind(&CRigidbody::triggerOnCollisionStay, this, _1));
		mInternal->onCollisionEnd.connect(std::bind(&CRigidbody::triggerOnCollisionEnd, this, _1));

		const Transform& tfrm = SO()->getTransform();
		mInternal->setTransform(tfrm.getPosition(), tfrm.getRotation());

		// Note: Merge into one call to avoid many virtual function calls
		mInternal->setPositionSolverCount(mPositionSolverCount);
		mInternal->setVelocitySolverCount(mVelocitySolverCount);
		mInternal->setMaxAngularVelocity(mMaxAngularVelocity);
		mInternal->setDrag(mLinearDrag);
		mInternal->setAngularDrag(mAngularDrag);
		mInternal->setSleepThreshold(mSleepThreshold);
		mInternal->setUseGravity(mUseGravity);
		mInternal->setIsKinematic(mIsKinematic);
		mInternal->setFlags(mFlags);

		if(((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoTensors) == 0)
		{
			mInternal->setCenterOfMass(mCMassPosition, mCMassRotation);
			mInternal->setInertiaTensor(mInertiaTensor);
			mInternal->setMass(mMass);
		}
		else
		{
			if (((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoMass) == 0)
				mInternal->setMass(mMass);

			mInternal->updateMassDistribution();
		}
	}

	void CRigidbody::OnTransformChanged(TransformChangedFlags flags)
	{
		if (!SO()->getActive())
			return;

		if((flags & TCF_Parent) != 0)
		{
			clearColliders();
			updateColliders();

			if (((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoTensors) != 0)
				mInternal->updateMassDistribution();

#if BS_DEBUG_MODE
			checkForNestedRigibody();
#endif
		}
		
		if(gPhysics()._isUpdateInProgress())
			return;

		const Transform& tfrm = SO()->getTransform();
		mInternal->setTransform(tfrm.getPosition(), tfrm.getRotation());

		if (mParentJoint != nullptr)
			mParentJoint->notifyRigidbodyMoved(static_object_cast<CRigidbody>(mThisHandle));
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
