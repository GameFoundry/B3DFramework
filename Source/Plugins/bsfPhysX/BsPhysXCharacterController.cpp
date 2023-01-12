//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXCharacterController.h"
#include "Utility/BsTime.h"
#include "BsPhysX.h"
#include "Components/BsCCollider.h"
#include "characterkinematic/PxControllerManager.h"

using namespace physx;

using namespace bs;

PxExtendedVec3 ToPxExtVector(const Vector3& input)
{
	return PxExtendedVec3(input.X, input.Y, input.Z);
}

Vector3 FromPxExtVector(const PxExtendedVec3& input)
{
	return Vector3((float)input.x, (float)input.y, (float)input.z);
}

PxCapsuleClimbingMode::Enum ToPxEnum(CharacterClimbingMode value)
{
	return value == CharacterClimbingMode::Normal
		? PxCapsuleClimbingMode::eEASY
		: PxCapsuleClimbingMode::eCONSTRAINED;
}

PxControllerNonWalkableMode::Enum ToPxEnum(CharacterNonWalkableMode value)
{
	return value == CharacterNonWalkableMode::Prevent
		? PxControllerNonWalkableMode::ePREVENT_CLIMBING
		: PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING;
}

CharacterClimbingMode FromPxEnum(PxCapsuleClimbingMode::Enum value)
{
	return value == PxCapsuleClimbingMode::eEASY
		? CharacterClimbingMode::Normal
		: CharacterClimbingMode::Constrained;
}

CharacterNonWalkableMode FromPxEnum(PxControllerNonWalkableMode::Enum value)
{
	return value == PxControllerNonWalkableMode::ePREVENT_CLIMBING
		? CharacterNonWalkableMode::Prevent
		: CharacterNonWalkableMode::PreventAndSlide;
}

PxCapsuleControllerDesc ToPxDesc(const CHAR_CONTROLLER_DESC& desc)
{
	PxCapsuleControllerDesc output;
	output.climbingMode = ToPxEnum(desc.ClimbingMode);
	output.nonWalkableMode = ToPxEnum(desc.NonWalkableMode);
	output.contactOffset = desc.ContactOffset;
	output.stepOffset = desc.StepOffset;
	output.slopeLimit = desc.SlopeLimit.GetValueInRadians();
	output.height = desc.Height;
	output.radius = desc.Radius;
	output.upDirection = ToPxVector(desc.Up);
	output.position = ToPxExtVector(desc.Position);

	return output;
}

PhysXCharacterController::PhysXCharacterController(PxControllerManager* manager, const CHAR_CONTROLLER_DESC& desc)
	: CharacterController(desc)
{
	PxCapsuleControllerDesc pxDesc = ToPxDesc(desc);
	pxDesc.reportCallback = this;
	pxDesc.material = GetPhysX().GetDefaultMaterial();
	pxDesc.height = pxDesc.height <= 0 ? 0.01f : pxDesc.height;

	mController = static_cast<PxCapsuleController*>(manager->createController(pxDesc));
	mController->setUserData(this);
}

PhysXCharacterController::~PhysXCharacterController()
{
	mController->setUserData(nullptr);
	mController->release();
}

CharacterCollisionFlags PhysXCharacterController::Move(const Vector3& displacement)
{
	PxControllerFilters filters;
	filters.mFilterCallback = this;
	filters.mFilterFlags = PxQueryFlag::eANY_HIT | PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
	filters.mCCTFilterCallback = this;

	float curTime = GetTime().GetTime();
	float delta = curTime - mLastMoveCall;
	mLastMoveCall = curTime;

	PxControllerCollisionFlags collisionFlag = mController->move(ToPxVector(displacement), mMinMoveDistance, delta, filters);

	CharacterCollisionFlags output;
	if(collisionFlag.isSet(PxControllerCollisionFlag::eCOLLISION_DOWN))
		output.Set(CharacterCollisionFlag::Down);

	if(collisionFlag.isSet(PxControllerCollisionFlag::eCOLLISION_UP))
		output.Set(CharacterCollisionFlag::Up);

	if(collisionFlag.isSet(PxControllerCollisionFlag::eCOLLISION_SIDES))
		output.Set(CharacterCollisionFlag::Sides);

	return output;
}

Vector3 PhysXCharacterController::GetPosition() const
{
	return FromPxExtVector(mController->getPosition());
}

void PhysXCharacterController::SetPosition(const Vector3& position)
{
	mController->setPosition(ToPxExtVector(position));
}

