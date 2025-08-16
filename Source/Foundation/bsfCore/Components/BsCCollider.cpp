//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Math/BsRay.h"
#include "Physics/BsPhysics.h"
#include "Private/RTTI/BsCColliderRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace std::placeholders;

using namespace b3d;

Collider::Collider(const HSceneObject& parent)
	: Component(parent)
{
	SetName("Collider");

	mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform | TCF_NotifyStopped);
}

Collider::Collider()
	:Collider(nullptr)
{ }

void Collider::SetIsTrigger(bool value)
{
	if(mIsTrigger == value)
		return;

	mIsTrigger = value;

	for(auto& shape : mShapes)
		shape->SetIsTrigger(value);

	// Triggers don't support parent rigidbody, so refresh
	RefreshParentRigidbody();
}

void Collider::SetMass(float mass)
{
	if(mMass == mass)
		return;

	mMass = mass;

	for(auto& entry : mShapes)
		entry->SetMass(mass);

	if(mParentDynamicRigidbody != nullptr)
		mParentDynamicRigidbody->UpdateMassDistribution();
}

void Collider::SetMaterial(const HPhysicsMaterial& material)
{
	mMaterial = material;

	for(auto& entry : mShapes)
		entry->SetMaterial(material);
}

void Collider::SetContactOffset(float value)
{
	value = std::max(0.0f, std::max(value, GetRestOffset()));

	mContactOffset = value;

	for(auto& entry : mShapes)
		entry->SetContactOffset(value);
}

void Collider::SetRestOffset(float value)
{
	value = std::min(value, GetContactOffset());

	mRestOffset = value;

	for(auto& entry : mShapes)
		entry->SetRestOffset(value);
}

void Collider::SetLayer(u64 layer)
{
	mLayer = layer;

	for(auto& entry : mShapes)
		entry->SetLayer(layer);
}

void Collider::SetCollisionReportMode(CollisionReportMode mode)
{
	mCollisionReportMode = mode;

	UpdateCollisionReportMode();
}

void Collider::OnCreated()
{
	const Vector3& scale = SceneObject()->GetTransform().GetScale();

	if(!RefreshParentRigidbody())
	{
		const bool updateShapeTransforms = false; // No need as we update them below. Avoid double work.
		UpdateTransform(updateShapeTransforms); // Perform initial transform update, unless the parent rigidbody transform already did it. This needs to happen before shape UpdateTransform below
	}

	u32 shapeIndex = 0;
	for(auto& entry : mShapes)
	{
		entry->SetParentCollider(this);
		entry->SetShapeIndexInParent(shapeIndex++);
		entry->SetIsTrigger(mIsTrigger);
		entry->SetScale(scale);
		// entry->UpdateTransform(); called implicitly by SetScale
	}

	B3D_ASSERT(mStaticRigidbody == nullptr);

	Rigidbody* const dynamicRigidbody = mParentDynamicRigidbody.IsValid() ? mParentDynamicRigidbody->GetInternal() : nullptr;
	if(dynamicRigidbody == nullptr)
	{
		const Transform& transform = SO()->GetTransform();

		mStaticRigidbody = GetPhysics().CreateStaticRigidbody();
		mStaticRigidbody->SetTransform(transform.GetPosition(), transform.GetRotation());

		for (const auto& shape : mShapes)
			mStaticRigidbody->AttachShape(shape);
	}
	else
	{
		// For dynamic rigidbodies we add/remove shapes when the collider is enabled/disabled. This is because we can't just add/remove
		// the rigidbody from the scene, as we can do with a static one, as we own that one.
	}
}

void Collider::OnDestroyed()
{
	Rigidbody* const dynamicRigidbody = mParentDynamicRigidbody.IsValid() ? mParentDynamicRigidbody->GetInternal() : nullptr;
	if(dynamicRigidbody == nullptr)
	{
		for (const auto& existingShape : mShapes)
		{
			if (existingShape == nullptr)
				continue;

			mStaticRigidbody->DetachShape(existingShape);
		}

		mStaticRigidbody->RemoveFromScene();
		mStaticRigidbody = nullptr;
	}
	else
	{
		mParentDynamicRigidbody->RemoveCollider(B3DStaticGameObjectCast<Collider>(mThisHandle));
		mParentDynamicRigidbody = nullptr;
	}

	B3D_ASSERT(mStaticRigidbody == nullptr);
}

