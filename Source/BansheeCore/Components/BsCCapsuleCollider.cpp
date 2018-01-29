//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Components/BsCCapsuleCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCCapsuleColliderRTTI.h"

namespace bs
{
	CCapsuleCollider::CCapsuleCollider()
	{
		setName("CapsuleCollider");
	}

	CCapsuleCollider::CCapsuleCollider(const HSceneObject& parent, float radius, float halfHeight)
		: CCollider(parent), mRadius(radius), mHalfHeight(halfHeight)
	{
		setName("CapsuleCollider");
	}

	void CCapsuleCollider::setNormal(const Vector3& normal)
	{
		if (mNormal == normal)
			return;

		mNormal = normal;
		mNormal.normalize();

		mLocalRotation = Quaternion::getRotationFromTo(Vector3::UNIT_X, normal);

		if (mInternal != nullptr)
			updateTransform();
	}

	void CCapsuleCollider::setCenter(const Vector3& center)
	{
		if (mLocalPosition == center)
			return;

		mLocalPosition = center;

		if (mInternal != nullptr)
			updateTransform();
	}

	void CCapsuleCollider::setHalfHeight(float halfHeight)
	{
		float clampedHalfHeight = std::max(halfHeight, 0.01f);
		if (mHalfHeight == clampedHalfHeight)
			return;

		mHalfHeight = clampedHalfHeight;

		if (mInternal != nullptr)
		{
			_getInternal()->setHalfHeight(clampedHalfHeight);

			if (mParent != nullptr)
				mParent->_updateMassDistribution();
		}
	}

	void CCapsuleCollider::setRadius(float radius)
	{
		float clampedRadius = std::max(radius, 0.01f);
		if (mRadius == clampedRadius)
			return;

		mRadius = clampedRadius;

		if (mInternal != nullptr)
		{
			_getInternal()->setRadius(clampedRadius);

			if (mParent != nullptr)
				mParent->_updateMassDistribution();
		}
	}

	SPtr<Collider> CCapsuleCollider::createInternal()
	{
		const Transform& tfrm = SO()->getTransform();
		SPtr<Collider> collider = CapsuleCollider::create(mRadius, mHalfHeight, tfrm.getPosition(), 
			tfrm.getRotation());

		collider->_setOwner(PhysicsOwnerType::Component, this);
		return collider;
	}

	RTTITypeBase* CCapsuleCollider::getRTTIStatic()
	{
		return CCapsuleColliderRTTI::instance();
	}

	RTTITypeBase* CCapsuleCollider::getRTTI() const
	{
		return CCapsuleCollider::getRTTIStatic();
	}
}