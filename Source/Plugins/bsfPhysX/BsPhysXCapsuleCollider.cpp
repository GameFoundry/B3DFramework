//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXCapsuleCollider.h"
#include "BsPhysX.h"
#include "PxPhysics.h"
#include "BsFPhysXCollider.h"

using namespace physx;

namespace bs
{
	PhysXCapsuleCollider::PhysXCapsuleCollider(PxPhysics* physx, PxScene* scene, const Vector3& position,
		const Quaternion& rotation, float radius, float halfHeight)
		:mRadius(radius), mHalfHeight(halfHeight)
	{
		PxCapsuleGeometry Geometry(radius, halfHeight);

		PxShape* shape = physx->createShape(geometry, *gPhysX().getDefaultMaterial(), true);
		shape->setLocalPose(toPxTransform(position, rotation));
		shape->userData = this;

		mInternal = bs_new<FPhysXCollider>(scene, shape);
		applyGeometry();
	}

	PhysXCapsuleCollider::~PhysXCapsuleCollider()
	{
		bs_delete(mInternal);
	}

	void PhysXCapsuleCollider::SetScale(const Vector3& scale)
	{
		CapsuleCollider::setScale(scale);
		applyGeometry();
	}

	void PhysXCapsuleCollider::SetHalfHeight(float halfHeight)
	{
		mHalfHeight = halfHeight;
		applyGeometry();
	}

	float PhysXCapsuleCollider::GetHalfHeight() const
	{
		return mHalfHeight;
	}

	void PhysXCapsuleCollider::SetRadius(float radius)
	{
		mRadius = radius;
		applyGeometry();
	}

	float PhysXCapsuleCollider::GetRadius() const
	{
		return mRadius;
	}

	void PhysXCapsuleCollider::ApplyGeometry()
	{
		PxCapsuleGeometry Geometry(std::max(0.01f, mRadius * std::max(mScale.x, mScale.z)),
			std::max(0.01f, mHalfHeight * mScale.y));

		getInternal()->_getShape()->setGeometry(geometry);
	}

	FPhysXCollider* PhysXCapsuleCollider::getInternal() const
	{
		return static_cast<FPhysXCollider*>(mInternal);
	}
}
