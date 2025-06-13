//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Physics/BsPhysics.h"
#include "Private/RTTI/BsCColliderRTTI.h"

using namespace std::placeholders;

using namespace bs;

CCollider::CCollider()
{
	SetName("Collider");

	mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
}

CCollider::CCollider(const HSceneObject& parent)
	: Component(parent)
{
	SetName("Collider");

	mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
}

void CCollider::SetIsTrigger(bool value)
{
	if(mIsTrigger == value)
		return;

	mIsTrigger = value;

	if(mInternal != nullptr)
	{
		mInternal->SetIsTrigger(value);

		UpdateParentRigidbody();
		UpdateTransform();
	}
}

void CCollider::SetMass(float mass)
{
	if(mMass == mass)
		return;

	mMass = mass;

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();

		for(auto& entry : shapes)
			entry->SetMass(mass);

		if(mParent != nullptr)
			mParent->UpdateMassDistribution();
	}
}

void CCollider::SetMaterial(const HPhysicsMaterial& material)
{
	mMaterial = material;

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();

		for(auto& entry : shapes)
			entry->SetMaterial(material);
	}
}

void CCollider::SetContactOffset(float value)
{
	value = std::max(0.0f, std::max(value, GetRestOffset()));

	mContactOffset = value;

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();

		for(auto& entry : shapes)
			entry->SetContactOffset(value);
	}
}

void CCollider::SetRestOffset(float value)
{
	value = std::min(value, GetContactOffset());

	mRestOffset = value;

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();

		for(auto& entry : shapes)
			entry->SetRestOffset(value);
	}
}

void CCollider::SetLayer(u64 layer)
{
	mLayer = layer;

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();

		for(auto& entry : shapes)
			entry->SetLayer(layer);
	}
}

void CCollider::SetCollisionReportMode(CollisionReportMode mode)
{
	mCollisionReportMode = mode;

	if(mInternal != nullptr)
		UpdateCollisionReportMode();
}

void CCollider::OnBeginPlay()
{
}

void CCollider::OnDestroyed()
{
	DestroyInternal();
}

void CCollider::OnDisabled()
{
	DestroyInternal();
}

void CCollider::OnEnabled()
{
	RestoreInternal();
}

void CCollider::OnTransformChanged(TransformChangedFlags flags)
{
	if(!GetEnabled())
		return;

	if((flags & TCF_Parent) != 0)
		UpdateParentRigidbody();

	// Don't update the transform if it's due to Physics update since then we can guarantee it will remain at the same
	// relative transform to its parent
	if(GetPhysics().IsUpdateInProgress())
		return;

	if((flags & (TCF_Parent | TCF_Transform)) != 0)
		UpdateTransform();
}

void CCollider::SetRigidbody(const HRigidbody& rigidbody)
{
	if(rigidbody == mParent)
		return;

	if(mParent != nullptr)
		mParent->RemoveCollider(B3DStaticGameObjectCast<CCollider>(mThisHandle));

	if(mInternal != nullptr)
	{
		Rigidbody* rigidBodyPtr = nullptr;

		if(rigidbody != nullptr)
			rigidBodyPtr = rigidbody->GetInternal();

		mInternal->SetRigidbody(rigidBodyPtr);
	}

	if(rigidbody != nullptr)
		rigidbody->AddCollider(B3DStaticGameObjectCast<CCollider>(mThisHandle));

	mParent = rigidbody;
	UpdateCollisionReportMode();
	UpdateTransform();
}

bool CCollider::RayCast(const Ray& ray, PhysicsQueryHit& hit, float maxDist) const
{
	if(mInternal == nullptr)
		return false;

	return mInternal->RayCast(ray, hit, maxDist);
}

bool CCollider::RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, float maxDist) const
{
	if(mInternal == nullptr)
		return false;

	return mInternal->RayCast(origin, unitDir, hit, maxDist);
}

void CCollider::RestoreInternal()
{
	if(mInternal == nullptr)
	{
		mInternal = CreateInternal();

		mInternal->OnCollisionBegin.Connect(std::bind(&CCollider::TriggerOnCollisionBegin, this, _1));
		mInternal->OnCollisionStay.Connect(std::bind(&CCollider::TriggerOnCollisionStay, this, _1));
		mInternal->OnCollisionEnd.Connect(std::bind(&CCollider::TriggerOnCollisionEnd, this, _1));
	}

	// Note: Merge into one call to avoid many virtual function calls
	mInternal->SetIsTrigger(mIsTrigger);

	TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();

	for(auto& entry : shapes)
	{
		entry->SetMass(mMass);
		entry->SetMaterial(mMaterial);
		entry->SetContactOffset(mContactOffset);
		entry->SetRestOffset(mRestOffset);
		entry->SetLayer(mLayer);
	}

	UpdateParentRigidbody();
	UpdateTransform();
	UpdateCollisionReportMode();
}

