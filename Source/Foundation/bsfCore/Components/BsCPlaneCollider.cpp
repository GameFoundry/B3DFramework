//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCPlaneCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCPlaneColliderRTTI.h"
#include "Scene/BsSceneManager.h"

using namespace bs;

CPlaneCollider::CPlaneCollider()
{
	SetName("PlaneCollider");

	mLocalRotation = Quaternion::GetRotationFromTo(Vector3::UNIT_X, mNormal);
}

CPlaneCollider::CPlaneCollider(const HSceneObject& parent)
	: CCollider(parent)
{
	SetName("PlaneCollider");

	mLocalRotation = Quaternion::GetRotationFromTo(Vector3::UNIT_X, mNormal);
}

void CPlaneCollider::SetNormal(const Vector3& normal)
{
	if(mNormal == normal)
		return;

	mNormal = normal;
	mNormal.Normalize();

	mLocalRotation = Quaternion::GetRotationFromTo(Vector3::UNIT_X, normal);
	mLocalPosition = mNormal * mDistance;

	if(mInternal != nullptr)
		UpdateTransform();
}

void CPlaneCollider::SetDistance(float distance)
{
	if(mDistance == distance)
		return;

	mDistance = distance;
	mLocalPosition = mNormal * distance;

	if(mInternal != nullptr)
		UpdateTransform();
}

SPtr<Collider> CPlaneCollider::CreateInternal()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	const Transform& tfrm = SO()->GetTransform();

	SPtr<Collider> collider = PlaneCollider::Create(*scene->GetPhysicsScene(), tfrm.GetPosition(), tfrm.GetRotation());

	collider->SetOwnerInternal(PhysicsOwnerType::Component, this);
	return collider;
}

bool CPlaneCollider::IsValidParent(const HRigidbody& parent) const
{
	// Planes cannot be added to non-kinematic rigidbodies
	return parent->GetIsKinematic();
}

RTTITypeBase* CPlaneCollider::GetRttiStatic()
{
	return CPlaneColliderRTTI::Instance();
}

RTTITypeBase* CPlaneCollider::GetRtti() const
{
	return CPlaneCollider::GetRttiStatic();
}
