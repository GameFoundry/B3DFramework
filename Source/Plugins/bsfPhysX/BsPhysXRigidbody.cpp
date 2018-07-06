//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXRigidbody.h"
#include "Physics/BsCollider.h"
#include "BsFPhysXCollider.h"
#include "Scene/BsSceneObject.h"
#include "Physics/BsPhysics.h"
#include "PxRigidDynamic.h"
#include "PxScene.h"
#include "extensions/PxRigidBodyExt.h"
#include "BsPhysX.h"

using namespace physx;

namespace bs
{
	PxForceMode::Enum toPxForceMode(ForceMode mode)
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

	PxForceMode::Enum toPxForceMode(PointForceMode mode)
	{
		switch (mode)
		{
		case PointForceMode::Force:
			return PxForceMode::eFORCE;
		case PointForceMode::Impulse:
			return PxForceMode::eIMPULSE;
		}

		return PxForceMode::eFORCE;
	}

	PhysXRigidbody::PhysXRigidbody(PxPhysics* physx, PxScene* scene, const HSceneObject& linkedSO)
		:Rigidbody(linkedSO)
	{

		const Transform& tfrm = linkedSO->getTransform();
		PxTransform pxTfrm = toPxTransform(tfrm.getPosition(), tfrm.getRotation());

		mInternal = physx->createRigidDynamic(pxTfrm);
		mInternal->userData = this;

		scene->addActor(*mInternal);
	}

	PhysXRigidbody::~PhysXRigidbody()
	{
		mInternal->userData = nullptr;
		mInternal->release();
	}

	void PhysXRigidbody::move(const Vector3& position)
	{
		if (getIsKinematic())
		{
			PxTransform target;
			if (!mInternal->getKinematicTarget(target))
				target = PxTransform(PxIdentity);

			target.p = toPxVector(position);

			mInternal->setKinematicTarget(target);
		}
		else
		{
			setTransform(position, getRotation());
		}
	}

	void PhysXRigidbody::rotate(const Quaternion& rotation)
	{
		if (getIsKinematic())
		{
			PxTransform target;
			if (!mInternal->getKinematicTarget(target))
				target = PxTransform(PxIdentity);

			target.q = toPxQuaternion(rotation);

			mInternal->setKinematicTarget(target);
		}
		else
		{
			setTransform(getPosition(), rotation);
		}
	}

	Vector3 PhysXRigidbody::getPosition() const
	{
		return fromPxVector(mInternal->getGlobalPose().p);
	}

	Quaternion PhysXRigidbody::getRotation() const
	{
		return fromPxQuaternion(mInternal->getGlobalPose().q);
	}

	void PhysXRigidbody::setTransform(const Vector3& pos, const Quaternion& rot)
	{
		mInternal->setGlobalPose(toPxTransform(pos, rot));
	}

	void PhysXRigidbody::setMass(float mass)
	{
		if(((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoMass) != 0)
		{
			LOGWRN("Attempting to set Rigidbody mass, but it has automatic mass calculation turned on.");
			return;
		}

		mInternal->setMass(mass);
	}

	float PhysXRigidbody::getMass() const
	{
		return mInternal->getMass();
	}

	void PhysXRigidbody::setIsKinematic(bool kinematic)
	{
		mInternal->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kinematic);
	}

	bool PhysXRigidbody::getIsKinematic() const
	{
		return ((UINT32)mInternal->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC) != 0;
	}

	bool PhysXRigidbody::isSleeping() const
	{
		return mInternal->isSleeping();
	}

	void PhysXRigidbody::sleep()
	{
		mInternal->putToSleep();
	}

	void PhysXRigidbody::wakeUp()
	{
		mInternal->wakeUp();
	}

	void PhysXRigidbody::setSleepThreshold(float threshold)
	{
		mInternal->setSleepThreshold(threshold);
	}

	float PhysXRigidbody::getSleepThreshold() const
	{
		return mInternal->getSleepThreshold();
	}

