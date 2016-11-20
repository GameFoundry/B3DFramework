#include "BsPhysXSphereCollider.h"
#include "BsPhysX.h"
#include "PxPhysics.h"
#include "BsFPhysXCollider.h"

using namespace physx;

namespace bs
{
	PhysXSphereCollider::PhysXSphereCollider(PxPhysics* physx, const Vector3& position, const Quaternion& rotation,
		float radius)
		:mRadius(radius)
	{
		PxSphereGeometry geometry(radius);

		PxShape* shape = physx->createShape(geometry, *gPhysX().getDefaultMaterial(), true);
		shape->setLocalPose(toPxTransform(position, rotation));
		shape->userData = this;

		mInternal = bs_new<FPhysXCollider>(shape);
		applyGeometry();
	}

	PhysXSphereCollider::~PhysXSphereCollider()
	{
		bs_delete(mInternal);
	}

	void PhysXSphereCollider::setScale(const Vector3& scale)
	{
		SphereCollider::setScale(scale);
		applyGeometry();
	}

	void PhysXSphereCollider::setRadius(float radius)
	{
		mRadius = radius;
		applyGeometry();
	}

	float PhysXSphereCollider::getRadius() const
	{
		return mRadius;
	}

	void PhysXSphereCollider::applyGeometry()
	{
		float radius = std::max(0.01f, mRadius * std::max(std::max(mScale.x, mScale.y), mScale.z));
		PxSphereGeometry geometry(radius);

		getInternal()->_getShape()->setGeometry(geometry);
	}

	FPhysXCollider* PhysXSphereCollider::getInternal() const
	{
		return static_cast<FPhysXCollider*>(mInternal);
	}
}