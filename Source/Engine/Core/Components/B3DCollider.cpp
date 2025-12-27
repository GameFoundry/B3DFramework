//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DCollider.h"
#include "Scene/B3DSceneObject.h"
#include "Components/B3DRigidbody.h"
#include "Math/B3DRay.h"
#include "Physics/B3DPhysics.h"
#include "RTTI/B3DColliderRTTI.h"
#include "Scene/B3DSceneInstance.h"


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

	for(auto& shape : mShapes)
		shape->SetMass(mass);

	if(mParentRigidbody != nullptr)
		mParentRigidbody->UpdateMassDistribution();
}

void Collider::SetMaterial(const HPhysicsMaterial& material)
{
	mMaterial = material;

	for(auto& shape : mShapes)
		shape->SetMaterial(material);
}

void Collider::SetContactOffset(float value)
{
	value = std::max(0.0f, std::max(value, GetRestOffset()));

	mContactOffset = value;

	for(auto& shape : mShapes)
		shape->SetContactOffset(value);
}

void Collider::SetRestOffset(float value)
{
	value = std::min(value, GetContactOffset());

	mRestOffset = value;

	for(auto& shape : mShapes)
		shape->SetRestOffset(value);
}

void Collider::SetLayer(u64 layer)
{
	mLayer = layer;

	for(auto& shape : mShapes)
		shape->SetLayer(layer);
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
	for(auto& shape : mShapes)
	{
		shape->SetParentCollider(this);
		shape->SetShapeIndexInParent(shapeIndex++);
		shape->SetIsTrigger(mIsTrigger);
		shape->SetContactOffset(mContactOffset);
		shape->SetRestOffset(mRestOffset);
		shape->SetLayer(mLayer);
		shape->SetMass(mMass);
		shape->SetScale(scale);
		// shape->UpdateTransform(); called implicitly by SetScale
	}

	B3D_ASSERT(mImplementation == nullptr);

	if(!mParentRigidbody.IsValid())
	{
		const Transform& transform = SO()->GetTransform();

		mImplementation = GetPhysics().CreateColliderImplementation();
		mImplementation->SetTransform(transform.GetPosition(), transform.GetRotation());

		for (const auto& shape : mShapes)
			mImplementation->AttachShape(shape);
	}
	else
	{
		// For rigidbodies we add/remove shapes when the collider is enabled/disabled. This is because we can't just add/remove
		// the rigidbody from the scene, as we can do with the internal collider implementation, as we own that one.
	}
}

void Collider::OnDestroyed()
{
	if(!mParentRigidbody.IsValid())
	{
		for (const auto& shape : mShapes)
		{
			if (shape == nullptr)
				continue;

			mImplementation->DetachShape(shape);
		}

		mImplementation = nullptr;
	}
	else
	{
		mParentRigidbody->RemoveCollider(B3DStaticGameObjectCast<Collider>(mThisHandle));
		mParentRigidbody = nullptr;
	}

	B3D_ASSERT(mImplementation == nullptr);
}

void Collider::OnEnabled()
{
	if(mParentRigidbody.IsValid())
	{
		for (const auto& shape : mShapes)
		{
			const RigidbodyFlags rigidbodyFlags = mParentRigidbody->GetFlags();
			shape->SetContinuousCollisionDetection(rigidbodyFlags.IsSet(RigidbodyFlag::CCD));

			mParentRigidbody->GetImplementation().AttachShape(shape);
		}
	}
	else
	{
		const SPtr<SceneInstance>& sceneInstance = SceneObject()->GetScene();
		mImplementation->AddToScene(*sceneInstance->GetPhysicsScene());
	}
}

