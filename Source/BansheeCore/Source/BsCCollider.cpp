//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCCollider.h"
#include "BsSceneObject.h"
#include "BsCRigidbody.h"
#include "BsPhysics.h"
#include "BsCColliderRTTI.h"

using namespace std::placeholders;

namespace bs
{
	CCollider::CCollider(const HSceneObject& parent)
		: Component(parent)
	{
		setName("Collider");

		mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
	}

	void CCollider::setIsTrigger(bool value)
	{
		if (mIsTrigger == value)
			return;

		mIsTrigger = value;

		if (mInternal != nullptr)
		{
			mInternal->setIsTrigger(value);

			updateParentRigidbody();
			updateTransform();
		}
	}

	void CCollider::setMass(float mass)
	{
		if (mMass == mass)
			return;

		mMass = mass;

		if (mInternal != nullptr)
		{
			mInternal->setMass(mass);

			if (mParent != nullptr)
				mParent->_updateMassDistribution();
		}
	}

	void CCollider::setMaterial(const HPhysicsMaterial& material)
	{
		mMaterial = material;

		if (mInternal != nullptr)
			mInternal->setMaterial(material);
	}

	void CCollider::setContactOffset(float value)
	{
		value = std::max(0.0f, std::max(value, getRestOffset()));

		mContactOffset = value;

		if (mInternal != nullptr)
			mInternal->setContactOffset(value);
	}

	void CCollider::setRestOffset(float value)
	{
		value = std::min(value, getContactOffset());

		mRestOffset = value;

		if (mInternal != nullptr)
			mInternal->setRestOffset(value);
	}

	void CCollider::setLayer(UINT64 layer)
	{
		mLayer = layer;

		if (mInternal != nullptr)
			mInternal->setLayer(layer);
	}

	void CCollider::setCollisionReportMode(CollisionReportMode mode)
	{
		mCollisionReportMode = mode;

		if (mInternal != nullptr)
			updateCollisionReportMode();
	}

	void CCollider::onInitialized()
	{

	}

	void CCollider::onDestroyed()
	{
		destroyInternal();
	}

	void CCollider::onDisabled()
	{
		destroyInternal();
	}

	void CCollider::onEnabled()
	{
		restoreInternal();
	}

	void CCollider::onTransformChanged(TransformChangedFlags flags)
	{
		if (!SO()->getActive())
			return;

		if ((flags & TCF_Parent) != 0)
			updateParentRigidbody();

		// Don't update the transform if it's due to Physics update since then we can guarantee it will remain at the same
		// relative transform to its parent
		if (gPhysics()._isUpdateInProgress())
			return;

		if ((flags & (TCF_Parent | TCF_Transform)) != 0)
			updateTransform();
	}

	void CCollider::setRigidbody(const HRigidbody& rigidbody, bool internal)
	{
		if (rigidbody == mParent)
			return;

		if (mInternal != nullptr && !internal)
		{
			if (mParent != nullptr)
				mParent->removeCollider(mThisHandle);

			Rigidbody* rigidBodyPtr = nullptr;

			if (rigidbody != nullptr)
				rigidBodyPtr = rigidbody->_getInternal();

			mInternal->setRigidbody(rigidBodyPtr);

			if (rigidbody != nullptr)
				rigidbody->addCollider(mThisHandle);
		}

		mParent = rigidbody;
		updateCollisionReportMode();
	}

	bool CCollider::rayCast(const Ray& ray, PhysicsQueryHit& hit, float maxDist) const
	{
		if (mInternal == nullptr)
			return false;

		return mInternal->rayCast(ray, hit, maxDist);
	}

