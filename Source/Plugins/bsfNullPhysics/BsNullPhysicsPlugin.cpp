//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullPhysicsPrerequisites.h"
#include "Physics/BsPhysicsManager.h"
#include "BsNullPhysics.h"

using namespace bs;

class BS_PLUGIN_EXPORT NullPhysicsFactory : public PhysicsFactory
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

extern "C" BS_PLUGIN_EXPORT NullPhysicsFactory* loadPlugin()
{
	return B3DNew<NullPhysicsFactory>();
}

extern "C" BS_PLUGIN_EXPORT void unloadPlugin(NullPhysicsFactory* instance)
{
	B3DDelete(instance);
}
