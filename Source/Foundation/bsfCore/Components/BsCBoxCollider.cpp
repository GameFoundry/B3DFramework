//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCBoxCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCBoxColliderRTTI.h"
#include "Scene/BsSceneManager.h"

namespace bs
{
	CBoxCollider::CBoxCollider()
	{
		setName("BoxCollider");
	}

	CBoxCollider::CBoxCollider(const HSceneObject& parent, const Vector3& extents)
		: CCollider(parent), mExtents(extents)
	{
		setName("BoxCollider");
	}

	void CBoxCollider::setExtents(const Vector3& extents)
	{
		Vector3 clampedExtents = Vector3::max(extents, Vector3(0.01f, 0.01f, 0.01f));

		if (mExtents == clampedExtents)
			return;

		mExtents = clampedExtents;

		if (mInternal != nullptr)
		{
			_getInternal()->setExtents(clampedExtents);

			if (mParent != nullptr)
				mParent->_updateMassDistribution();
		}
	}

	void CBoxCollider::setCenter(const Vector3& center)
	{
		if (mLocalPosition == center)
			return;

		mLocalPosition = center;

		if (mInternal != nullptr)
			updateTransform();
	}

	SPtr<Collider> CBoxCollider::createInternal()
	{
		const SPtr<SceneInstance>& scene = SO()->getScene();
		const Transform& tfrm = SO()->getTransform();

		SPtr<Collider> collider = BoxCollider::create(*scene->getPhysicsScene(), mExtents, tfrm.getPosition(), 
			tfrm.getRotation());
		collider->_setOwner(PhysicsOwnerType::Component, this);

		return collider;
	}

	RTTITypeBase* CBoxCollider::getRTTIStatic()
	{
		return CBoxColliderRTTI::instance();
	}

	RTTITypeBase* CBoxCollider::getRTTI() const
	{
		return CBoxCollider::getRTTIStatic();
	}
}
