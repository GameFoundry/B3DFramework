//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPhysXCharacterController.h"
#include "BsTime.h"
#include "BsPhysX.h"
#include "BsCCollider.h"
#include "characterkinematic\PxControllerManager.h"

using namespace physx;

namespace bs
{
	PxExtendedVec3 toPxExtVector(const Vector3& input)
	{
		return PxExtendedVec3(input.x, input.y, input.z);
	}

	Vector3 fromPxExtVector(const PxExtendedVec3& input)
	{
		return Vector3((float)input.x, (float)input.y, (float)input.z);
	}

	PxCapsuleClimbingMode::Enum toPxEnum(CharacterClimbingMode value)
	{
		return value == CharacterClimbingMode::Normal
			? PxCapsuleClimbingMode::eEASY
			: PxCapsuleClimbingMode::eCONSTRAINED;
	}

	PxControllerNonWalkableMode::Enum toPxEnum(CharacterNonWalkableMode value)
	{
		return value == CharacterNonWalkableMode::Prevent
			? PxControllerNonWalkableMode::ePREVENT_CLIMBING
			: PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING;
	}

	CharacterClimbingMode fromPxEnum(PxCapsuleClimbingMode::Enum value)
	{
		return value == PxCapsuleClimbingMode::eEASY
			? CharacterClimbingMode::Normal
			: CharacterClimbingMode::Constrained;
	}

	CharacterNonWalkableMode fromPxEnum(PxControllerNonWalkableMode::Enum value)
	{
		return value == PxControllerNonWalkableMode::ePREVENT_CLIMBING
			? CharacterNonWalkableMode::Prevent
			: CharacterNonWalkableMode::PreventAndSlide;
	}

	PxCapsuleControllerDesc toPxDesc(const CHAR_CONTROLLER_DESC& desc)
	{
		PxCapsuleControllerDesc output;
		output.climbingMode = toPxEnum(desc.climbingMode);
		output.nonWalkableMode = toPxEnum(desc.nonWalkableMode);
		output.contactOffset = desc.contactOffset;
		output.stepOffset = desc.stepOffset;
		output.slopeLimit = desc.slopeLimit.valueRadians();
		output.height = desc.height;
		output.radius = desc.radius;
		output.upDirection = toPxVector(desc.up);
		output.position = toPxExtVector(desc.position);

		return output;
	}

	PhysXCharacterController::PhysXCharacterController(PxControllerManager* manager, const CHAR_CONTROLLER_DESC& desc)
		:CharacterController(desc)
	{
		PxCapsuleControllerDesc pxDesc = toPxDesc(desc);
		pxDesc.reportCallback = this;
		pxDesc.material = gPhysX().getDefaultMaterial();

		mController = static_cast<PxCapsuleController*>(manager->createController(pxDesc));
		mController->setUserData(this);
	}

	PhysXCharacterController::~PhysXCharacterController()
	{
		mController->setUserData(nullptr);
		mController->release();
	}

	CharacterCollisionFlags PhysXCharacterController::move(const Vector3& displacement)
	{
		PxControllerFilters filters;
		filters.mFilterCallback = this;
		filters.mFilterFlags = PxQueryFlag::eANY_HIT | PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
		filters.mCCTFilterCallback = this;

		float curTime = gTime().getTime();
		float delta = curTime - mLastMoveCall;
		mLastMoveCall = curTime;

		PxControllerCollisionFlags collisionFlag = mController->move(toPxVector(displacement), mMinMoveDistance, delta, filters);

		CharacterCollisionFlags output;
		if (collisionFlag.isSet(PxControllerCollisionFlag::eCOLLISION_DOWN))
			output.set(CharacterCollisionFlag::Down);

		if (collisionFlag.isSet(PxControllerCollisionFlag::eCOLLISION_UP))
			output.set(CharacterCollisionFlag::Up);

		if (collisionFlag.isSet(PxControllerCollisionFlag::eCOLLISION_SIDES))
			output.set(CharacterCollisionFlag::Sides);

		return output;
	}

	Vector3 PhysXCharacterController::getPosition() const
	{
		return fromPxExtVector(mController->getPosition());
	}

	void PhysXCharacterController::setPosition(const Vector3& position)
	{
		mController->setPosition(toPxExtVector(position));
	}

	Vector3 PhysXCharacterController::getFootPosition() const
	{
		return fromPxExtVector(mController->getFootPosition());
	}

	void PhysXCharacterController::setFootPosition(const Vector3& position)
	{
		mController->setFootPosition(toPxExtVector(position));
	}

