//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/B3DPhysicsMaterial.h"
#include "RTTI/B3DPhysicsMaterialRTTI.h"
#include "Resources/B3DResources.h"
#include "Physics/B3DPhysics.h"

using namespace b3d;

HPhysicsMaterial PhysicsMaterial::Create(float staticFriction, float dynamicFriction, float restitution)
{
	TShared<PhysicsMaterial> newMaterial = CreateShared(staticFriction, dynamicFriction, restitution);

	return B3DStaticResourceCast<PhysicsMaterial>(GetResources().CreateResourceHandle(newMaterial));
}

TShared<PhysicsMaterial> PhysicsMaterial::CreateShared(float staticFriction, float dynamicFriction, float restitution)
{
	TShared<PhysicsMaterial> newMaterial = GetPhysics().CreateMaterial(staticFriction, dynamicFriction, restitution);
	newMaterial->SetShared(newMaterial);
	newMaterial->Initialize();

	return newMaterial;
}

RTTIType* PhysicsMaterial::GetRttiStatic()
{
	return PhysicsMaterialRTTI::Instance();
}

RTTIType* PhysicsMaterial::GetRtti() const
{
	return GetRttiStatic();
}
