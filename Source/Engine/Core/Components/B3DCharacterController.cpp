//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DCharacterController.h"
#include "Scene/B3DSceneObject.h"
#include "Scene/B3DSceneInstance.h"
#include "RTTI/B3DCharacterControllerRTTI.h"
#include "B3DCollider.h"
#include "Physics/B3DPhysics.h"

using namespace b3d;

CharacterController::CharacterController(const HSceneObject& parent)
	: Component(parent)
{
	SetName("CharacterController");

	mNotifyFlags = TCF_Transform;
}

CharacterController::CharacterController()
	: CharacterController(nullptr)
{ }

CharacterCollisionFlags CharacterController::Move(const Vector3& displacement)
{
	CharacterCollisionFlags outFlags;

	if(mImplementation == nullptr)
		return outFlags;

	outFlags = mImplementation->Move(displacement);
	UpdateSceneObjectPositionFromController();

	return outFlags;
}

Vector3 CharacterController::GetFootPosition() const
{
	if(mImplementation == nullptr)
		return Vector3::kZero;

	return mImplementation->GetFootPosition();
}

void CharacterController::SetFootPosition(const Vector3& position)
{
	if(mImplementation == nullptr)
		return;

	mImplementation->SetFootPosition(position);
	UpdateSceneObjectPositionFromController();
}

void CharacterController::SetRadius(float radius)
{
	mInformation.Radius = radius;

	if(mImplementation != nullptr)
		UpdateDimensions();
}

void CharacterController::SetHeight(float height)
{
	mInformation.Height = height;

	if(mImplementation != nullptr)
		UpdateDimensions();
}

void CharacterController::SetUp(const Vector3& up)
{
	mInformation.Up = up;

	if(mImplementation != nullptr)
		mImplementation->SetUp(up);
}

void CharacterController::SetClimbingMode(CharacterClimbingMode mode)
{
	mInformation.ClimbingMode = mode;

	if(mImplementation != nullptr)
		mImplementation->SetClimbingMode(mode);
}

void CharacterController::SetNonWalkableMode(CharacterNonWalkableMode mode)
{
	mInformation.NonWalkableMode = mode;

	if(mImplementation != nullptr)
		mImplementation->SetNonWalkableMode(mode);
}

void CharacterController::SetMinMoveDistance(float value)
{
	mInformation.MinMoveDistance = value;

	if(mImplementation != nullptr)
		mImplementation->SetMinMoveDistance(value);
}

void CharacterController::SetContactOffset(float value)
{
	mInformation.ContactOffset = value;

	if(mImplementation != nullptr)
		mImplementation->SetContactOffset(value);
}

void CharacterController::SetStepOffset(float value)
{
	mInformation.StepOffset = value;

	if(mImplementation != nullptr)
		mImplementation->SetStepOffset(value);
}

void CharacterController::SetSlopeLimit(Radian value)
{
	mInformation.SlopeLimit = value;

	if(mImplementation != nullptr)
		mImplementation->SetSlopeLimit(value);
}

void CharacterController::OnDestroyed()
{
	mImplementation = nullptr;
}

void CharacterController::OnDisabled()
{
	mImplementation = nullptr;
}

void CharacterController::OnEnabled()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();

	mInformation.Position = SO()->GetTransform().GetPosition();
	mImplementation = scene->GetPhysicsScene()->CreateCharacterController(*this, mInformation);

	UpdateDimensions();
}

void CharacterController::OnTransformChanged(TransformChangedFlags flags)
{
	if(mImplementation == nullptr)
		return;

	mImplementation->SetPosition(SO()->GetTransform().GetPosition());
}

void CharacterController::UpdateSceneObjectPositionFromController()
{
	mNotifyFlags = (TransformChangedFlags)0;
	SO()->SetWorldPosition(mImplementation->GetPosition());
	mNotifyFlags = TCF_Transform;
}

void CharacterController::UpdateDimensions()
{
	const Vector3 scale = SO()->GetTransform().GetScale();
	const float height = mInformation.Height * Math::Abs(scale.Y);
	const float radius = mInformation.Radius * Math::Abs(std::max(scale.X, scale.Z));

	mImplementation->SetHeight(height);
	mImplementation->SetRadius(radius);
}

void CharacterController::TriggerOnColliderHit(const ControllerColliderCollision& value)
{
	// Const-cast and modify is okay because we're the only object receiving this event
	auto& hit = const_cast<ControllerColliderCollision&>(value);

	if(hit.ColliderShape)
	{
		Collider* const collider = hit.ColliderShape->GetParentCollider();
		hit.Collider = B3DStaticGameObjectCast<Collider>(collider->GetHandle());
	}

	OnColliderHit(hit);
}

void CharacterController::TriggerOnControllerHit(const ControllerControllerCollision& value)
{
	// Const-cast and modify is okay because we're the only object receiving this event
	ControllerControllerCollision& hit = const_cast<ControllerControllerCollision&>(value);

	if(hit.ControllerRaw)
	{
		const CharacterController* const controller = hit.ControllerRaw;
		hit.Controller = B3DStaticGameObjectCast<CharacterController>(controller->GetHandle());
	}

	OnControllerHit(hit);
}

RTTIType* CharacterController::GetRttiStatic()
{
	return CharacterControllerRTTI::Instance();
}

RTTIType* CharacterController::GetRtti() const
{
	return CharacterController::GetRttiStatic();
}
