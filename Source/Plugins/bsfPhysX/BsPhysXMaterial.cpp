//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXMaterial.h"
#include "PxPhysics.h"

namespace bs
{
	PhysXMaterial::PhysXMaterial(physx::PxPhysics* physx, float staFric, float dynFriction, float restitution)
		:mInternal(nullptr)
	{
		mInternal = physx->CreateMaterial(staFric, dynFriction, restitution);
	}

	PhysXMaterial::~PhysXMaterial()
	{
		mInternal->Release();
	}

	void PhysXMaterial::SetStaticFriction(float value)
	{
		mInternal->SetStaticFriction(value);
	}

	float PhysXMaterial::GetStaticFriction() const
	{
		return mInternal->GetStaticFriction();
	}

	void PhysXMaterial::SetDynamicFriction(float value)
	{
		mInternal->SetDynamicFriction(value);
	}

	float PhysXMaterial::GetDynamicFriction() const
	{
		return mInternal->GetDynamicFriction();
	}

	void PhysXMaterial::SetRestitutionCoefficient(float value)
	{
		mInternal->SetRestitution(value);
	}

	float PhysXMaterial::GetRestitutionCoefficient() const
	{
		return mInternal->GetRestitution();
	}
}
