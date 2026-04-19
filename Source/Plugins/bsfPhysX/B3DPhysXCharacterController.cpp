//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPhysXCharacterController.h"
#include "Utility/B3DTime.h"
#include "B3DPhysX.h"
#include "Components/B3DCollider.h"
#include "characterkinematic/PxControllerManager.h"

using namespace physx;
using namespace b3d;

static PxCapsuleControllerDesc ToPxDesc(const CharacterControllerCreateInformation& createInformation)
{
	PxCapsuleControllerDesc output;
	output.climbingMode = ToPxEnum(createInformation.ClimbingMode);
	output.nonWalkableMode = ToPxEnum(createInformation.NonWalkableMode);
	output.contactOffset = createInformation.ContactOffset;
	output.stepOffset = createInformation.StepOffset;
	output.slopeLimit = createInformation.SlopeLimit.GetValueInRadians();
	output.height = createInformation.Height;
	output.radius = createInformation.Radius;
	output.upDirection = ToPxVector(createInformation.Up);
	output.position = ToPxExtVector(createInformation.Position);

	return output;
}

PhysXCharacterController::PhysXCharacterController(PxControllerManager* manager, CharacterController& owner, const CharacterControllerCreateInformation& createInformation)
	:mOwner(owner)
{
	PxCapsuleControllerDesc pxDesc = ToPxDesc(createInformation);
	pxDesc.reportCallback = this;
	pxDesc.material = GetPhysX().GetDefaultMaterial();
	pxDesc.height = pxDesc.height <= 0 ? 0.01f : pxDesc.height;

	mController = static_cast<PxCapsuleController*>(manager->createController(pxDesc));
	mController->setUserData(&owner);
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

	float curTime = GetTime().GetRealTimeInSeconds();
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

void PhysXCharacterController::onShapeHit(const PxControllerShapeHit& hit)
{
	if(mOwner.OnColliderHit.Empty())
		return;

	ControllerColliderCollision collision;
	collision.Position = FromPxExtVector(hit.worldPos);
	collision.Normal = FromPxVector(hit.worldNormal);
	collision.MotionDir = FromPxVector(hit.dir);
	collision.MotionAmount = hit.length;
	collision.TriangleIndex = hit.triangleIndex;
	collision.ColliderShape = (ColliderShape*)hit.shape->userData;

	mOwner.OnColliderHit(collision);
}

void PhysXCharacterController::onControllerHit(const PxControllersHit& hit)
{
	if(mOwner.OnControllerHit.Empty())
		return;

	ControllerControllerCollision collision;
	collision.Position = FromPxExtVector(hit.worldPos);
	collision.Normal = FromPxVector(hit.worldNormal);
	collision.MotionDir = FromPxVector(hit.dir);
	collision.MotionAmount = hit.length;
	collision.ControllerRaw = (CharacterController*)hit.controller->getUserData();

	mOwner.OnControllerHit(collision);
}

PxQueryHitType::Enum PhysXCharacterController::preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags)
{
	PxFilterData colliderFilterData = shape->getSimulationFilterData();
	u64 colliderLayer = *(u64*)&colliderFilterData.word0;

	const bool canCollide = GetPhysics().IsCollisionEnabled(colliderLayer, mOwner.GetLayer());

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