void Collider::OnDisabled()
{
	if(mParentRigidbody.IsValid())
	{
		for (const auto& shape : mShapes)
		{
			mParentRigidbody->GetImplementation().DetachShape(shape);
			shape->SetContinuousCollisionDetection(false);

		}
	}
	else
		mImplementation->RemoveFromScene();
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

bool Collider::SetRigidbody(const HRigidbody& rigidbody)
{
	if(rigidbody == mParentRigidbody)
		return false;

	// Detach shapes from original body, destroy internal implementation if needed
	if(mParentRigidbody != nullptr)
		mParentRigidbody->RemoveCollider(B3DStaticGameObjectCast<Collider>(mThisHandle));

	if(mParentRigidbody.IsValid())
	{
		if(GetEnabled()) // If not enabled, shapes won't be part of the dynamic rigidbody
		{
			for(auto& shape : mShapes)
			{
				mParentRigidbody->GetImplementation().DetachShape(shape);

				shape->SetContinuousCollisionDetection(false);
			}
		}
	}
	else
	{
		for(auto& shape : mShapes)
			mImplementation->DetachShape(shape);

		if(GetEnabled()) // If not enabled, body won't be part of the scene
			mImplementation->RemoveFromScene();

		mImplementation = nullptr;
	}

	// Attach shapes to the new body, create internal implementation if needed
	if(rigidbody.IsValid())
	{
		if(GetEnabled())
		{
			for(auto& shape : mShapes)
			{
				const RigidbodyFlags rigidbodyFlags = rigidbody->GetFlags();
				shape->SetContinuousCollisionDetection(rigidbodyFlags.IsSet(RigidbodyFlag::CCD));

				rigidbody->GetImplementation().AttachShape(shape);
			}
		}
	}
	else
	{
		B3D_ASSERT(mImplementation == nullptr);

		const Transform& transform = SceneObject()->GetTransform();
		const SPtr<SceneInstance>& scene = SceneObject()->GetScene();

		mImplementation = GetPhysics().CreateColliderImplementation();
		mImplementation->SetTransform(transform.GetPosition(), transform.GetRotation());

		for(auto& shape : mShapes)
			mImplementation->AttachShape(shape);

		if(GetEnabled())
			mImplementation->AddToScene(*scene->GetPhysicsScene());
	}

	if(rigidbody != nullptr)
		rigidbody->AddCollider(B3DStaticGameObjectCast<Collider>(mThisHandle));

	mParentRigidbody = rigidbody;
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
		return SetRigidbody(HRigidbody());

	HSceneObject currentSO = SO();
	while(currentSO != nullptr)
	{
		HRigidbody parent = currentSO->GetComponent<Rigidbody>();
		if(parent != nullptr)
		{
			if(parent->GetEnabled() && IsValidParent(parent))
				return SetRigidbody(parent);
			else
				return SetRigidbody(HRigidbody());
		}

		currentSO = currentSO->GetParent();
	}

	// Not found
	return SetRigidbody(HRigidbody());
}

void Collider::UpdateTransform(bool updateShapeTransforms)
{
	const Transform& transform = SO()->GetTransform();

	if(mParentRigidbody != nullptr)
	{
		const Transform& parentTransform = mParentRigidbody->SO()->GetTransform();
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

		mParentRigidbody->UpdateMassDistribution();
	}
	else
	{
		mAdjustedPosition = transform.GetPosition();
		mAdjustedRotation = transform.GetRotation();

		// This can be null only when updating transform during creation (at that point we don't know if the parent is a rigidbody and if we need the internal collider implementation)
		if(mImplementation != nullptr)
			mImplementation->SetTransform(transform.GetPosition(), transform.GetRotation());
	}

	if(updateShapeTransforms)
	{
		for(auto& shape : mShapes)
			shape->UpdateTransform();
	}
}

void Collider::UpdateCollisionReportMode()
{
	CollisionReportMode mode = mCollisionReportMode;

	if(mParentRigidbody != nullptr)
		mode = mParentRigidbody->GetCollisionReportMode();

	for(auto& shape : mShapes)
		shape->SetCollisionReportMode(mode);
}

RTTIType* Collider::GetRttiStatic()
{
	return ColliderRTTI::Instance();
}

RTTIType* Collider::GetRtti() const
{
	return Collider::GetRttiStatic();
}
