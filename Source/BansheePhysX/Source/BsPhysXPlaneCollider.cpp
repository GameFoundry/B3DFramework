#include "BsPhysXPlaneCollider.h"
#include "BsPhysX.h"
#include "PxPhysics.h"
#include "BsFPhysXCollider.h"

using namespace physx;

namespace bs
{
	PhysXPlaneCollider::PhysXPlaneCollider(PxPhysics* physx, const Vector3& position, const Quaternion& rotation)
	{
		PxPlaneGeometry geometry;

		PxShape* shape = physx->createShape(geometry, *gPhysX().getDefaultMaterial(), true);
		shape->setLocalPose(toPxTransform(position, rotation));
		shape->userData = this;

		mInternal = bs_new<FPhysXCollider>(shape);
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