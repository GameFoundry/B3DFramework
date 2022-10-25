//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsPhysicsMaterial.h"
#include "Private/RTTI/BsPhysicsMaterialRTTI.h"
#include "Resources/BsResources.h"
#include "Physics/BsPhysics.h"

using namespace bs;

HPhysicsMaterial PhysicsMaterial::Create(float staticFriction, float dynamicFriction, float restitution)
{
	SPtr<PhysicsMaterial> newMaterial = CreatePtrInternal(staticFriction, dynamicFriction, restitution);

	return static_resource_cast<PhysicsMaterial>(gResources().CreateResourceHandleInternal(newMaterial));
}

SPtr<PhysicsMaterial> PhysicsMaterial::CreatePtrInternal(float staticFriction, float dynamicFriction, float restitution)
{
	SPtr<PhysicsMaterial> newMaterial = gPhysics().CreateMaterial(staticFriction, dynamicFriction, restitution);
	newMaterial->SetThisPtrInternal(newMaterial);
	newMaterial->Initialize();

	return newMaterial;
}

RTTITypeBase* PhysicsMaterial::GetRttiStatic()
{
	return PhysicsMaterialRTTI::Instance();
}

RTTITypeBase* PhysicsMaterial::GetRtti() const
{
	return GetRttiStatic();
}
