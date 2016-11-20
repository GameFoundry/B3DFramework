//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCSphereCollider.h"
#include "BsSceneObject.h"
#include "BsCRigidbody.h"
#include "BsCSphereColliderRTTI.h"

namespace bs
{
	CSphereCollider::CSphereCollider(const HSceneObject& parent, float radius)
		: CCollider(parent), mRadius(radius)
	{
		setName("SphereCollider");
	}

	void CSphereCollider::setRadius(float radius)
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

	void CSphereCollider::setCenter(const Vector3& center)
	{
		if (mLocalPosition == center)
			return;

		mLocalPosition = center; 
		
		if (mInternal != nullptr)
			updateTransform();
	}

	SPtr<Collider> CSphereCollider::createInternal()
	{
		SPtr<Collider> collider = SphereCollider::create(mRadius, SO()->getWorldPosition(), SO()->getWorldRotation());

		collider->_setOwner(PhysicsOwnerType::Component, this);
		return collider;
	}

	RTTITypeBase* CSphereCollider::getRTTIStatic()
	{
		return CSphereColliderRTTI::instance();
	}

	RTTITypeBase* CSphereCollider::getRTTI() const
	{
		return CSphereCollider::getRTTIStatic();
	}
}