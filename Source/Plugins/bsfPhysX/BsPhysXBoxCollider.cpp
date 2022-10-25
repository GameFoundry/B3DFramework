//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXBoxCollider.h"
#include "BsPhysX.h"
#include "PxPhysics.h"
#include "BsFPhysXCollider.h"

using namespace physx;

namespace bs
{
	PhysXBoxCollider::PhysXBoxCollider(PxPhysics* physx, PxScene* scene, const Vector3& position, const Quaternion& rotation, const Vector3& extents)
		: mExtents(extents)
	{
		PxBoxGeometry geometry(extents.X, extents.Y, extents.Z);

		PxShape* shape = physx->createShape(geometry, *gPhysX().GetDefaultMaterial(), true);
		shape->setLocalPose(toPxTransform(position, rotation));
		shape->userData = this;

		mInternal = bs_new<FPhysXCollider>(scene, shape);
		ApplyGeometry();
	}

	PhysXBoxCollider::~PhysXBoxCollider()
	{
		bs_delete(mInternal);
	}

	void PhysXBoxCollider::SetScale(const Vector3& scale)
	{
		BoxCollider::SetScale(scale);
		ApplyGeometry();
	}

	void PhysXBoxCollider::SetExtents(const Vector3& extents)
	{
		mExtents = extents;
		ApplyGeometry();
	}

	Vector3 PhysXBoxCollider::GetExtents() const
	{
		return mExtents;
	}

	void PhysXBoxCollider::ApplyGeometry()
	{
		PxBoxGeometry geometry(std::max(0.01f, mExtents.X * mScale.X), std::max(0.01f, mExtents.Y * mScale.Y), std::max(0.01f, mExtents.Z * mScale.Z));

		GetInternal()->GetShapeInternal()->setGeometry(geometry);
	}

	FPhysXCollider* PhysXBoxCollider::GetInternal() const
	{
		return static_cast<FPhysXCollider*>(mInternal);
	}
} // namespace bs
