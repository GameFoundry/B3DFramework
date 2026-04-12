//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullPhysicsPrerequisites.h"
#include "Physics/B3DPhysicsManager.h"
#include "B3DNullPhysics.h"

using namespace b3d;

class B3D_PLUGIN_EXPORT NullPhysicsFactory : public PhysicsFactory
{
public:
	void StartUp(bool cooking)
	{
		PhysicsCreateInformation createInformation;
		createInformation.InitCooking = cooking;

		Physics::StartUp<NullPhysics>(createInformation);
	}

	void ShutDown()
	{
		Physics::ShutDown();
	}
};

extern "C" void* LoadPlugin_bsfNullPhysics()
{
	return static_cast<void*>(B3DNew<NullPhysicsFactory>());
}

extern "C" void UnloadPlugin_bsfNullPhysics(void* instance)
{
	B3DDelete(static_cast<PhysicsFactory*>(instance));
}

extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	return LoadPlugin_bsfNullPhysics();
}

extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin(NullPhysicsFactory* instance)
{
	UnloadPlugin_bsfNullPhysics(static_cast<void*>(instance));
}
