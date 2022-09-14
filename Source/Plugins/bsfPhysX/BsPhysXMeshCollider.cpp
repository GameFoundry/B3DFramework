//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXMeshCollider.h"
#include "BsPhysX.h"
#include "PxPhysics.h"
#include "BsFPhysXCollider.h"
#include "BsPhysXMesh.h"

using namespace physx;

namespace bs
{
	PhysXMeshCollider::PhysXMeshCollider(PxPhysics* physx, PxScene* scene, const Vector3& position,
		const Quaternion& rotation)
	{
		PxSphereGeometry geometry(0.01f); // Dummy

		PxShape* shape = physx->createShape(geometry, *gPhysX().getDefaultMaterial(), true);
		shape->setLocalPose(toPxTransform(position, rotation));
		shape->userData = this;

		mInternal = bs_new<FPhysXCollider>(scene, shape);
	}

	PhysXMeshCollider::~PhysXMeshCollider()
	{
		bs_delete(mInternal);
	}

	void PhysXMeshCollider::setScale(const Vector3& scale)
	{
		MeshCollider::setScale(scale);
		applyGeometry();
	}

	void PhysXMeshCollider::onMeshChanged()
	{
		applyGeometry();
	}

	void PhysXMeshCollider::applyGeometry()
	{
		if (!mMesh.isLoaded())
		{
			setGeometry(PxSphereGeometry(0.01f)); // Dummy
			return;
		}

		FPhysXMesh* physxMesh = static_cast<FPhysXMesh*>(mMesh->GetInternalInternal());

		if (mMesh->getType() == PhysicsMeshType::Convex)
		{
			PxConvexMeshGeometry geometry;
			geometry.scale = PxMeshScale(toPxVector(getScale()), PxIdentity);
			geometry.convexMesh = physxMesh->GetConvexInternal();

			setGeometry(geometry);
		}
		else // Triangle
		{
			PxTriangleMeshGeometry geometry;
			geometry.scale = PxMeshScale(toPxVector(getScale()), PxIdentity);
			geometry.triangleMesh = physxMesh->GetTriangleInternal();

			setGeometry(geometry);
		}
	}

	void PhysXMeshCollider::setGeometry(const PxGeometry& geometry)
	{
		PxShape* shape = getInternal()->GetShapeInternal();
		if (shape->getGeometryType() != geometry.getType())
		{
			PxShape* newShape = gPhysX().getPhysX()->createShape(geometry, *gPhysX().getDefaultMaterial(), true);
			getInternal()->SetShapeInternal(newShape);
		}
		else
			getInternal()->GetShapeInternal()->setGeometry(geometry);
	}

	FPhysXCollider* PhysXMeshCollider::getInternal() const
	{
		return static_cast<FPhysXCollider*>(mInternal);
	}
}
