//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXSphereCollider.h"
#include "BsPhysX.h"
#include "PxPhysics.h"
#include "BsFPhysXCollider.h"

using namespace physx;

namespace bs
{
	PhysXSphereCollider::PhysXSphereCollider(PxPhysics* physx, PxScene* scene, const Vector3& position,
		const Quaternion& rotation, float radius)
		:mRadius(radius)
	{
		PxSphereGeometry Geometry(radius);

		PxShape* shape = physx->CreateShape(geometry, *gPhysX().GetDefaultMaterial(), true);
		shape->SetLocalPose(toPxTransform(position, rotation));
		shape->userData = this;

		mInternal = bs_new<FPhysXCollider>(scene, shape);
		applyGeometry();
	}

	PhysXSphereCollider::~PhysXSphereCollider()
	{
		bs_delete(mInternal);
	}

	void PhysXSphereCollider::SetScale(const Vector3& scale)
	{
		SphereCollider::setScale(scale);
		applyGeometry();
	}

	void PhysXSphereCollider::SetRadius(float radius)
	{
		mRadius = radius;
		applyGeometry();
	}

	float PhysXSphereCollider::GetRadius() const
	{
		return mRadius;
	}

	void PhysXSphereCollider::ApplyGeometry()
	{
		float radius = std::max(0.01f, mRadius * std::max(std::max(mScale.x, mScale.y), mScale.z));
		PxSphereGeometry Geometry(radius);

		getInternal()->_getShape()->SetGeometry(geometry);
	}

	FPhysXCollider* PhysXSphereCollider::getInternal() const
	{
		return static_cast<FPhysXCollider*>(mInternal);
	}
}
