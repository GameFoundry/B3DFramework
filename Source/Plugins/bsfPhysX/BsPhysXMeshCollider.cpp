//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXMeshCollider.h"
#include "BsPhysX.h"
#include "PxPhysics.h"
#include "BsFPhysXCollider.h"
#include "BsPhysXMesh.h"

using namespace physx;

using namespace bs;

PhysXMeshCollider::PhysXMeshCollider(PxPhysics* physx, PxScene* scene, const Vector3& position, const Quaternion& rotation)
{
	PxSphereGeometry geometry(0.01f); // Dummy

	PxShape* shape = physx->createShape(geometry, *GetPhysX().GetDefaultMaterial(), true);
	shape->setLocalPose(toPxTransform(position, rotation));
	shape->userData = this;

	mInternal = B3DNew<FPhysXCollider>(scene, shape);
}

PhysXMeshCollider::~PhysXMeshCollider()
{
	B3DDelete(mInternal);
}

void PhysXMeshCollider::SetScale(const Vector3& scale)
{
	MeshCollider::SetScale(scale);
	ApplyGeometry();
}

void PhysXMeshCollider::OnMeshChanged()
{
	ApplyGeometry();
}

void PhysXMeshCollider::ApplyGeometry()
{
	if(!mMesh.IsLoaded())
	{
		SetGeometry(PxSphereGeometry(0.01f)); // Dummy
		return;
	}

	FPhysXMesh* physxMesh = static_cast<FPhysXMesh*>(mMesh->GetInternalInternal());

	if(mMesh->GetType() == PhysicsMeshType::Convex)
	{
		PxConvexMeshGeometry geometry;
		geometry.scale = PxMeshScale(toPxVector(GetScale()), PxIdentity);
		geometry.convexMesh = physxMesh->GetConvexInternal();

		SetGeometry(geometry);
	}
	else // Triangle
	{
		PxTriangleMeshGeometry geometry;
		geometry.scale = PxMeshScale(toPxVector(GetScale()), PxIdentity);
		geometry.triangleMesh = physxMesh->GetTriangleInternal();

		SetGeometry(geometry);
	}
}

void PhysXMeshCollider::SetGeometry(const PxGeometry& geometry)
{
	PxShape* shape = GetInternal()->GetShapeInternal();
	if(shape->getGeometryType() != geometry.getType())
	{
		PxShape* newShape = GetPhysX().GetPhysX()->createShape(geometry, *GetPhysX().GetDefaultMaterial(), true);
		GetInternal()->SetShapeInternal(newShape);
	}
	else
		GetInternal()->GetShapeInternal()->setGeometry(geometry);
}

FPhysXCollider* PhysXMeshCollider::GetInternal() const
{
	return static_cast<FPhysXCollider*>(mInternal);
}
