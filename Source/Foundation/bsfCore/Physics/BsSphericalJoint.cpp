//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsSphericalJoint.h"
#include "Physics/BsPhysics.h"

namespace bs
{
	SPtr<SphericalJoint> SphericalJoint::create(PhysicsScene& scene, const SPHERICAL_JOINT_DESC& desc)
	{
		return scene.createSphericalJoint(desc);
	}
}