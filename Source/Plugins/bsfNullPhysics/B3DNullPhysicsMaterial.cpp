//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullPhysicsMaterial.h"

using namespace b3d;

NullPhysicsMaterial::NullPhysicsMaterial(float staFric, float dynFriction, float restitution)
	: mStaticFriction(staFric), mDynamicFriction(dynFriction), mRestitutionCoefficient(restitution)
{
}
