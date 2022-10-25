//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsSphereCollider.h"
#include "Physics/BsPhysics.h"

using namespace bs;

SPtr<SphereCollider> SphereCollider::Create(PhysicsScene& scene, float radius, const Vector3& position, const Quaternion& rotation)
{
	return scene.CreateSphereCollider(radius, position, rotation);
}
