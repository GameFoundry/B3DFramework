//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXSphereCollider.h"
#include "BsPhysX.h"
#include "PxPhysics.h"
#include "BsFPhysXCollider.h"

using namespace physx;

using namespace bs;

PhysXSphereCollider::PhysXSphereCollider(PxPhysics* physx, PxScene* scene, const Vector3& position, const Quaternion& rotation, float radius)
	: mRadius(radius)
{
	PxSphereGeometry geometry(radius);

	PxShape* shape = physx->createShape(geometry, *GetPhysX().GetDefaultMaterial(), true);
	shape->setLocalPose(toPxTransform(position, rotation));
	shape->userData = this;

	mInternal = bs_new<FPhysXCollider>(scene, shape);
	ApplyGeometry();
}

PhysXSphereCollider::~PhysXSphereCollider()
{
	bs_delete(mInternal);
}

void PhysXSphereCollider::SetScale(const Vector3& scale)
{
	SphereCollider::SetScale(scale);
	ApplyGeometry();
}

void PhysXSphereCollider::SetRadius(float radius)
{
	mRadius = radius;
	ApplyGeometry();
}

float PhysXSphereCollider::GetRadius() const
{
	return mRadius;
}

void PhysXSphereCollider::ApplyGeometry()
{
	float radius = std::max(0.01f, mRadius * std::max(std::max(mScale.X, mScale.Y), mScale.Z));
	PxSphereGeometry geometry(radius);

	GetInternal()->GetShapeInternal()->setGeometry(geometry);
}

FPhysXCollider* PhysXSphereCollider::GetInternal() const
{
	return static_cast<FPhysXCollider*>(mInternal);
}
