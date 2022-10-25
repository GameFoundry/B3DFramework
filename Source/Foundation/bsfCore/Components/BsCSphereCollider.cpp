//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCSphereCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCSphereColliderRTTI.h"
#include "Scene/BsSceneManager.h"

using namespace bs;

CSphereCollider::CSphereCollider()
{
	SetName("SphereCollider");
}

CSphereCollider::CSphereCollider(const HSceneObject& parent, float radius)
	: CCollider(parent), mRadius(radius)
{
	SetName("SphereCollider");
}

void CSphereCollider::SetRadius(float radius)
{
	float clampedRadius = std::max(radius, 0.01f);
	if(mRadius == clampedRadius)
		return;

	mRadius = clampedRadius;

	if(mInternal != nullptr)
	{
		GetInternalInternal()->SetRadius(clampedRadius);

		if(mParent != nullptr)
			mParent->UpdateMassDistributionInternal();
	}
}

void CSphereCollider::SetCenter(const Vector3& center)
{
	if(mLocalPosition == center)
		return;

	mLocalPosition = center;

	if(mInternal != nullptr)
		UpdateTransform();
}

SPtr<Collider> CSphereCollider::CreateInternal()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	const Transform& tfrm = SO()->GetTransform();

	SPtr<Collider> collider = SphereCollider::Create(*scene->GetPhysicsScene(), mRadius, tfrm.GetPosition(), tfrm.GetRotation());

	collider->SetOwnerInternal(PhysicsOwnerType::Component, this);
	return collider;
}

RTTITypeBase* CSphereCollider::GetRttiStatic()
{
	return CSphereColliderRTTI::Instance();
}

RTTITypeBase* CSphereCollider::GetRtti() const
{
	return CSphereCollider::GetRttiStatic();
}
