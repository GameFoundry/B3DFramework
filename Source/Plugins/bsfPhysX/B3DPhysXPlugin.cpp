//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPhysXPrerequisites.h"
#include "Physics/B3DPhysicsManager.h"
#include "B3DPhysX.h"

using namespace b3d;

class B3D_PLUGIN_EXPORT PhysXFactory : public PhysicsFactory
{
public:
	void StartUp(bool cooking)
	{
		PhysicsCreateInformation desc;
		desc.InitCooking = cooking;

		Physics::StartUp<PhysX>(desc);
	}

	void ShutDown()
	{
		Physics::ShutDown();
	}
};

extern "C" void* LoadPlugin_bsfPhysX()
{
	return static_cast<void*>(B3DNew<PhysXFactory>());
}

extern "C" void UnloadPlugin_bsfPhysX(void* instance)
{
	B3DDelete(static_cast<PhysicsFactory*>(instance));
}

extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	return LoadPlugin_bsfPhysX();
}

extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin(PhysXFactory* instance)
{
	UnloadPlugin_bsfPhysX(static_cast<void*>(instance));
}
