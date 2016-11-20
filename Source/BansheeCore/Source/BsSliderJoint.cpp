//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSliderJoint.h"
#include "BsPhysics.h"

namespace bs
{
	SPtr<SliderJoint> SliderJoint::create(const SLIDER_JOINT_DESC& desc)
	{
		return gPhysics().createSliderJoint(desc);
	}
}