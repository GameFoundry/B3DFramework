//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Physics/BsPhysics.h"
#include "Private/RTTI/BsCColliderRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace std::placeholders;

using namespace b3d;

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

		if(mRigidbody != nullptr)
			mRigidbody->UpdateMassDistribution();
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

	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	const SPtr<PhysicsScene>& physicsScene = scene->GetPhysicsScene();

	// Don't update the transform if it's due to Physics update since then we can guarantee it will remain at the same
	// relative transform to its parent
	if(physicsScene->IsUpdateInProgress())
		return;

	if((flags & (TCF_Parent | TCF_Transform)) != 0)
		UpdateTransform();
}

void CCollider::SetRigidbody(const HRigidbody& rigidbody)
{
	if(rigidbody == mRigidbody)
		return;

	if(mRigidbody != nullptr)
		mRigidbody->RemoveCollider(B3DStaticGameObjectCast<CCollider>(mThisHandle));

	if(mInternal != nullptr)
	{
		Rigidbody* rigidBodyPtr = nullptr;

		if(rigidbody != nullptr)
			rigidBodyPtr = rigidbody->GetInternal();

		mInternal->SetRigidbody(rigidBodyPtr);
	}

	if(rigidbody != nullptr)
		rigidbody->AddCollider(B3DStaticGameObjectCast<CCollider>(mThisHandle));

	mRigidbody = rigidbody;
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
	if(mRigidbody != nullptr)
		mRigidbody->RemoveCollider(B3DStaticGameObjectCast<CCollider>(mThisHandle));

	mRigidbody = nullptr;

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
	const Transform& transform = SO()->GetTransform();

	if(mRigidbody != nullptr)
	{
		const Transform& parentTransform = mRigidbody->SO()->GetTransform();
		const Vector3& parentPosition = parentTransform.GetPosition();
		const Quaternion& parentRotation = parentTransform.GetRotation();

		const Vector3& myPosition = transform.GetPosition();
		const Quaternion& myRotation = transform.GetRotation();

		Vector3 scale = parentTransform.GetScale();
		Vector3 inverseScale = scale;
		if(!Math::ApproxEquals(inverseScale.X,0.0f)) inverseScale.X = 1.0f / inverseScale.X;
		if(!Math::ApproxEquals(inverseScale.Y,0.0f)) inverseScale.Y = 1.0f / inverseScale.Y;
		if(!Math::ApproxEquals(inverseScale.Z,0.0f)) inverseScale.Z = 1.0f / inverseScale.Z;

		const Quaternion& inverseRotation = parentRotation.Inverse();

		const Vector3& relativePosition = inverseRotation.Rotate(myPosition - parentPosition) * inverseScale;
		const Quaternion& relativeRotation = inverseRotation * myRotation;

		if(mInternal)
			mInternal->SetTransform(relativePosition, relativeRotation);

		mRigidbody->UpdateMassDistribution();
	}
	else
	{
		if(mInternal)
			mInternal->SetTransform(transform.GetPosition(), transform.GetRotation());
	}

	if(mInternal)
		mInternal->SetScale(transform.GetScale());
}

void CCollider::UpdateCollisionReportMode()
{
	CollisionReportMode mode = mCollisionReportMode;

	if(mRigidbody != nullptr)
		mode = mRigidbody->GetCollisionReportMode();

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
