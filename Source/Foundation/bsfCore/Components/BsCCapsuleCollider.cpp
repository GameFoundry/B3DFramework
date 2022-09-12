//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCCapsuleCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCCapsuleColliderRTTI.h"
#include "Scene/BsSceneManager.h"

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

	void CCapsuleCollider::SetNormal(const Vector3& normal)
	{
		if (mNormal == normal)
			return;

		mNormal = bs::Vector3::normalize(normal);
		mLocalRotation = Quaternion::getRotationFromTo(Vector3::UNIT_X, mNormal);

		if (mInternal != nullptr)
			updateTransform();
	}

	void CCapsuleCollider::SetCenter(const Vector3& center)
	{
		if (mLocalPosition == center)
			return;

		mLocalPosition = center;

		if (mInternal != nullptr)
			updateTransform();
	}

	void CCapsuleCollider::SetHalfHeight(float halfHeight)
	{
		float clampedHalfHeight = std::max(halfHeight, 0.01f);
		if (mHalfHeight == clampedHalfHeight)
			return;

		mHalfHeight = clampedHalfHeight;

		if (mInternal != nullptr)
		{
			_getInternal()->SetHalfHeight(clampedHalfHeight);

			if (mParent != nullptr)
				mParent->_updateMassDistribution();
		}
	}

	void CCapsuleCollider::SetRadius(float radius)
	{
		float clampedRadius = std::max(radius, 0.01f);
		if (mRadius == clampedRadius)
			return;

		mRadius = clampedRadius;

		if (mInternal != nullptr)
		{
			_getInternal()->SetRadius(clampedRadius);

			if (mParent != nullptr)
				mParent->_updateMassDistribution();
		}
	}

	SPtr<Collider> CCapsuleCollider::CreateInternal()
	{
		const SPtr<SceneInstance>& scene = SO()->GetScene();
		const Transform& tfrm = SO()->GetTransform();

		SPtr<Collider> collider = CapsuleCollider::create(*scene->GetPhysicsScene(), mRadius, mHalfHeight,
			tfrm.GetPosition(), tfrm.getRotation());

		collider->_setOwner(PhysicsOwnerType::Component, this);
		return collider;
	}

	RTTITypeBase* CCapsuleCollider::getRTTIStatic()
	{
		return CCapsuleColliderRTTI::Instance();
	}

	RTTITypeBase* CCapsuleCollider::getRTTI() const
	{
		return CCapsuleCollider::GetRTTIStatic();
	}
}
