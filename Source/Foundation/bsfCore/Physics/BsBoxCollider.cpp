//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsBoxCollider.h"
#include "Physics/BsPhysics.h"

using namespace bs;

SPtr<BoxCollider> BoxCollider::Create(PhysicsScene& scene, const Vector3& extents, const Vector3& position, const Quaternion& rotation)
{
	return scene.CreateBoxCollider(extents, position, rotation);
}
