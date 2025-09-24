//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullPhysicsPrerequisites.h"
#include "Physics/BsPhysicsManager.h"
#include "BsNullPhysics.h"

using namespace b3d;

class B3D_PLUGIN_EXPORT NullPhysicsFactory : public PhysicsFactory
{
public:
	void StartUp(bool cooking)
	{
		PHYSICS_INIT_DESC initDesc;
		initDesc.InitCooking = cooking;

		Physics::StartUp<NullPhysics>(initDesc);
	}

	void ShutDown()
	{
		Physics::ShutDown();
	}
};

extern "C" B3D_PLUGIN_EXPORT NullPhysicsFactory* LoadPlugin()
{
	return B3DNew<NullPhysicsFactory>();
}

extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin(NullPhysicsFactory* instance)
{
	B3DDelete(instance);
}