	float PhysXCharacterController::getRadius() const
	{
		return mController->getRadius();
	}

	void PhysXCharacterController::setRadius(float radius)
	{
		mController->setRadius(radius);
	}

	float PhysXCharacterController::getHeight() const
	{
		return mController->getHeight();
	}

	void PhysXCharacterController::setHeight(float height)
	{
		mController->setHeight(height);
	}

	Vector3 PhysXCharacterController::getUp() const
	{
		return fromPxVector(mController->getUpDirection());
	}

	void PhysXCharacterController::setUp(const Vector3& up)
	{
		mController->setUpDirection(toPxVector(up));
	}

	CharacterClimbingMode PhysXCharacterController::getClimbingMode() const
	{
		return fromPxEnum(mController->getClimbingMode());
	}

	void PhysXCharacterController::setClimbingMode(CharacterClimbingMode mode)
	{
		mController->setClimbingMode(toPxEnum(mode));
	}

	CharacterNonWalkableMode PhysXCharacterController::getNonWalkableMode() const
	{
		return fromPxEnum(mController->getNonWalkableMode());
	}

	void PhysXCharacterController::setNonWalkableMode(CharacterNonWalkableMode mode)
	{
		mController->setNonWalkableMode(toPxEnum(mode));
	}

	float PhysXCharacterController::getMinMoveDistance() const
	{
		return mMinMoveDistance;
	}

	void PhysXCharacterController::setMinMoveDistance(float value)
	{
		mMinMoveDistance = value;
	}

	float PhysXCharacterController::getContactOffset() const
	{
		return mController->getContactOffset();
	}

	void PhysXCharacterController::setContactOffset(float value)
	{
		mController->setContactOffset(value);
	}

	float PhysXCharacterController::getStepOffset() const
	{
		return mController->getStepOffset();
	}

	void PhysXCharacterController::setStepOffset(float value)
	{
		mController->setStepOffset(value);
	}

	Radian PhysXCharacterController::getSlopeLimit() const
	{
		return Radian(mController->getSlopeLimit());
	}

	void PhysXCharacterController::setSlopeLimit(Radian value)
	{
		mController->setSlopeLimit(value.valueRadians());
	}

	void PhysXCharacterController::onShapeHit(const PxControllerShapeHit& hit)
	{
		if (onColliderHit.empty())
			return;

		ControllerColliderCollision collision;
		collision.position = fromPxExtVector(hit.worldPos);
		collision.normal = fromPxVector(hit.worldNormal);
		collision.motionDir = fromPxVector(hit.dir);
		collision.motionAmount = hit.length;
		collision.triangleIndex = hit.triangleIndex;
		collision.colliderRaw = (Collider*)hit.shape->userData;

		onColliderHit(collision);
	}

	void PhysXCharacterController::onControllerHit(const PxControllersHit& hit)
	{
		if (CharacterController::onControllerHit.empty())
			return;

		ControllerControllerCollision collision;
		collision.position = fromPxExtVector(hit.worldPos);
		collision.normal = fromPxVector(hit.worldNormal);
		collision.motionDir = fromPxVector(hit.dir);
		collision.motionAmount = hit.length;
		collision.controllerRaw = (CharacterController*)hit.controller->getUserData();

		CharacterController::onControllerHit(collision);
	}

	PxQueryHitType::Enum PhysXCharacterController::preFilter(const PxFilterData& filterData, const PxShape* shape,
		const PxRigidActor* actor, PxHitFlags& queryFlags)
	{
		PxFilterData colliderFilterData = shape->getSimulationFilterData();
		UINT64 colliderLayer = *(UINT64*)&colliderFilterData.word0;

		bool canCollide = gPhysics().isCollisionEnabled(colliderLayer, getLayer());

		if(canCollide)
			return PxSceneQueryHitType::eBLOCK;

		return PxSceneQueryHitType::eNONE;
	}

	PxQueryHitType::Enum PhysXCharacterController::postFilter(const PxFilterData& filterData,
		const PxQueryHit& hit)
	{
		return PxSceneQueryHitType::eBLOCK;
	}

	bool PhysXCharacterController::filter(const PxController& a, const PxController& b)
	{
		CharacterController* controllerA = (CharacterController*)a.getUserData();
		CharacterController* controllerB = (CharacterController*)b.getUserData();

		bool canCollide = gPhysics().isCollisionEnabled(controllerA->getLayer(), controllerB->getLayer());
		return canCollide;
	}
}