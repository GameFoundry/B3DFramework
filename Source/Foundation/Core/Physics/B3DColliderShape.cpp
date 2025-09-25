//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/B3DColliderShape.h"
#include "B3DPhysics.h"
#include "RTTI/B3DColliderShapeRTTI.h"

using namespace b3d;

ColliderShape::ColliderShape()
	:mIsTrigger(false), mContinuousCollisionDetectionEnabled(false)
{ }

PlaneColliderShapeInformation ColliderShape::GetPlaneShapeInformation() const
{
	if(std::holds_alternative<PlaneColliderShapeInformation>(mShapeInformation))
		return std::get<PlaneColliderShapeInformation>(mShapeInformation);

	return PlaneColliderShapeInformation();
}

BoxColliderShapeInformation ColliderShape::GetBoxShapeInformation() const
{
	if(std::holds_alternative<BoxColliderShapeInformation>(mShapeInformation))
		return std::get<BoxColliderShapeInformation>(mShapeInformation);

	return BoxColliderShapeInformation();
}

SphereColliderShapeInformation ColliderShape::GetSphereShapeInformation() const
{
	if(std::holds_alternative<SphereColliderShapeInformation>(mShapeInformation))
		return std::get<SphereColliderShapeInformation>(mShapeInformation);

	return SphereColliderShapeInformation();
}

CapsuleColliderShapeInformation ColliderShape::GetCapsuleShapeInformation() const
{
	if(std::holds_alternative<CapsuleColliderShapeInformation>(mShapeInformation))
		return std::get<CapsuleColliderShapeInformation>(mShapeInformation);

	return CapsuleColliderShapeInformation();
}

MeshColliderShapeInformation ColliderShape::GetMeshShapeInformation() const
{
	if(std::holds_alternative<MeshColliderShapeInformation>(mShapeInformation))
		return std::get<MeshColliderShapeInformation>(mShapeInformation);

	return MeshColliderShapeInformation();
}

SPtr<ColliderShape> ColliderShape::CreatePlane(const PlaneColliderShapeInformation& information)
{
	SPtr<ColliderShape> shape = GetPhysics().CreateColliderShape();
	shape->SetShape(information);

	return shape;
}

SPtr<ColliderShape> ColliderShape::CreateBox(const BoxColliderShapeInformation& information)
{
	SPtr<ColliderShape> shape = GetPhysics().CreateColliderShape();
	shape->SetShape(information);

	return shape;
}

SPtr<ColliderShape> ColliderShape::CreateSphere(const SphereColliderShapeInformation& information)
{
	SPtr<ColliderShape> shape = GetPhysics().CreateColliderShape();
	shape->SetShape(information);

	return shape;
}

SPtr<ColliderShape> ColliderShape::CreateCapsule(const CapsuleColliderShapeInformation& information)
{
	SPtr<ColliderShape> shape = GetPhysics().CreateColliderShape();
	shape->SetShape(information);

	return shape;
}

SPtr<ColliderShape> ColliderShape::CreateMesh(const MeshColliderShapeInformation& information)
{
	SPtr<ColliderShape> shape = GetPhysics().CreateColliderShape();
	shape->SetShape(information);

	return shape;
}

SPtr<ColliderShape> ColliderShape::CreateEmpty()
{
	return GetPhysics().CreateColliderShape();
}

RTTIType* ColliderShape::GetRttiStatic()
{
	return ColliderShapeRTTI::Instance();
}

RTTIType* ColliderShape::GetRtti() const
{
	return ColliderShape::GetRttiStatic();
}

