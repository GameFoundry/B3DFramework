//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsPhysicsMaterial.h"
#include "Private/RTTI/BsPhysicsMaterialRTTI.h"
#include "Resources/BsResources.h"
#include "Physics/BsPhysics.h"

namespace bs
{
	HPhysicsMaterial PhysicsMaterial::Create(float staticFriction, float dynamicFriction, float restitution)
	{
		SPtr<PhysicsMaterial> newMaterial = _createPtr(staticFriction, dynamicFriction, restitution);

		return static_resource_cast<PhysicsMaterial>(gResources()._createResourceHandle(newMaterial));
	}

	SPtr<PhysicsMaterial> PhysicsMaterial::_createPtr(float staticFriction, float dynamicFriction, float restitution)
	{
		SPtr<PhysicsMaterial> newMaterial = gPhysics().createMaterial(staticFriction, dynamicFriction, restitution);
		newMaterial->_setThisPtr(newMaterial);
		newMaterial->initialize();

		return newMaterial;
	}

	RTTITypeBase* PhysicsMaterial::getRTTIStatic()
	{
		return PhysicsMaterialRTTI::Instance();
	}

	RTTITypeBase* PhysicsMaterial::getRTTI() const
	{
		return GetRTTIStatic();
	}
}
