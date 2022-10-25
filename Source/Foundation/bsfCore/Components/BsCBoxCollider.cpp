//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCBoxCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCBoxColliderRTTI.h"
#include "Scene/BsSceneManager.h"

using namespace bs;

CBoxCollider::CBoxCollider()
{
	SetName("BoxCollider");
}

CBoxCollider::CBoxCollider(const HSceneObject& parent, const Vector3& extents)
	: CCollider(parent), mExtents(extents)
{
	SetName("BoxCollider");
}

void CBoxCollider::SetExtents(const Vector3& extents)
{
	Vector3 clampedExtents = Vector3::Max(extents, Vector3(0.01f, 0.01f, 0.01f));

	if(mExtents == clampedExtents)
		return;

	mExtents = clampedExtents;

	if(mInternal != nullptr)
	{
		GetInternalInternal()->SetExtents(clampedExtents);

		if(mParent != nullptr)
			mParent->UpdateMassDistributionInternal();
	}
}

void CBoxCollider::SetCenter(const Vector3& center)
{
	if(mLocalPosition == center)
		return;

	mLocalPosition = center;

	if(mInternal != nullptr)
		UpdateTransform();
}

SPtr<Collider> CBoxCollider::CreateInternal()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	const Transform& tfrm = SO()->GetTransform();

	SPtr<Collider> collider = BoxCollider::Create(*scene->GetPhysicsScene(), mExtents, tfrm.GetPosition(), tfrm.GetRotation());
	collider->SetOwnerInternal(PhysicsOwnerType::Component, this);

	return collider;
}

RTTITypeBase* CBoxCollider::GetRttiStatic()
{
	return CBoxColliderRTTI::Instance();
}

RTTITypeBase* CBoxCollider::GetRtti() const
{
	return CBoxCollider::GetRttiStatic();
}
