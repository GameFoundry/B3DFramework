//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsCapsuleCollider.h"
#include "Physics/BsPhysics.h"

using namespace bs;

SPtr<CapsuleCollider> CapsuleCollider::Create(PhysicsScene& scene, float radius, float halfHeight, const Vector3& position, const Quaternion& rotation)
{
	return scene.CreateCapsuleCollider(radius, halfHeight, position, rotation);
}
