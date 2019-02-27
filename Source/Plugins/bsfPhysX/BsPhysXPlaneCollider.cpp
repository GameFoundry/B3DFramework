//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXPlaneCollider.h"
#include "BsPhysX.h"
#include "PxPhysics.h"
#include "BsFPhysXCollider.h"

using namespace physx;

namespace bs
{
	PhysXPlaneCollider::PhysXPlaneCollider(PxPhysics* physx, PxScene* scene, const Vector3& position, 
		const Quaternion& rotation)
	{
		PxPlaneGeometry geometry;

		PxShape* shape = physx->createShape(geometry, *gPhysX().getDefaultMaterial(), true);
		shape->setLocalPose(toPxTransform(position, rotation));
		shape->userData = this;

		mInternal = bs_new<FPhysXCollider>(scene, shape);
	}

	PhysXPlaneCollider::~PhysXPlaneCollider()
	{
		bs_delete(mInternal);
	}

	FPhysXCollider* PhysXPlaneCollider::getInternal() const
	{
		return static_cast<FPhysXCollider*>(mInternal);
	}
}