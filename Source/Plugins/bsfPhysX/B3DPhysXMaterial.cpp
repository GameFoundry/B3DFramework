//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPhysXMaterial.h"
#include "PxPhysics.h"

using namespace b3d;

PhysXMaterial::PhysXMaterial(physx::PxPhysics* physx, float staFric, float dynFriction, float restitution)
	: mInternal(nullptr)
{
	mInternal = physx->createMaterial(staFric, dynFriction, restitution);
}

PhysXMaterial::~PhysXMaterial()
{
	mInternal->release();
}

void PhysXMaterial::SetStaticFriction(float value)
{
	mInternal->setStaticFriction(value);
}

float PhysXMaterial::GetStaticFriction() const
{
	return mInternal->getStaticFriction();
}

void PhysXMaterial::SetDynamicFriction(float value)
{
	mInternal->setDynamicFriction(value);
}

float PhysXMaterial::GetDynamicFriction() const
{
	return mInternal->getDynamicFriction();
}

void PhysXMaterial::SetRestitutionCoefficient(float value)
{
	mInternal->setRestitution(value);
}

float PhysXMaterial::GetRestitutionCoefficient() const
{
	return mInternal->getRestitution();
}
