//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFCollider.h"
#include "BsRigidbody.h"

namespace bs
{
	FCollider::~FCollider()
	{ }

	void FCollider::setMaterial(const HPhysicsMaterial& material)
	{
		mMaterial = material;
	}
}