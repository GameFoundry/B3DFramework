//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsFCollider.h"
#include "Physics/BsRigidbody.h"

using namespace bs;

void FCollider::SetMaterial(const HPhysicsMaterial& material)
{
	mMaterial = material;
}
