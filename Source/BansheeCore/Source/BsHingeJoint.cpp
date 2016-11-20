//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsHingeJoint.h"
#include "BsPhysics.h"

namespace bs
{
	SPtr<HingeJoint> HingeJoint::create(const HINGE_JOINT_DESC& desc)
	{
		return gPhysics().createHingeJoint(desc);
	}
}