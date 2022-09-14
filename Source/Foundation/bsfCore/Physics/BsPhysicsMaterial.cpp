//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsPhysicsMaterial.h"
#include "Private/RTTI/BsPhysicsMaterialRTTI.h"
#include "Resources/BsResources.h"
#include "Physics/BsPhysics.h"

namespace bs
{
	HPhysicsMaterial PhysicsMaterial::create(float staticFriction, float dynamicFriction, float restitution)
	{
		SPtr<PhysicsMaterial> newMaterial = CreatePtrInternal(staticFriction, dynamicFriction, restitution);

		return static_resource_cast<PhysicsMaterial>(gResources().CreateResourceHandleInternal(newMaterial));
	}

	SPtr<PhysicsMaterial> PhysicsMaterial::CreatePtrInternal(float staticFriction, float dynamicFriction, float restitution)
	{
		SPtr<PhysicsMaterial> newMaterial = gPhysics().createMaterial(staticFriction, dynamicFriction, restitution);
		newMaterial->SetThisPtrInternal(newMaterial);
		newMaterial->initialize();

		return newMaterial;
	}

	RTTITypeBase* PhysicsMaterial::getRTTIStatic()
	{
		return PhysicsMaterialRTTI::instance();
	}

	RTTITypeBase* PhysicsMaterial::getRTTI() const
	{
		return getRTTIStatic();
	}
}
