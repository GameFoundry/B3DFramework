//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCCapsuleCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCCapsuleColliderRTTI.h"

namespace bs
{
	CCapsuleCollider::CCapsuleCollider()
	{
		setName("CapsuleCollider");

		mLocalRotation = Quaternion::getRotationFromTo(Vector3::UNIT_X, mNormal);
	}

	CCapsuleCollider::CCapsuleCollider(const HSceneObject& parent, float radius, float halfHeight)
		: CCollider(parent), mRadius(radius), mHalfHeight(halfHeight)
	{
		setName("CapsuleCollider");

		mLocalRotation = Quaternion::getRotationFromTo(Vector3::UNIT_X, mNormal);
	}

	void CCapsuleCollider::setNormal(const Vector3& normal)
	{
		if (mNormal == normal)
			return;

		mNormal = bs::Vector3::normalize(normal);
		mLocalRotation = Quaternion::getRotationFromTo(Vector3::UNIT_X, mNormal);

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