void CCollider::DestroyInternal()
{
	if(mParent != nullptr)
		mParent->RemoveCollider(B3DStaticGameObjectCast<CCollider>(mThisHandle));

	mParent = nullptr;

	// This should release the last reference and destroy the internal collider
	if(mInternal)
	{
		mInternal->SetOwner(PhysicsOwnerType::None, nullptr);
		mInternal = nullptr;
	}
}

void CCollider::UpdateParentRigidbody()
{
	if(mIsTrigger)
	{
		SetRigidbody(HRigidbody());
		return;
	}

	HSceneObject currentSO = SO();
	while(currentSO != nullptr)
	{
		HRigidbody parent = currentSO->GetComponent<CRigidbody>();
		if(parent != nullptr)
		{
			if(parent->GetEnabled() && IsValidParent(parent))
				SetRigidbody(parent);
			else
				SetRigidbody(HRigidbody());

			return;
		}

		currentSO = currentSO->GetParent();
	}

	// Not found
	SetRigidbody(HRigidbody());
}

void CCollider::UpdateTransform()
{
	const Transform& tfrm = SO()->GetTransform();
	Vector3 myScale = tfrm.GetScale();

	if(mParent != nullptr)
	{
		const Transform& parentTfrm = mParent->SO()->GetTransform();
		Vector3 parentPos = parentTfrm.GetPosition();
		Quaternion parentRot = parentTfrm.GetRotation();

		Vector3 myPos = tfrm.GetPosition();
		Quaternion myRot = tfrm.GetRotation();

		Vector3 scale = parentTfrm.GetScale();
		Vector3 invScale = scale;
		if(invScale.X != 0) invScale.X = 1.0f / invScale.X;
		if(invScale.Y != 0) invScale.Y = 1.0f / invScale.Y;
		if(invScale.Z != 0) invScale.Z = 1.0f / invScale.Z;

		Quaternion invRotation = parentRot.Inverse();

		Vector3 relativePos = invRotation.Rotate(myPos - parentPos) * invScale;
		Quaternion relativeRot = invRotation * myRot;

		relativePos = relativePos + relativeRot.Rotate(mLocalPosition * scale);
		relativeRot = relativeRot * mLocalRotation;

		if(mInternal)
			mInternal->SetTransform(relativePos, relativeRot);

		mParent->UpdateMassDistribution();
	}
	else
	{
		Quaternion myRot = tfrm.GetRotation();
		Vector3 myPos = tfrm.GetPosition() + myRot.Rotate(mLocalPosition * myScale);
		myRot = myRot * mLocalRotation;

		if(mInternal)
			mInternal->SetTransform(myPos, myRot);
	}

	if(mInternal)
		mInternal->SetScale(myScale);
}

void CCollider::UpdateCollisionReportMode()
{
	CollisionReportMode mode = mCollisionReportMode;

	if(mParent != nullptr)
		mode = mParent->GetCollisionReportMode();

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();

		for(auto& entry : shapes)
			entry->SetCollisionReportMode(mode);
	}
}

void CCollider::TriggerOnCollisionBegin(const CollisionDataRaw& data)
{
	OnCollisionBegin(PopulateCollisionData(data));
}

void CCollider::TriggerOnCollisionStay(const CollisionDataRaw& data)
{
	OnCollisionStay(PopulateCollisionData(data));
}

void CCollider::TriggerOnCollisionEnd(const CollisionDataRaw& data)
{
	OnCollisionEnd(PopulateCollisionData(data));
}

CollisionData CCollider::PopulateCollisionData(const CollisionDataRaw& data)
{
	CollisionData hit;
	hit.ContactPoints = data.ContactPoints;
	hit.Collider[0] = B3DStaticGameObjectCast<CCollider>(mThisHandle);

	ColliderShape* const myColliderShape = data.ColliderShapes[0];
	if(myColliderShape != nullptr)
	{
		Collider* const myCollider = myColliderShape->GetCollider();
		if(B3D_ENSURE(myCollider != nullptr))
			hit.ColliderShapes[0] = myCollider->GetShapes()[myColliderShape->GetShapeIndexInParent()];
	}

	ColliderShape* const otherColliderShape = data.ColliderShapes[1];
	if(otherColliderShape != nullptr)
	{
		Collider* const otherCollider = otherColliderShape->GetCollider();
		if(B3D_ENSURE(otherCollider != nullptr))
		{
			CCollider* other = (CCollider*)otherCollider->GetOwner(PhysicsOwnerType::Component);
			hit.Collider[1] = B3DStaticGameObjectCast<CCollider>(other->GetHandle());
			hit.ColliderShapes[1] = otherCollider->GetShapes()[otherColliderShape->GetShapeIndexInParent()];
		}
	}

	return hit;
}

RTTIType* CCollider::GetRttiStatic()
{
	return CColliderRTTI::Instance();
}

RTTIType* CCollider::GetRtti() const
{
	return CCollider::GetRttiStatic();
}
