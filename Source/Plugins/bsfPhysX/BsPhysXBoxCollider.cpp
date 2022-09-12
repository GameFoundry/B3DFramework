//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXBoxCollider.h"
#include "BsPhysX.h"
#include "PxPhysics.h"
#include "BsFPhysXCollider.h"

using namespace physx;

namespace bs
{
	PhysXBoxCollider::PhysXBoxCollider(PxPhysics* physx, PxScene* scene, const Vector3& position,
		const Quaternion& rotation, const Vector3& extents)
		:mExtents(extents)
	{
		PxBoxGeometry Geometry(extents.x, extents.y, extents.z);

		PxShape* shape = physx->CreateShape(geometry, *gPhysX().GetDefaultMaterial(), true);
		shape->SetLocalPose(toPxTransform(position, rotation));
		shape->userData = this;

		mInternal = bs_new<FPhysXCollider>(scene, shape);
		applyGeometry();
	}

	PhysXBoxCollider::~PhysXBoxCollider()
	{
		bs_delete(mInternal);
	}

	void PhysXBoxCollider::SetScale(const Vector3& scale)
	{
		BoxCollider::setScale(scale);
		applyGeometry();
	}

	void PhysXBoxCollider::SetExtents(const Vector3& extents)
	{
		mExtents = extents;
		applyGeometry();
	}

	Vector3 PhysXBoxCollider::GetExtents() const
	{
		return mExtents;
	}

	void PhysXBoxCollider::ApplyGeometry()
	{
		PxBoxGeometry Geometry(std::max(0.01f, mExtents.x * mScale.x),
			std::max(0.01f, mExtents.y * mScale.y), std::max(0.01f, mExtents.z * mScale.z));

		getInternal()->_getShape()->SetGeometry(geometry);
	}

	FPhysXCollider* PhysXBoxCollider::getInternal() const
	{
		return static_cast<FPhysXCollider*>(mInternal);
	}
}
