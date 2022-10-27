//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXCapsuleCollider.h"
#include "BsPhysX.h"
#include "PxPhysics.h"
#include "BsFPhysXCollider.h"

using namespace physx;

using namespace bs;

PhysXCapsuleCollider::PhysXCapsuleCollider(PxPhysics* physx, PxScene* scene, const Vector3& position, const Quaternion& rotation, float radius, float halfHeight)
	: mRadius(radius), mHalfHeight(halfHeight)
{
	PxCapsuleGeometry geometry(radius, halfHeight);

	PxShape* shape = physx->createShape(geometry, *gPhysX().GetDefaultMaterial(), true);
	shape->setLocalPose(toPxTransform(position, rotation));
	shape->userData = this;

	mInternal = bs_new<FPhysXCollider>(scene, shape);
	ApplyGeometry();
}

PhysXCapsuleCollider::~PhysXCapsuleCollider()
{
	bs_delete(mInternal);
}

void PhysXCapsuleCollider::SetScale(const Vector3& scale)
{
	CapsuleCollider::SetScale(scale);
	ApplyGeometry();
}

void PhysXCapsuleCollider::SetHalfHeight(float halfHeight)
{
	mHalfHeight = halfHeight;
	ApplyGeometry();
}

float PhysXCapsuleCollider::GetHalfHeight() const
{
	return mHalfHeight;
}

void PhysXCapsuleCollider::SetRadius(float radius)
{
	mRadius = radius;
	ApplyGeometry();
}

float PhysXCapsuleCollider::GetRadius() const
{
	return mRadius;
}

void PhysXCapsuleCollider::ApplyGeometry()
{
	PxCapsuleGeometry geometry(std::max(0.01f, mRadius * std::max(mScale.X, mScale.Z)), std::max(0.01f, mHalfHeight * mScale.Y));

	GetInternal()->GetShapeInternal()->setGeometry(geometry);
}

FPhysXCollider* PhysXCapsuleCollider::GetInternal() const
{
	return static_cast<FPhysXCollider*>(mInternal);
}
