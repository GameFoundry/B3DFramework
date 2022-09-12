//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCCharacterController.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsSceneManager.h"
#include "Physics/BsCollider.h"
#include "Private/RTTI/BsCCharacterControllerRTTI.h"
#include "BsCCollider.h"

using namespace std::placeholders;

namespace bs
{
	CCharacterController::CCharacterController()
	{
		setName("CharacterController");

		mNotifyFlags = TCF_Transform;
	}

	CCharacterController::CCharacterController(const HSceneObject& parent)
		: Component(parent)
	{
		setName("CharacterController");

		mNotifyFlags = TCF_Transform;
	}

	CharacterCollisionFlags CCharacterController::Move(const Vector3& displacement)
	{
		CharacterCollisionFlags output;

		if (mInternal == nullptr)
			return output;

		output = mInternal->move(displacement);
		updatePositionFromController();

		return output;
	}

	Vector3 CCharacterController::GetFootPosition() const
	{
		if (mInternal == nullptr)
			return Vector3::ZERO;

		return mInternal->getFootPosition();
	}

	void CCharacterController::SetFootPosition(const Vector3& position)
	{
		if (mInternal == nullptr)
			return;

		mInternal->setFootPosition(position);
		updatePositionFromController();
	}

	void CCharacterController::SetRadius(float radius)
	{
		mDesc.radius = radius;

		if (mInternal != nullptr)
			updateDimensions();
	}

	void CCharacterController::SetHeight(float height)
	{
		mDesc.height = height;

		if (mInternal != nullptr)
			updateDimensions();
	}

	void CCharacterController::SetUp(const Vector3& up)
	{
		mDesc.up = up;

		if (mInternal != nullptr)
			mInternal->setUp(up);
	}

	void CCharacterController::SetClimbingMode(CharacterClimbingMode mode)
	{
		mDesc.climbingMode = mode;

		if (mInternal != nullptr)
			mInternal->setClimbingMode(mode);
	}

	void CCharacterController::SetNonWalkableMode(CharacterNonWalkableMode mode)
	{
		mDesc.nonWalkableMode = mode;

		if (mInternal != nullptr)
			mInternal->setNonWalkableMode(mode);
	}

	void CCharacterController::SetMinMoveDistance(float value)
	{
		mDesc.minMoveDistance = value;

		if (mInternal != nullptr)
			mInternal->setMinMoveDistance(value);
	}

	void CCharacterController::SetContactOffset(float value)
	{
		mDesc.contactOffset = value;

		if (mInternal != nullptr)
			mInternal->setContactOffset(value);
	}

	void CCharacterController::SetStepOffset(float value)
	{
		mDesc.stepOffset = value;

		if (mInternal != nullptr)
			mInternal->setStepOffset(value);
	}

	void CCharacterController::SetSlopeLimit(Radian value)
	{
		mDesc.slopeLimit = value;

		if (mInternal != nullptr)
			mInternal->setSlopeLimit(value);
	}

	void CCharacterController::SetLayer(UINT64 layer)
	{
		mLayer = layer;

		if (mInternal != nullptr)
			mInternal->setLayer(layer);
	}

	void CCharacterController::OnInitialized()
	{

	}

	void CCharacterController::OnDestroyed()
	{
		destroyInternal();
	}

	void CCharacterController::OnDisabled()
	{
		destroyInternal();
	}

	void CCharacterController::OnEnabled()
	{
		const SPtr<SceneInstance>& scene = SO()->getScene();

		mDesc.position = SO()->getTransform().getPosition();
		mInternal = CharacterController::create(*scene->getPhysicsScene(), mDesc);
		mInternal->_setOwner(PhysicsOwnerType::Component, this);

		mInternal->onColliderHit.connect(std::bind(&CCharacterController::triggerOnColliderHit, this, _1));
		mInternal->onControllerHit.connect(std::bind(&CCharacterController::triggerOnControllerHit, this, _1));

		mInternal->setLayer(mLayer);
		updateDimensions();
	}

	void CCharacterController::OnTransformChanged(TransformChangedFlags flags)
	{
		if (!SO()->getActive() || mInternal == nullptr)
			return;

		mInternal->setPosition(SO()->getTransform().getPosition());
	}

	void CCharacterController::UpdatePositionFromController()
	{
		mNotifyFlags = (TransformChangedFlags)0;
		SO()->setWorldPosition(mInternal->getPosition());
		mNotifyFlags = TCF_Transform;
	}

	void CCharacterController::UpdateDimensions()
	{
		Vector3 scale = SO()->getTransform().getScale();
		float height = mDesc.height * Math::abs(scale.y);
		float radius = mDesc.radius * Math::abs(std::max(scale.x, scale.z));

		mInternal->setHeight(height);
		mInternal->setRadius(radius);
	}

	void CCharacterController::DestroyInternal()
	{
		// This should release the last reference and destroy the internal controller
		if(mInternal)
		{
			mInternal->_setOwner(PhysicsOwnerType::None, nullptr);
			mInternal = nullptr;
		}
	}

	void CCharacterController::TriggerOnColliderHit(const ControllerColliderCollision& value)
	{
		// Const-cast and modify is okay because we're the only object receiving this event
		auto& hit = const_cast<ControllerColliderCollision&>(value);

		if(hit.colliderRaw)
		{
			const auto collider = (CCollider*)hit.colliderRaw->_getOwner(PhysicsOwnerType::Component);
			hit.collider = static_object_cast<CCollider>(collider->getHandle());
		}

		onColliderHit(hit);
	}

	void CCharacterController::TriggerOnControllerHit(const ControllerControllerCollision& value)
	{
		// Const-cast and modify is okay because we're the only object receiving this event
		ControllerControllerCollision& hit = const_cast<ControllerControllerCollision&>(value);

		if(hit.controllerRaw)
		{
			const auto controller = (CCharacterController*)hit.controllerRaw->_getOwner(PhysicsOwnerType::Component);
			hit.controller = static_object_cast<CCharacterController>(controller->getHandle());
		}

		onControllerHit(hit);
	}

	RTTITypeBase* CCharacterController::getRTTIStatic()
	{
		return CCharacterControllerRTTI::Instance();
	}

	RTTITypeBase* CCharacterController::getRTTI() const
	{
		return CCharacterController::GetRTTIStatic();
	}
}