Vector3 PhysXCharacterController::GetFootPosition() const
{
	return FromPxExtVector(mController->getFootPosition());
}

void PhysXCharacterController::SetFootPosition(const Vector3& position)
{
	mController->setFootPosition(ToPxExtVector(position));
}

float PhysXCharacterController::GetRadius() const
{
	return mController->getRadius();
}

void PhysXCharacterController::SetRadius(float radius)
{
	mController->setRadius(radius);
}

float PhysXCharacterController::GetHeight() const
{
	return mController->getHeight();
}

void PhysXCharacterController::SetHeight(float height)
{
	mController->setHeight(height);
}

Vector3 PhysXCharacterController::GetUp() const
{
	return FromPxVector(mController->getUpDirection());
}

void PhysXCharacterController::SetUp(const Vector3& up)
{
	mController->setUpDirection(ToPxVector(up));
}

CharacterClimbingMode PhysXCharacterController::GetClimbingMode() const
{
	return FromPxEnum(mController->getClimbingMode());
}

void PhysXCharacterController::SetClimbingMode(CharacterClimbingMode mode)
{
	mController->setClimbingMode(ToPxEnum(mode));
}

CharacterNonWalkableMode PhysXCharacterController::GetNonWalkableMode() const
{
	return FromPxEnum(mController->getNonWalkableMode());
}

void PhysXCharacterController::SetNonWalkableMode(CharacterNonWalkableMode mode)
{
	mController->setNonWalkableMode(ToPxEnum(mode));
}

float PhysXCharacterController::GetMinMoveDistance() const
{
	return mMinMoveDistance;
}

void PhysXCharacterController::SetMinMoveDistance(float value)
{
	mMinMoveDistance = value;
}

float PhysXCharacterController::GetContactOffset() const
{
	return mController->getContactOffset();
}

void PhysXCharacterController::SetContactOffset(float value)
{
	mController->setContactOffset(value);
}

float PhysXCharacterController::GetStepOffset() const
{
	return mController->getStepOffset();
}

void PhysXCharacterController::SetStepOffset(float value)
{
	mController->setStepOffset(value);
}

Radian PhysXCharacterController::GetSlopeLimit() const
{
	return Radian(mController->getSlopeLimit());
}

void PhysXCharacterController::SetSlopeLimit(Radian value)
{
	mController->setSlopeLimit(value.GetValueInRadians());
}

void PhysXCharacterController::onShapeHit(const PxControllerShapeHit& hit)
{
	if(OnColliderHit.Empty())
		return;

	ControllerColliderCollision collision;
	collision.Position = FromPxExtVector(hit.worldPos);
	collision.Normal = FromPxVector(hit.worldNormal);
	collision.MotionDir = FromPxVector(hit.dir);
	collision.MotionAmount = hit.length;
	collision.TriangleIndex = hit.triangleIndex;
	collision.ColliderRaw = (Collider*)hit.shape->userData;

	OnColliderHit(collision);
}

void PhysXCharacterController::onControllerHit(const PxControllersHit& hit)
{
	if(CharacterController::OnControllerHit.Empty())
		return;

	ControllerControllerCollision collision;
	collision.Position = FromPxExtVector(hit.worldPos);
	collision.Normal = FromPxVector(hit.worldNormal);
	collision.MotionDir = FromPxVector(hit.dir);
	collision.MotionAmount = hit.length;
	collision.ControllerRaw = (CharacterController*)hit.controller->getUserData();

	CharacterController::OnControllerHit(collision);
}

PxQueryHitType::Enum PhysXCharacterController::preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags)
{
	PxFilterData colliderFilterData = shape->getSimulationFilterData();
	u64 colliderLayer = *(u64*)&colliderFilterData.word0;

	bool canCollide = GetPhysics().IsCollisionEnabled(colliderLayer, GetLayer());

	if(canCollide)
		return PxSceneQueryHitType::eBLOCK;

	return PxSceneQueryHitType::eNONE;
}

PxQueryHitType::Enum PhysXCharacterController::postFilter(const PxFilterData& filterData, const PxQueryHit& hit)
{
	return PxSceneQueryHitType::eBLOCK;
}

bool PhysXCharacterController::filter(const PxController& a, const PxController& b)
{
	CharacterController* controllerA = (CharacterController*)a.getUserData();
	CharacterController* controllerB = (CharacterController*)b.getUserData();

	bool canCollide = GetPhysics().IsCollisionEnabled(controllerA->GetLayer(), controllerB->GetLayer());
	return canCollide;
}