	bool CCollider::rayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit,
		float maxDist) const
	{
		if (mInternal == nullptr)
			return false;

		return mInternal->rayCast(origin, unitDir, hit, maxDist);
	}

	void CCollider::restoreInternal()
	{
		if (mInternal == nullptr)
		{
			mInternal = createInternal();

			mInternal->onCollisionBegin.connect(std::bind(&CCollider::triggerOnCollisionBegin, this, _1));
			mInternal->onCollisionStay.connect(std::bind(&CCollider::triggerOnCollisionStay, this, _1));
			mInternal->onCollisionEnd.connect(std::bind(&CCollider::triggerOnCollisionEnd, this, _1));
		}

		// Note: Merge into one call to avoid many virtual function calls
		mInternal->setIsTrigger(mIsTrigger);
		mInternal->setMass(mMass);
		mInternal->setMaterial(mMaterial);
		mInternal->setContactOffset(mContactOffset);
		mInternal->setRestOffset(mRestOffset);
		mInternal->setLayer(mLayer);

		updateParentRigidbody();
		updateTransform();
		updateCollisionReportMode();
	}

	void CCollider::destroyInternal()
	{
		if (mParent != nullptr)
			mParent->removeCollider(mThisHandle);

		mParent = nullptr;

		// This should release the last reference and destroy the internal collider
		mInternal->_setOwner(PhysicsOwnerType::None, nullptr);
		mInternal = nullptr;
	}

	void CCollider::updateParentRigidbody()
	{
		if (mIsTrigger)
		{
			setRigidbody(HRigidbody());
			return;
		}

		HSceneObject currentSO = SO();
		while (currentSO != nullptr)
		{
			HRigidbody parent = currentSO->getComponent<CRigidbody>();
			if (parent != nullptr)
			{
				if(currentSO->getActive() && isValidParent(parent))
					setRigidbody(parent);
				else
					setRigidbody(HRigidbody());

				return;
			}

			currentSO = currentSO->getParent();
		}

		// Not found
		setRigidbody(HRigidbody());
	}

	void CCollider::updateTransform()
	{
		Vector3 myScale = SO()->getWorldScale();

		if (mParent != nullptr)
		{
			Vector3 parentPos = mParent->SO()->getWorldPosition();
			Quaternion parentRot = mParent->SO()->getWorldRotation();

			Vector3 myPos = SO()->getWorldPosition();
			Quaternion myRot = SO()->getWorldRotation();

			Vector3 scale = mParent->SO()->getWorldScale();
			Vector3 invScale = scale;
			if (invScale.x != 0) invScale.x = 1.0f / invScale.x;
			if (invScale.y != 0) invScale.y = 1.0f / invScale.y;
			if (invScale.z != 0) invScale.z = 1.0f / invScale.z;

			Quaternion invRotation = parentRot.inverse();

			Vector3 relativePos = invRotation.rotate(myPos - parentPos) *  invScale;
			Quaternion relativeRot = invRotation * myRot;

			relativePos = relativePos + myRot.rotate(mLocalPosition * scale);
			relativeRot = relativeRot * mLocalRotation;

			mInternal->setTransform(relativePos, relativeRot);
			mParent->_updateMassDistribution();
		}
		else
		{
			Quaternion myRot = SO()->getWorldRotation();
			Vector3 myPos = SO()->getWorldPosition() + myRot.rotate(mLocalPosition * myScale);
			myRot = myRot * mLocalRotation;

			mInternal->setTransform(myPos, myRot);
		}

		mInternal->setScale(myScale);
	}

	void CCollider::updateCollisionReportMode()
	{
		CollisionReportMode mode = mCollisionReportMode;

		if (mParent != nullptr)
			mode = mParent->getCollisionReportMode();

		if(mInternal != nullptr)
			mInternal->setCollisionReportMode(mode);
	}

	void CCollider::triggerOnCollisionBegin(const CollisionData& data)
	{
		// Const-cast and modify is okay because we're the only object receiving this event
		CollisionData& hit = const_cast<CollisionData&>(data);
		hit.collider[0] = mThisHandle;

		if(hit.collidersRaw[1] != nullptr)
		{
			CCollider* other = (CCollider*)hit.collidersRaw[1]->_getOwner(PhysicsOwnerType::Component);
			hit.collider[1] = other->getHandle();
		}

		onCollisionBegin(hit);
	}

	void CCollider::triggerOnCollisionStay(const CollisionData& data)
	{
		// Const-cast and modify is okay because we're the only object receiving this event
		CollisionData& hit = const_cast<CollisionData&>(data);
		hit.collider[0] = mThisHandle;

		if (hit.collidersRaw[1] != nullptr)
		{
			CCollider* other = (CCollider*)hit.collidersRaw[1]->_getOwner(PhysicsOwnerType::Component);
			hit.collider[1] = other->getHandle();
		}

		onCollisionStay(hit);
	}

	void CCollider::triggerOnCollisionEnd(const CollisionData& data)
	{
		// Const-cast and modify is okay because we're the only object receiving this event
		CollisionData& hit = const_cast<CollisionData&>(data);
		hit.collider[0] = mThisHandle;

		if (hit.collidersRaw[1] != nullptr)
		{
			CCollider* other = (CCollider*)hit.collidersRaw[1]->_getOwner(PhysicsOwnerType::Component);
			hit.collider[1] = other->getHandle();
		}

		onCollisionEnd(hit);
	}

	RTTITypeBase* CCollider::getRTTIStatic()
	{
		return CColliderRTTI::instance();
	}

	RTTITypeBase* CCollider::getRTTI() const
	{
		return CCollider::getRTTIStatic();
	}
}