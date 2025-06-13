//************************************ bs::framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXCollider.h"
#include "BsPhysX.h"
#include "BsPhysXRigidbody.h"
#include "BsPhysXMaterial.h"
#include "PxScene.h"

using namespace physx;
using namespace bs;

PhysXCollider::PhysXCollider(PhysXScene& physicsScene, const Vector3& position, const Quaternion& rotation, const Vector3& scale)
	: Collider(physicsScene, position, rotation, scale)
{
	CreateStaticBody();
}

PhysXCollider::~PhysXCollider()
{
	for(auto& entry : mShapes)
		entry->DetachFromCollider();

	DestroyStaticBody();
}

void PhysXCollider::SetShapes(const TArrayView<SPtr<ColliderShape>>& shapes)
{
	PhysXScene& physXScene = static_cast<PhysXScene&>(mPhysicsScene);
	PxScene& pxScene = physXScene.GetPxScene();

	if(mStaticBody != nullptr)
		pxScene.removeActor(*mStaticBody);

	Collider::SetShapes(shapes);
	
	if(mStaticBody != nullptr)
		pxScene.addActor(*mStaticBody);
}

void PhysXCollider::SetRigidbody(Rigidbody* rigidbody)
{
	if(mRigidbody == rigidbody)
		return;

	for(auto& entry : mShapes)
		entry->DetachFromCollider();

	if(rigidbody != nullptr)
		DestroyStaticBody();
	else
	{
		if(mStaticBody == nullptr)
			CreateStaticBody();
	}

	mRigidbody = rigidbody;
	
	for(auto& entry : mShapes)
		entry->AttachToCollider(*this);

	if(mStaticBody != nullptr)
	{
		// Add body to scene after adding the shapes
		PhysXScene& physXScene = static_cast<PhysXScene&>(mPhysicsScene);
		PxScene& pxScene = physXScene.GetPxScene();

		pxScene.addActor(*mStaticBody);
	}
}

void PhysXCollider::UpdateTransform()
{
	Collider::UpdateTransform();

	if(mStaticBody != nullptr)
		mStaticBody->setGlobalPose(ToPxTransform(mPosition, mRotation));
}

void PhysXCollider::CreateStaticBody()
{
	if(!B3D_ENSURE(mStaticBody == nullptr))
		return;

	mStaticBody = GetPhysX().GetPhysX()->createRigidStatic(PxTransform(PxIdentity));
	mStaticBody->setGlobalPose(ToPxTransform(mPosition, mRotation));
}

void PhysXCollider::DestroyStaticBody()
{
	if(mStaticBody != nullptr)
	{
		PhysXScene& physXScene = static_cast<PhysXScene&>(mPhysicsScene);
		PxScene& pxScene = physXScene.GetPxScene();

		pxScene.removeActor(*mStaticBody);

		mStaticBody->release();
		mStaticBody = nullptr;
	}
}