void Collider::OnEnabled()
{
	Rigidbody* const dynamicRigidbody = mParentDynamicRigidbody.IsValid() ? mParentDynamicRigidbody->GetInternal() : nullptr;
	if(dynamicRigidbody != nullptr)
	{
		for (const auto& shape : mShapes)
			dynamicRigidbody->AttachShape(shape);
	}
	else
	{
		const SPtr<SceneInstance>& sceneInstance = SceneObject()->GetScene();
		mStaticRigidbody->AddToScene(*sceneInstance->GetPhysicsScene());
	}
}

void Collider::OnDisabled()
{
	Rigidbody* const dynamicRigidbody = mParentDynamicRigidbody.IsValid() ? mParentDynamicRigidbody->GetInternal() : nullptr;
	if(dynamicRigidbody != nullptr)
	{
		for (const auto& existingShape : mShapes)
		{
			if (existingShape == nullptr)
				continue;

			dynamicRigidbody->DetachShape(existingShape);
		}
	}
	else
		mStaticRigidbody->RemoveFromScene();
}

void Collider::OnTransformChanged(TransformChangedFlags flags)
{
	if(!GetEnabled())
		return;

	if((flags & TCF_Parent) != 0)
		RefreshParentRigidbody();

	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	const SPtr<PhysicsScene>& physicsScene = scene->GetPhysicsScene();

	// Don't update the transform if it's due to Physics update since then we can guarantee it will remain at the same
	// relative transform to its parent
	if(physicsScene->IsUpdateInProgress())
		return;

	if((flags & (TCF_Parent | TCF_Transform)) != 0)
		UpdateTransform();
}

bool Collider::SetDynamicRigidbody(const HRigidbody& rigidbody)
{
	if(rigidbody == mParentDynamicRigidbody)
		return false;

	// Detach shapes from original body, destroy static body if needed
	if(mParentDynamicRigidbody != nullptr)
		mParentDynamicRigidbody->RemoveCollider(B3DStaticGameObjectCast<Collider>(mThisHandle));

	Rigidbody* const originalDynamicRigidbody = mParentDynamicRigidbody.IsValid() ? mParentDynamicRigidbody->GetInternal() : nullptr;
	if(originalDynamicRigidbody != nullptr)
	{
		if(GetEnabled()) // If not enabled, shapes won't be part of the dynamic rigidbody
		{
			for(auto& entry : mShapes)
				originalDynamicRigidbody->DetachShape(entry);
		}
	}
	else
	{
		for(auto& entry : mShapes)
			mStaticRigidbody->DetachShape(entry);

		if(GetEnabled()) // If not enabled, body won't be part of the scene
			mStaticRigidbody->RemoveFromScene();

		mStaticRigidbody = nullptr;
	}

	// Attach shapes to the new body, create static body if needed
	Rigidbody* const newDynamicRigidbody = rigidbody.IsValid() ? rigidbody->GetInternal() : nullptr;
	if(newDynamicRigidbody)
	{
		if(GetEnabled())
		{
			for(auto& entry : mShapes)
				newDynamicRigidbody->AttachShape(entry);
		}
	}
	else
	{
		B3D_ASSERT(mStaticRigidbody == nullptr);

		const Transform& transform = SceneObject()->GetTransform();
		const SPtr<SceneInstance>& scene = SceneObject()->GetScene();

		mStaticRigidbody = GetPhysics().CreateStaticRigidbody();
		mStaticRigidbody->SetTransform(transform.GetPosition(), transform.GetRotation());

		for(auto& entry : mShapes)
			mStaticRigidbody->AttachShape(entry);

		if(GetEnabled())
			mStaticRigidbody->AddToScene(*scene->GetPhysicsScene());
	}

	if(rigidbody != nullptr)
		rigidbody->AddCollider(B3DStaticGameObjectCast<Collider>(mThisHandle));

	mParentDynamicRigidbody = rigidbody;
	UpdateCollisionReportMode();
	UpdateTransform();

	return true;
}

