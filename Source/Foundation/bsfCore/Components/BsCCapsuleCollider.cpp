//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCCapsuleCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCCapsuleColliderRTTI.h"
#include "Scene/BsSceneManager.h"

using namespace bs;

CCapsuleCollider::CCapsuleCollider()
{
	SetName("CapsuleCollider");

	mLocalRotation = Quaternion::GetRotationFromTo(Vector3::UNIT_X, mNormal);
}

CCapsuleCollider::CCapsuleCollider(const HSceneObject& parent, float radius, float halfHeight)
	: CCollider(parent), mRadius(radius), mHalfHeight(halfHeight)
{
	SetName("CapsuleCollider");

	mLocalRotation = Quaternion::GetRotationFromTo(Vector3::UNIT_X, mNormal);
}

void CCapsuleCollider::SetNormal(const Vector3& normal)
{
	if(mNormal == normal)
		return;

	mNormal = bs::Vector3::Normalize(normal);
	mLocalRotation = Quaternion::GetRotationFromTo(Vector3::UNIT_X, mNormal);

	if(mInternal != nullptr)
		UpdateTransform();
}

void CCapsuleCollider::SetCenter(const Vector3& center)
{
	if(mLocalPosition == center)
		return;

	mLocalPosition = center;

	if(mInternal != nullptr)
		UpdateTransform();
}

void CCapsuleCollider::SetHalfHeight(float halfHeight)
{
	float clampedHalfHeight = std::max(halfHeight, 0.01f);
	if(mHalfHeight == clampedHalfHeight)
		return;

	mHalfHeight = clampedHalfHeight;

	if(mInternal != nullptr)
	{
		GetInternalInternal()->SetHalfHeight(clampedHalfHeight);

		if(mParent != nullptr)
			mParent->UpdateMassDistributionInternal();
	}
}

void CCapsuleCollider::SetRadius(float radius)
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

SPtr<Collider> CCapsuleCollider::CreateInternal()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	const Transform& tfrm = SO()->GetTransform();

	SPtr<Collider> collider = CapsuleCollider::Create(*scene->GetPhysicsScene(), mRadius, mHalfHeight, tfrm.GetPosition(), tfrm.GetRotation());

	collider->SetOwnerInternal(PhysicsOwnerType::Component, this);
	return collider;
}

RTTITypeBase* CCapsuleCollider::GetRttiStatic()
{
	return CCapsuleColliderRTTI::Instance();
}

RTTITypeBase* CCapsuleCollider::GetRtti() const
{
	return CCapsuleCollider::GetRttiStatic();
}
