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
		const Transform& tfrm = linkedSO->GetTransform();
		PxTransform pxTfrm = toPxTransform(tfrm.GetPosition(), tfrm.getRotation());

		mInternal = physx->CreateRigidDynamic(pxTfrm);
		mInternal->userData = this;

		scene->AddActor(*mInternal);
	}

	PhysXRigidbody::~PhysXRigidbody()
	{
		mInternal->userData = nullptr;
		mInternal->Release();
	}

	void PhysXRigidbody::Move(const Vector3& position)
	{
		if (getIsKinematic())
		{
			PxTransform target;
			if (!mInternal->GetKinematicTarget(target))
				target = PxTransform(PxIdentity);

			target.p = toPxVector(position);

			mInternal->SetKinematicTarget(target);
		}
		else
		{
			setTransform(position, getRotation());
		}
	}

	void PhysXRigidbody::Rotate(const Quaternion& rotation)
	{
		if (getIsKinematic())
		{
			PxTransform target;
			if (!mInternal->GetKinematicTarget(target))
				target = PxTransform(PxIdentity);

			target.q = toPxQuaternion(rotation);

			mInternal->SetKinematicTarget(target);
		}
		else
		{
			setTransform(getPosition(), rotation);
		}
	}

	Vector3 PhysXRigidbody::GetPosition() const
	{
		return FromPxVector(mInternal->GetGlobalPose().p);
	}

	Quaternion PhysXRigidbody::GetRotation() const
	{
		return FromPxQuaternion(mInternal->GetGlobalPose().q);
	}

	void PhysXRigidbody::SetTransform(const Vector3& pos, const Quaternion& rot)
	{
		mInternal->SetGlobalPose(toPxTransform(pos, rot));
	}

	void PhysXRigidbody::SetMass(float mass)
	{
		if(((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoMass) != 0)
		{
			BS_LOG(Warning, Physics, "Attempting to set Rigidbody mass, but it has automatic mass calculation turned on.");
			return;
		}

		mInternal->SetMass(mass);
	}

	float PhysXRigidbody::GetMass() const
	{
		return mInternal->GetMass();
	}

	void PhysXRigidbody::SetIsKinematic(bool kinematic)
	{
		mInternal->SetRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kinematic);
	}

	bool PhysXRigidbody::GetIsKinematic() const
	{
		return ((UINT32)mInternal->GetRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC) != 0;
	}

	bool PhysXRigidbody::IsSleeping() const
	{
		return mInternal->IsSleeping();
	}

	void PhysXRigidbody::Sleep()
	{
		mInternal->PutToSleep();
	}

	void PhysXRigidbody::WakeUp()
	{
		mInternal->WakeUp();
	}

	void PhysXRigidbody::SetSleepThreshold(float threshold)
	{
		mInternal->SetSleepThreshold(threshold);
	}

	float PhysXRigidbody::GetSleepThreshold() const
	{
		return mInternal->GetSleepThreshold();
	}

	void PhysXRigidbody::SetUseGravity(bool gravity)
	{
		mInternal->SetActorFlag(PxActorFlag::eDISABLE_GRAVITY, !gravity);
	}

	bool PhysXRigidbody::GetUseGravity() const
	{
		return ((UINT32)mInternal->GetActorFlags() & PxActorFlag::eDISABLE_GRAVITY) == 0;
	}

	void PhysXRigidbody::SetVelocity(const Vector3& velocity)
	{
		mInternal->SetLinearVelocity(toPxVector(velocity));
	}

	Vector3 PhysXRigidbody::GetVelocity() const
	{
		return FromPxVector(mInternal->GetLinearVelocity());
	}

	void PhysXRigidbody::SetAngularVelocity(const Vector3& velocity)
	{
		mInternal->SetAngularVelocity(toPxVector(velocity));
	}

	Vector3 PhysXRigidbody::GetAngularVelocity() const
	{
		return FromPxVector(mInternal->GetAngularVelocity());
	}

	void PhysXRigidbody::SetDrag(float drag)
	{
		mInternal->SetLinearDamping(drag);
	}

	float PhysXRigidbody::GetDrag() const
	{
		return mInternal->GetLinearDamping();
	}

	void PhysXRigidbody::SetAngularDrag(float drag)
	{
		mInternal->SetAngularDamping(drag);
	}

	float PhysXRigidbody::GetAngularDrag() const
	{
		return mInternal->GetAngularDamping();
	}

	void PhysXRigidbody::SetInertiaTensor(const Vector3& tensor)
	{
		if (((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoTensors) != 0)
		{
			BS_LOG(Warning, Physics,
				"Attempting to set Rigidbody inertia tensor, but it has automatic tensor calculation turned on.");
			return;
		}

		mInternal->SetMassSpaceInertiaTensor(toPxVector(tensor));
	}

	Vector3 PhysXRigidbody::GetInertiaTensor() const
	{
		return FromPxVector(mInternal->GetMassSpaceInertiaTensor());
	}

	void PhysXRigidbody::SetMaxAngularVelocity(float maxVelocity)
	{
		mInternal->SetMaxAngularVelocity(maxVelocity);
	}

	float PhysXRigidbody::GetMaxAngularVelocity() const
	{
		return mInternal->GetMaxAngularVelocity();
	}

	void PhysXRigidbody::SetCenterOfMass(const Vector3& position, const Quaternion& rotation)
	{
		if (((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoTensors) != 0)
		{
			BS_LOG(Warning, Physics,
				"Attempting to set Rigidbody center of mass, but it has automatic tensor calculation turned on.");
			return;
		}

		mInternal->SetCMassLocalPose(toPxTransform(position, rotation));
	}

	Vector3 PhysXRigidbody::GetCenterOfMassPosition() const
	{
		PxTransform cMassTfrm = mInternal->GetCMassLocalPose();
		return FromPxVector(cMassTfrm.p);
	}

	Quaternion PhysXRigidbody::GetCenterOfMassRotation() const
	{
		PxTransform cMassTfrm = mInternal->GetCMassLocalPose();
		return FromPxQuaternion(cMassTfrm.q);
	}

	void PhysXRigidbody::SetPositionSolverCount(UINT32 count)
	{
		mInternal->SetSolverIterationCounts(std::max(1U, count), getVelocitySolverCount());
	}

	UINT32 PhysXRigidbody::GetPositionSolverCount() const
	{
		UINT32 posCount = 1;
		UINT32 velCount = 1;

		mInternal->GetSolverIterationCounts(posCount, velCount);
		return posCount;
	}

	void PhysXRigidbody::SetVelocitySolverCount(UINT32 count)
	{
		mInternal->SetSolverIterationCounts(getPositionSolverCount(), std::max(1U, count));
	}

	UINT32 PhysXRigidbody::GetVelocitySolverCount() const
	{
		UINT32 posCount = 1;
		UINT32 velCount = 1;

		mInternal->GetSolverIterationCounts(posCount, velCount);
		return velCount;
	}

	void PhysXRigidbody::SetFlags(RigidbodyFlag flags)
	{
		bool ccdEnabledOld = mInternal->GetRigidBodyFlags() & PxRigidBodyFlag::eENABLE_CCD;
		bool ccdEnabledNew = ((UINT32)flags & (UINT32)RigidbodyFlag::CCD) != 0;
		
		if(ccdEnabledOld != ccdEnabledNew)
		{
			mInternal->SetRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, ccdEnabledNew);

			// Enable/disable CCD on shapes so the filter can handle them properly
			UINT32 numShapes = mInternal->GetNbShapes();
			PxShape** shapes = (PxShape**)bs_stack_alloc(sizeof(PxShape*) * numShapes);

			mInternal->GetShapes(shapes, sizeof(PxShape*) * numShapes);

			for (UINT32 i = 0; i < numShapes; i++)
			{
				Collider* collider = (Collider*)shapes[i]->userData;
				collider->_getInternal()->_setCCD(ccdEnabledNew);
			}
		}

		Rigidbody::setFlags(flags);
	}

	void PhysXRigidbody::AddForce(const Vector3& force, ForceMode mode)
	{
		mInternal->AddForce(toPxVector(force), toPxForceMode(mode));
	}

	void PhysXRigidbody::AddTorque(const Vector3& force, ForceMode mode)
	{
		mInternal->AddTorque(toPxVector(force), toPxForceMode(mode));
	}

	void PhysXRigidbody::AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode)
	{
		const PxVec3& pxForce = toPxVector(force);
		const PxVec3& pxPos = toPxVector(position);

		const PxTransform globalPose = mInternal->GetGlobalPose();
		PxVec3 centerOfMass = globalPose.Transform(mInternal->GetCMassLocalPose().p);

		PxForceMode::Enum pxMode = toPxForceMode(mode);

		PxVec3 torque = (pxPos - centerOfMass).Cross(pxForce);
		mInternal->AddForce(pxForce, pxMode);
		mInternal->AddTorque(torque, pxMode);
	}

	Vector3 PhysXRigidbody::GetVelocityAtPoint(const Vector3& point) const
	{
		const PxVec3& pxPoint = toPxVector(point);

		const PxTransform globalPose = mInternal->GetGlobalPose();
		const PxVec3 centerOfMass = globalPose.Transform(mInternal->GetCMassLocalPose().p);
		const PxVec3 rpoint = pxPoint - centerOfMass;

		PxVec3 velocity = mInternal->GetLinearVelocity();
		velocity += mInternal->GetAngularVelocity().Cross(rpoint);

		return FromPxVector(velocity);
	}

	void PhysXRigidbody::UpdateMassDistribution()
	{
		if (((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoTensors) == 0)
			return;

		if (((UINT32)mFlags & (UINT32)RigidbodyFlag::AutoMass) == 0)
		{
			PxRigidBodyExt::setMassAndUpdateInertia(*mInternal, mInternal->GetMass());
		}
		else
		{
			UINT32 numShapes = mInternal->GetNbShapes();
			if (numShapes == 0)
			{
				PxRigidBodyExt::setMassAndUpdateInertia(*mInternal, mInternal->GetMass());
				return;
			}

			PxShape** shapes = (PxShape**)bs_stack_alloc(sizeof(PxShape*) * numShapes);
			mInternal->GetShapes(shapes, numShapes);

			float* masses = (float*)bs_stack_alloc(sizeof(float) * numShapes);
			for (UINT32 i = 0; i < numShapes; i++)
				masses[i] = ((Collider*)shapes[i]->userData)->GetMass();

			PxRigidBodyExt::setMassAndUpdateInertia(*mInternal, masses, numShapes);

			bs_stack_free(masses);
			bs_stack_free(shapes);
		}
	}

	void PhysXRigidbody::AddCollider(Collider* collider)
	{
		if (collider == nullptr)
			return;

		FPhysXCollider* physxCollider = static_cast<FPhysXCollider*>(collider->_getInternal());
		physxCollider->_setCCD(((UINT32)mFlags & (UINT32)RigidbodyFlag::CCD) != 0);

		mInternal->AttachShape(*physxCollider->_getShape());
	}

	void PhysXRigidbody::RemoveCollider(Collider* collider)
	{
		if (collider == nullptr)
			return;

		FPhysXCollider* physxCollider = static_cast<FPhysXCollider*>(collider->_getInternal());
		physxCollider->_setCCD(false);

		mInternal->DetachShape(*physxCollider->_getShape());
	}

	void PhysXRigidbody::RemoveColliders()
	{
		UINT32 numShapes = mInternal->GetNbShapes();
		PxShape** shapes = (PxShape**)bs_stack_alloc(sizeof(PxShape*) * numShapes);

		mInternal->GetShapes(shapes, sizeof(PxShape*) * numShapes);

		for (UINT32 i = 0; i < numShapes; i++)
		{
			Collider* collider = (Collider*)shapes[i]->userData;
			collider->_getInternal()->_setCCD(false);

			mInternal->DetachShape(*shapes[i]);
		}

		bs_stack_free(shapes);
	}
}
