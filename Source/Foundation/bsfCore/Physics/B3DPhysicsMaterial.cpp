//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsPhysicsMaterial.h"
#include "Private/RTTI/BsPhysicsMaterialRTTI.h"
#include "Resources/BsResources.h"
#include "Physics/BsPhysics.h"

using namespace b3d;

HPhysicsMaterial PhysicsMaterial::Create(float staticFriction, float dynamicFriction, float restitution)
{
	SPtr<PhysicsMaterial> newMaterial = CreatePtrInternal(staticFriction, dynamicFriction, restitution);

	return B3DStaticResourceCast<PhysicsMaterial>(GetResources().CreateResourceHandle(newMaterial));
}

SPtr<PhysicsMaterial> PhysicsMaterial::CreatePtrInternal(float staticFriction, float dynamicFriction, float restitution)
{
	SPtr<PhysicsMaterial> newMaterial = GetPhysics().CreateMaterial(staticFriction, dynamicFriction, restitution);
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
