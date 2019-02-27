//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsD6Joint.h"
#include "Physics/BsPhysics.h"

namespace bs
{
	SPtr<D6Joint> D6Joint::create(PhysicsScene& scene, const D6_JOINT_DESC& desc)
	{
		return scene.createD6Joint(desc);
	}
}