	void PhysXRigidbody::setUseGravity(bool gravity)
	{
		mInternal->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !gravity);
	}

	bool PhysXRigidbody::getUseGravity() const
	{
		return ((UINT32)mInternal->getActorFlags() & PxActorFlag::eDISABLE_GRAVITY) == 0;
	}

	void PhysXRigidbody::setVelocity(const Vector3& velocity)
	{
		mInternal->setLinearVelocity(toPxVector(velocity));
	}

	Vector3 PhysXRigidbody::getVelocity() const
	{
		return fromPxVector(mInternal->getLinearVelocity());
	}

	void PhysXRigidbody::setAngularVelocity(const Vector3& velocity)
	{
		mInternal->setAngularVelocity(toPxVector(velocity));
	}

	Vector3 PhysXRigidbody::getAngularVelocity() const
	{
		return fromPxVector(mInternal->getAngularVelocity());
	}

	void PhysXRigidbody::setDrag(float drag)
	{
		mInternal->setLinearDamping(drag);
	}

	float PhysXRigidbody::getDrag() const
	{
		return mInternal->getLinearDamping();
	}

	void PhysXRigidbody::setAngularDrag(float drag)
	{
		mInternal->setAngularDamping(drag);
	}

	float PhysXRigidbody::getAngularDrag() const
	{
		return mInternal->getAngularDamping();
	}

	void PhysXRigidbody::setInertiaTensor(const Vector3& tensor)
	{
		if (((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoTensors) != 0)
		{
			LOGWRN("Attempting to set Rigidbody inertia tensor, but it has automatic tensor calculation turned on.");
			return;
		}

		mInternal->setMassSpaceInertiaTensor(toPxVector(tensor));
	}

	Vector3 PhysXRigidbody::getInertiaTensor() const
	{
		return fromPxVector(mInternal->getMassSpaceInertiaTensor());
	}

	void PhysXRigidbody::setMaxAngularVelocity(float maxVelocity)
	{
		mInternal->setMaxAngularVelocity(maxVelocity);
	}

	float PhysXRigidbody::getMaxAngularVelocity() const
	{
		return mInternal->getMaxAngularVelocity();
	}

	void PhysXRigidbody::setCenterOfMass(const Vector3& position, const Quaternion& rotation)
	{
		if (((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoTensors) != 0)
		{
			LOGWRN("Attempting to set Rigidbody center of mass, but it has automatic tensor calculation turned on.");
			return;
		}

		mInternal->setCMassLocalPose(toPxTransform(position, rotation));
	}

	Vector3 PhysXRigidbody::getCenterOfMassPosition() const
	{
		PxTransform cMassTfrm = mInternal->getCMassLocalPose();
		return fromPxVector(cMassTfrm.p);
	}

	Quaternion PhysXRigidbody::getCenterOfMassRotation() const
	{
		PxTransform cMassTfrm = mInternal->getCMassLocalPose();
		return fromPxQuaternion(cMassTfrm.q);
	}

	void PhysXRigidbody::setPositionSolverCount(UINT32 count)
	{
		mInternal->setSolverIterationCounts(std::max(1U, count), getVelocitySolverCount());
	}

	UINT32 PhysXRigidbody::getPositionSolverCount() const
	{
		UINT32 posCount = 1;
		UINT32 velCount = 1;

		mInternal->getSolverIterationCounts(posCount, velCount);
		return posCount;
	}

	void PhysXRigidbody::setVelocitySolverCount(UINT32 count)
	{
		mInternal->setSolverIterationCounts(getPositionSolverCount(), std::max(1U, count));
	}

	UINT32 PhysXRigidbody::getVelocitySolverCount() const
	{
		UINT32 posCount = 1;
		UINT32 velCount = 1;

		mInternal->getSolverIterationCounts(posCount, velCount);
		return velCount;
	}

	void PhysXRigidbody::setFlags(RigidbodyFlag flags)
	{
		bool ccdEnabledOld = mInternal->getRigidBodyFlags() & PxRigidBodyFlag::eENABLE_CCD;
		bool ccdEnabledNew = ((UINT32)flags & (UINT32)RigidbodyFlag::CCD) != 0;
		
		if(ccdEnabledOld != ccdEnabledNew)
		{
			if(ccdEnabledNew)
			{
				if (!gPhysics().hasFlag(PhysicsFlag::CCD_Enable))
					LOGWRN("Enabling CCD on a Rigidbody but CCD is not enabled globally.");
			}

			mInternal->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, ccdEnabledNew);

			// Enable/disable CCD on shapes so the filter can handle them properly
			UINT32 numShapes = mInternal->getNbShapes();
			PxShape** shapes = (PxShape**)bs_stack_alloc(sizeof(PxShape*) * numShapes);

			mInternal->getShapes(shapes, sizeof(PxShape*) * numShapes);

			for (UINT32 i = 0; i < numShapes; i++)
			{
				Collider* collider = (Collider*)shapes[i]->userData;
				collider->_getInternal()->_setCCD(ccdEnabledNew);
			}
		}

		Rigidbody::setFlags(flags);
	}

	void PhysXRigidbody::addForce(const Vector3& force, ForceMode mode)
	{
		mInternal->addForce(toPxVector(force), toPxForceMode(mode));
	}

	void PhysXRigidbody::addTorque(const Vector3& force, ForceMode mode)
	{
		mInternal->addTorque(toPxVector(force), toPxForceMode(mode));
	}

	void PhysXRigidbody::addForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode)
	{
		const PxVec3& pxForce = toPxVector(force);
		const PxVec3& pxPos = toPxVector(position);

		const PxTransform globalPose = mInternal->getGlobalPose();
		PxVec3 centerOfMass = globalPose.transform(mInternal->getCMassLocalPose().p);

		PxForceMode::Enum pxMode = toPxForceMode(mode);

		PxVec3 torque = (pxPos - centerOfMass).cross(pxForce);
		mInternal->addForce(pxForce, pxMode);
		mInternal->addTorque(torque, pxMode);
	}

	Vector3 PhysXRigidbody::getVelocityAtPoint(const Vector3& point) const
	{
		const PxVec3& pxPoint = toPxVector(point);

		const PxTransform globalPose = mInternal->getGlobalPose();
		const PxVec3 centerOfMass = globalPose.transform(mInternal->getCMassLocalPose().p);
		const PxVec3 rpoint = pxPoint - centerOfMass;

		PxVec3 velocity = mInternal->getLinearVelocity();
		velocity += mInternal->getAngularVelocity().cross(rpoint);

		return fromPxVector(velocity);
	}

	void PhysXRigidbody::updateMassDistribution() 
	{
		if (((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoTensors) == 0)
			return;

		if (((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoMass) == 0)
		{
			PxRigidBodyExt::setMassAndUpdateInertia(*mInternal, mInternal->getMass());
		}
		else
		{
			UINT32 numShapes = mInternal->getNbShapes();
			if (numShapes == 0)
			{
				PxRigidBodyExt::setMassAndUpdateInertia(*mInternal, mInternal->getMass());
				return;
			}

			PxShape** shapes = (PxShape**)bs_stack_alloc(sizeof(PxShape*) * numShapes);
			mInternal->getShapes(shapes, numShapes);

			float* masses = (float*)bs_stack_alloc(sizeof(float) * numShapes);
			for (UINT32 i = 0; i < numShapes; i++)
				masses[i] = ((Collider*)shapes[i]->userData)->getMass();

			PxRigidBodyExt::setMassAndUpdateInertia(*mInternal, masses, numShapes);

			bs_stack_free(masses);
			bs_stack_free(shapes);
		}
	}

	void PhysXRigidbody::addCollider(Collider* collider)
	{
		if (collider == nullptr)
			return;

		FPhysXCollider* physxCollider = static_cast<FPhysXCollider*>(collider->_getInternal());
		physxCollider->_setCCD(((UINT32)mFlags & (UINT32)RigidbodyFlag::CCD) != 0);

		mInternal->attachShape(*physxCollider->_getShape());
	}

	void PhysXRigidbody::removeCollider(Collider* collider)
	{
		if (collider == nullptr)
			return;

		FPhysXCollider* physxCollider = static_cast<FPhysXCollider*>(collider->_getInternal());
		physxCollider->_setCCD(false);

		mInternal->detachShape(*physxCollider->_getShape());
	}

	void PhysXRigidbody::removeColliders()
	{
		UINT32 numShapes = mInternal->getNbShapes();
		PxShape** shapes = (PxShape**)bs_stack_alloc(sizeof(PxShape*) * numShapes);

		mInternal->getShapes(shapes, sizeof(PxShape*) * numShapes);

		for (UINT32 i = 0; i < numShapes; i++)
		{
			Collider* collider = (Collider*)shapes[i]->userData;
			collider->_getInternal()->_setCCD(false);

			mInternal->detachShape(*shapes[i]);
		}

		bs_stack_free(shapes);
	}
}