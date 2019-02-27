//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsDistanceJoint.h"
#include "Physics/BsPhysics.h"

namespace bs
{
	SPtr<DistanceJoint> DistanceJoint::create(PhysicsScene& scene, const DISTANCE_JOINT_DESC& desc)
	{
		return scene.createDistanceJoint(desc);
	}
}