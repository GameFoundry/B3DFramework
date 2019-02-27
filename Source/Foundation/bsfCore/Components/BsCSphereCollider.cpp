//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCSphereCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCSphereColliderRTTI.h"
#include "Scene/BsSceneManager.h"

namespace bs
{
	CSphereCollider::CSphereCollider()
	{
		setName("SphereCollider");
	}

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
		const SPtr<SceneInstance>& scene = SO()->getScene();
		const Transform& tfrm = SO()->getTransform();

		SPtr<Collider> collider = SphereCollider::create(*scene->getPhysicsScene(), mRadius, tfrm.getPosition(), 
			tfrm.getRotation());

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
