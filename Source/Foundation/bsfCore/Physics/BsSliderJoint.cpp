//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsSliderJoint.h"
#include "Physics/BsPhysics.h"

namespace bs
{
	SPtr<SliderJoint> SliderJoint::create(PhysicsScene& scene, const SLIDER_JOINT_DESC& desc)
	{
		return scene.createSliderJoint(desc);
	}
}