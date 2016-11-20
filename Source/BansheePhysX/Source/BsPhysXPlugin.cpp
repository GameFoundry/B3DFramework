#include "BsPhysXPrerequisites.h"
#include "BsPhysicsManager.h"
#include "BsPhysX.h"

namespace bs
{
	class BS_PHYSX_EXPORT PhysXFactory : public PhysicsFactory
	{
	public:
		void startUp(bool cooking) override
		{
			PHYSICS_INIT_DESC desc;
			desc.initCooking = cooking;

			Physics::startUp<PhysX>(desc);
		}

		void shutDown() override
		{
			Physics::shutDown();
		}
	};

	extern "C" BS_PHYSX_EXPORT PhysXFactory* loadPlugin()
	{
		return bs_new<PhysXFactory>();
	}

	extern "C" BS_PHYSX_EXPORT void unloadPlugin(PhysXFactory* instance)
	{
		bs_delete(instance);
	}
}