bool Collider::RayCast(const Ray& ray, PhysicsQueryHit& outHit, float maximumDistance) const
{
	return GetPhysics().RayCast(ray.Origin, ray.Direction, *this, outHit, maximumDistance);
}

bool Collider::RayCast(const Vector3& origin, const Vector3& direction, PhysicsQueryHit& outHit, float maximumDistance) const
{
	return GetPhysics().RayCast(origin, direction, *this, outHit, maximumDistance);
}

bool Collider::RefreshParentRigidbody()
{
	if(mIsTrigger)
		return SetDynamicRigidbody(HRigidbody());

	HSceneObject currentSO = SO();
	while(currentSO != nullptr)
	{
		HRigidbody parent = currentSO->GetComponent<CRigidbody>();
		if(parent != nullptr)
		{
			if(parent->GetEnabled() && IsValidParent(parent))
				return SetDynamicRigidbody(parent);
			else
				return SetDynamicRigidbody(HRigidbody());
		}

		currentSO = currentSO->GetParent();
	}

	// Not found
	return SetDynamicRigidbody(HRigidbody());
}

void Collider::UpdateTransform(bool updateShapeTransforms)
{
	const Transform& transform = SO()->GetTransform();

	if(mParentDynamicRigidbody != nullptr)
	{
		const Transform& parentTransform = mParentDynamicRigidbody->SO()->GetTransform();
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

		mAdjustedPosition = inverseRotation.Rotate(myPosition - parentPosition) * inverseScale;
		mAdjustedRotation = inverseRotation * myRotation;

		mParentDynamicRigidbody->UpdateMassDistribution();
	}
	else
	{
		mAdjustedPosition = transform.GetPosition();
		mAdjustedRotation = transform.GetRotation();

		// This can be null only when updating transform during creation (at that point we don't know if the parent is a dynamic rigidbody and if we need a static body)
		if(mStaticRigidbody != nullptr)
			mStaticRigidbody->SetTransform(transform.GetPosition(), transform.GetRotation());
	}

	if(updateShapeTransforms)
	{
		for(auto& entry : mShapes)
			entry->UpdateTransform();
	}
}

void Collider::UpdateCollisionReportMode()
{
	CollisionReportMode mode = mCollisionReportMode;

	if(mParentDynamicRigidbody != nullptr)
		mode = mParentDynamicRigidbody->GetCollisionReportMode();

	for(auto& entry : mShapes)
		entry->SetCollisionReportMode(mode);
}

CollisionData Collider::PopulateCollisionData(const CollisionDataRaw& data)
{
	CollisionData hit;
	hit.ContactPoints = data.ContactPoints;
	hit.Collider[0] = B3DStaticGameObjectCast<Collider>(mThisHandle);

	ColliderShape* const myColliderShape = data.ColliderShapes[0];
	if(myColliderShape != nullptr)
	{
		Collider* const myCollider = myColliderShape->GetParentCollider();
		if(B3D_ENSURE(myCollider != nullptr))
			hit.ColliderShapes[0] = myCollider->GetShapes()[myColliderShape->GetShapeIndexInParent()];
	}

	ColliderShape* const otherColliderShape = data.ColliderShapes[1];
	if(otherColliderShape != nullptr)
	{
		Collider* const otherCollider = otherColliderShape->GetParentCollider();
		if(B3D_ENSURE(otherCollider != nullptr))
		{
			hit.Collider[1] = B3DStaticGameObjectCast<Collider>(otherCollider->GetHandle());
			hit.ColliderShapes[1] = otherCollider->GetShapes()[otherColliderShape->GetShapeIndexInParent()];
		}
	}

	return hit;
}

RTTIType* Collider::GetRttiStatic()
{
	return ColliderRTTI::Instance();
}

RTTIType* Collider::GetRtti() const
{
	return Collider::GetRttiStatic();
}
