#include "BsPhysXCapsuleCollider.h"
#include "BsPhysX.h"
#include "PxPhysics.h"
#include "BsFPhysXCollider.h"

using namespace physx;

namespace bs
{
	PhysXCapsuleCollider::PhysXCapsuleCollider(PxPhysics* physx, const Vector3& position, const Quaternion& rotation,
		float radius, float halfHeight)
		:mRadius(radius), mHalfHeight(halfHeight)
	{
		PxCapsuleGeometry geometry(radius, halfHeight);

		PxShape* shape = physx->createShape(geometry, *gPhysX().getDefaultMaterial(), true);
		shape->setLocalPose(toPxTransform(position, rotation));
		shape->userData = this;

		mInternal = bs_new<FPhysXCollider>(shape);
		applyGeometry();
	}

	PhysXCapsuleCollider::~PhysXCapsuleCollider()
	{
		bs_delete(mInternal);
	}

	void PhysXCapsuleCollider::setScale(const Vector3& scale)
	{
		CapsuleCollider::setScale(scale);
		applyGeometry();
	}

	void PhysXCapsuleCollider::setHalfHeight(float halfHeight)
	{
		mHalfHeight = halfHeight;
		applyGeometry();
	}

	float PhysXCapsuleCollider::getHalfHeight() const
	{
		return mHalfHeight;
	}

	void PhysXCapsuleCollider::setRadius(float radius)
	{
		mRadius = radius;
		applyGeometry();
	}

	float PhysXCapsuleCollider::getRadius() const
	{
		return mRadius;
	}

	void PhysXCapsuleCollider::applyGeometry()
	{
		PxCapsuleGeometry geometry(std::max(0.01f, mRadius * std::max(mScale.x, mScale.z)), 
			std::max(0.01f, mHalfHeight * mScale.y));

		getInternal()->_getShape()->setGeometry(geometry);
	}

	FPhysXCollider* PhysXCapsuleCollider::getInternal() const
	{
		return static_cast<FPhysXCollider*>(mInternal);
	}
}