//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXPrerequisites.h"
#include "Physics/BsPhysicsManager.h"
#include "BsPhysX.h"

using namespace bs;

class BS_PLUGIN_EXPORT PhysXFactory : public PhysicsFactory
{
public:
	void StartUp(bool cooking)
	{
		PHYSICS_INIT_DESC desc;
		desc.InitCooking = cooking;

		Physics::StartUp<PhysX>(desc);
	}

	void ShutDown()
	{
		Physics::ShutDown();
	}
};

extern "C" BS_PLUGIN_EXPORT PhysXFactory* loadPlugin()
{
	return B3DNew<PhysXFactory>();
}

extern "C" BS_PLUGIN_EXPORT void unloadPlugin(PhysXFactory* instance)
{
	B3DDelete(instance);
}
