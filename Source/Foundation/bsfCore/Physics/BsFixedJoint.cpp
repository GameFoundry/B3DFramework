//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsFixedJoint.h"
#include "Physics/BsPhysics.h"

namespace bs
{
	SPtr<FixedJoint> FixedJoint::create(PhysicsScene& scene, const FIXED_JOINT_DESC& desc)
	{
		return scene.createFixedJoint(desc);
	}
}