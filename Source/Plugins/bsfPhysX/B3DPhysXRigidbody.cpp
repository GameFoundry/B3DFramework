//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPhysXRigidbody.h"
#include "Scene/B3DSceneObject.h"
#include "Physics/B3DPhysics.h"
#include "PxRigidDynamic.h"
#include "PxScene.h"
#include "extensions/PxRigidBodyExt.h"
#include "B3DPhysX.h"
#include "B3DPhysXColliderShape.h"

using namespace physx;

using namespace b3d;

PxForceMode::Enum ToPxForceMode(ForceMode mode)
{
	switch(mode)
	{
	case ForceMode::Force:
		return PxForceMode::eFORCE;
	case ForceMode::Impulse:
		return PxForceMode::eIMPULSE;
	case ForceMode::Velocity:
		return PxForceMode::eVELOCITY_CHANGE;
	case ForceMode::Acceleration:
		return PxForceMode::eACCELERATION;
	}

	return PxForceMode::eFORCE;
}

PxForceMode::Enum ToPxForceMode(PointForceMode mode)
{
	switch(mode)
	{
	case PointForceMode::Force:
		return PxForceMode::eFORCE;
	case PointForceMode::Impulse:
		return PxForceMode::eIMPULSE;
	}

	return PxForceMode::eFORCE;
}

PhysXRigidbody::PhysXRigidbody(Rigidbody& owner)
{
	mPxRigidDynamic = GetPhysX().GetPhysX()->createRigidDynamic(PxTransform(PxIdentity));
	mPxRigidDynamic->userData = &owner;
}

PhysXRigidbody::~PhysXRigidbody()
{
	if(mPxScene != nullptr)
		PhysXRigidbody::RemoveFromScene();

	mPxRigidDynamic->userData = nullptr;
	mPxRigidDynamic->release();
}

void PhysXRigidbody::Move(const Vector3& position)
{
	PxTransform target;
	if(!mPxRigidDynamic->getKinematicTarget(target))
		target = PxTransform(PxIdentity);

	target.p = ToPxVector(position);

	mPxRigidDynamic->setKinematicTarget(target);
}

void PhysXRigidbody::Rotate(const Quaternion& rotation)
{
	PxTransform target;
	if(!mPxRigidDynamic->getKinematicTarget(target))
		target = PxTransform(PxIdentity);

	target.q = ToPxQuaternion(rotation);

	mPxRigidDynamic->setKinematicTarget(target);
}

void PhysXRigidbody::SetTransform(const Vector3& position, const Quaternion& rotation)
{
	mPxRigidDynamic->setGlobalPose(ToPxTransform(position, rotation));
}

void PhysXRigidbody::GetTransform(Vector3& outPosition, Quaternion& outRotation)
{
	const PxTransform& pxTransform = mPxRigidDynamic->getGlobalPose();

	outPosition = FromPxVector(pxTransform.p);
	outRotation = FromPxQuaternion(pxTransform.q);
}

void PhysXRigidbody::SetMass(float mass)
{
	mPxRigidDynamic->setMass(mass);
}

void PhysXRigidbody::SetIsKinematic(bool kinematic)
{
	mPxRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kinematic);
}

bool PhysXRigidbody::IsSleeping() const
{
	return mPxRigidDynamic->isSleeping();
}

void PhysXRigidbody::Sleep()
{
	mPxRigidDynamic->putToSleep();
}

void PhysXRigidbody::WakeUp()
{
	mPxRigidDynamic->wakeUp();
}

void PhysXRigidbody::SetSleepThreshold(float threshold)
{
	mPxRigidDynamic->setSleepThreshold(threshold);
}

void PhysXRigidbody::SetUseGravity(bool gravity)
{
	mPxRigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !gravity);
}

void PhysXRigidbody::SetVelocity(const Vector3& velocity)
{
	mPxRigidDynamic->setLinearVelocity(ToPxVector(velocity));
}

Vector3 PhysXRigidbody::GetVelocity() const
{
	return FromPxVector(mPxRigidDynamic->getLinearVelocity());
}

void PhysXRigidbody::SetAngularVelocity(const Vector3& velocity)
{
	mPxRigidDynamic->setAngularVelocity(ToPxVector(velocity));
}

Vector3 PhysXRigidbody::GetAngularVelocity() const
{
	return FromPxVector(mPxRigidDynamic->getAngularVelocity());
}

void PhysXRigidbody::SetDrag(float drag)
{
	mPxRigidDynamic->setLinearDamping(drag);
}

void PhysXRigidbody::SetAngularDrag(float drag)
{
	mPxRigidDynamic->setAngularDamping(drag);
}

void PhysXRigidbody::SetInertiaTensor(const Vector3& tensor)
{
	mPxRigidDynamic->setMassSpaceInertiaTensor(ToPxVector(tensor));
}

Vector3 PhysXRigidbody::GetInertiaTensor() const
{
	return FromPxVector(mPxRigidDynamic->getMassSpaceInertiaTensor());
}

void PhysXRigidbody::SetMaxAngularVelocity(float maxVelocity)
{
	mPxRigidDynamic->setMaxAngularVelocity(maxVelocity);
}

void PhysXRigidbody::SetCenterOfMass(const Vector3& position, const Quaternion& rotation)
{
	mPxRigidDynamic->setCMassLocalPose(ToPxTransform(position, rotation));
}

