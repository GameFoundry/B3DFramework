#include "BsPhysXMaterial.h"
#include "PxPhysics.h"

namespace bs
{
	PhysXMaterial::PhysXMaterial(physx::PxPhysics* physx, float staFric, float dynFriction, float restitution)
		:mInternal(nullptr)
	{
		mInternal = physx->createMaterial(staFric, dynFriction, restitution);
	}

	PhysXMaterial::~PhysXMaterial()
	{
		mInternal->release();
	}

	void PhysXMaterial::setStaticFriction(float value)
	{
		mInternal->setStaticFriction(value);
	}

	float PhysXMaterial::getStaticFriction() const
	{
		return mInternal->getStaticFriction();
	}

	void PhysXMaterial::setDynamicFriction(float value)
	{
		mInternal->setDynamicFriction(value);
	}

	float PhysXMaterial::getDynamicFriction() const
	{
		return mInternal->getDynamicFriction();
	}

	void PhysXMaterial::setRestitutionCoefficient(float value)
	{
		mInternal->setRestitution(value);
	}

	float PhysXMaterial::getRestitutionCoefficient() const
	{
		return mInternal->getRestitution();
	}
}