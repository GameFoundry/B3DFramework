//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Physics/BsPhysicsMaterial.h"
#include "Private/RTTI/BsPhysicsMaterialRTTI.h"
#include "Resources/BsResources.h"
#include "Physics/BsPhysics.h"

namespace bs
{
	HPhysicsMaterial PhysicsMaterial::create(float staticFriction, float dynamicFriction, float restitution)
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
		return PhysicsMaterialRTTI::instance();
	}

	RTTITypeBase* PhysicsMaterial::getRTTI() const
	{
		return getRTTIStatic();
	}
}