void PhysXRigidbody::GetCenterOfMass(Vector3& outPosition, Quaternion& outRotation) 
{
	const PxTransform& centerOfMassTransform = mPxRigidDynamic->getCMassLocalPose();
	outPosition = FromPxVector(centerOfMassTransform.p);
	outRotation = FromPxQuaternion(centerOfMassTransform.q);
}

void PhysXRigidbody::SetSolverIterationCounts(u32 positionCount, u32 velocityCount)
{
	mPxRigidDynamic->setSolverIterationCounts(std::max(1U, positionCount), std::max(1U, velocityCount));
}

void PhysXRigidbody::SetFlags(RigidbodyFlags flags)
{
	bool ccdEnabledOld = mPxRigidDynamic->getRigidBodyFlags() & PxRigidBodyFlag::eENABLE_CCD;
	bool ccdEnabledNew = flags.IsSet(RigidbodyFlag::CCD);

	if(ccdEnabledOld != ccdEnabledNew)
	{
		mPxRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, ccdEnabledNew);

		// Enable/disable CCD on shapes so the filter can handle them properly
		const u32 shapeCount = mPxRigidDynamic->getNbShapes();
		StackMemory<PxShape*[]> shapes(shapeCount);

		mPxRigidDynamic->getShapes(shapes, sizeof(PxShape*) * shapeCount);

		for(u32 i = 0; i < shapeCount; i++)
		{
			ColliderShape* const colliderShape = (ColliderShape*)shapes[i]->userData;
			colliderShape->SetContinuousCollisionDetection(ccdEnabledNew);
		}
	}
}

void PhysXRigidbody::AddForce(const Vector3& force, ForceMode mode)
{
	mPxRigidDynamic->addForce(ToPxVector(force), ToPxForceMode(mode));
}

void PhysXRigidbody::AddTorque(const Vector3& force, ForceMode mode)
{
	mPxRigidDynamic->addTorque(ToPxVector(force), ToPxForceMode(mode));
}

void PhysXRigidbody::AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode)
{
	const PxVec3& pxForce = ToPxVector(force);
	const PxVec3& pxPos = ToPxVector(position);

	const PxTransform globalPose = mPxRigidDynamic->getGlobalPose();
	PxVec3 centerOfMass = globalPose.transform(mPxRigidDynamic->getCMassLocalPose().p);

	PxForceMode::Enum pxMode = ToPxForceMode(mode);

	PxVec3 torque = (pxPos - centerOfMass).cross(pxForce);
	mPxRigidDynamic->addForce(pxForce, pxMode);
	mPxRigidDynamic->addTorque(torque, pxMode);
}

Vector3 PhysXRigidbody::GetVelocityAtPoint(const Vector3& point) const
{
	const PxVec3& pxPoint = ToPxVector(point);

	const PxTransform globalPose = mPxRigidDynamic->getGlobalPose();
	const PxVec3 centerOfMass = globalPose.transform(mPxRigidDynamic->getCMassLocalPose().p);
	const PxVec3 rpoint = pxPoint - centerOfMass;

	PxVec3 velocity = mPxRigidDynamic->getLinearVelocity();
	velocity += mPxRigidDynamic->getAngularVelocity().cross(rpoint);

	return FromPxVector(velocity);
}

void PhysXRigidbody::UpdateMassDistribution(bool autoMassEnabled)
{
	if(autoMassEnabled)
		PxRigidBodyExt::setMassAndUpdateInertia(*mPxRigidDynamic, mPxRigidDynamic->getMass());
	else
	{
		const u32 shapeCount = mPxRigidDynamic->getNbShapes();
		if(shapeCount == 0)
		{
			PxRigidBodyExt::setMassAndUpdateInertia(*mPxRigidDynamic, mPxRigidDynamic->getMass());
			return;
		}

		StackMemory<PxShape*[]> shapes(shapeCount);
		mPxRigidDynamic->getShapes(shapes, shapeCount);

		StackMemory<float[]> masses(shapeCount);
		for(u32 shapeIndex = 0; shapeIndex < shapeCount; shapeIndex++)
			masses[shapeIndex] = ((ColliderShape*)shapes[shapeIndex]->userData)->GetMass();

		PxRigidBodyExt::setMassAndUpdateInertia(*mPxRigidDynamic, masses, shapeCount);
	}
}

void PhysXRigidbody::AttachShape(const SPtr<ColliderShape>& shape)
{
	if(!B3D_ENSURE(shape != nullptr))
		return;

	const PhysXColliderShape& physxShape = static_cast<const PhysXColliderShape&>(*shape);
	mPxRigidDynamic->attachShape(*physxShape.GetPxShape());
}

void PhysXRigidbody::DetachShape(const SPtr<ColliderShape>& shape)
{
	if(!B3D_ENSURE(shape != nullptr))
		return;

	const PhysXColliderShape& physxShape = static_cast<const PhysXColliderShape&>(*shape);
	mPxRigidDynamic->detachShape(*physxShape.GetPxShape());
}

void PhysXRigidbody::AddToScene(PhysicsScene& scene)
{
	if(!B3D_ENSURE(mPxScene == nullptr))
		return;

	const PhysXScene& physxScene = static_cast<PhysXScene&>(scene);
	mPxScene = &physxScene.GetPxScene();

	mPxScene->addActor(*mPxRigidDynamic);
}

void PhysXRigidbody::RemoveFromScene()
{
	if(!B3D_ENSURE(mPxScene != nullptr))
		return;

	mPxScene->removeActor(*mPxRigidDynamic);
	mPxScene = nullptr;
}
