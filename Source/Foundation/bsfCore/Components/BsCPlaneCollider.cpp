//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCPlaneCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCPlaneColliderRTTI.h"
#include "Scene/BsSceneManager.h"

namespace bs
{
	CPlaneCollider::CPlaneCollider()
	{
		setName("PlaneCollider");

		mLocalRotation = Quaternion::getRotationFromTo(Vector3::UNIT_X, mNormal);
	}

	CPlaneCollider::CPlaneCollider(const HSceneObject& parent)
		: CCollider(parent)
	{
		setName("PlaneCollider");

		mLocalRotation = Quaternion::getRotationFromTo(Vector3::UNIT_X, mNormal);
	}

	void CPlaneCollider::setNormal(const Vector3& normal)
	{
		if (mNormal == normal)
			return;

		mNormal = normal;
		mNormal.normalize();
		
		mLocalRotation = Quaternion::getRotationFromTo(Vector3::UNIT_X, normal);
		mLocalPosition = mNormal * mDistance;

		if(mInternal != nullptr)
			updateTransform();
	}

	void CPlaneCollider::setDistance(float distance)
	{
		if (mDistance == distance)
			return;

		mDistance = distance; 
		mLocalPosition = mNormal * distance;

		if (mInternal != nullptr)
			updateTransform();
	}

	SPtr<Collider> CPlaneCollider::createInternal()
	{
		const SPtr<SceneInstance>& scene = SO()->getScene();
		const Transform& tfrm = SO()->getTransform();

		SPtr<Collider> collider = PlaneCollider::create(*scene->getPhysicsScene(), tfrm.getPosition(), tfrm.getRotation());

		collider->_setOwner(PhysicsOwnerType::Component, this);
		return collider;
	}

	bool CPlaneCollider::isValidParent(const HRigidbody& parent) const
	{
		// Planes cannot be added to non-kinematic rigidbodies
		return parent->getIsKinematic();
	}

	RTTITypeBase* CPlaneCollider::getRTTIStatic()
	{
		return CPlaneColliderRTTI::instance();
	}

	RTTITypeBase* CPlaneCollider::getRTTI() const
	{
		return CPlaneCollider::getRTTIStatic();
	}
}
