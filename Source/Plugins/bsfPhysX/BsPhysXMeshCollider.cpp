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
		PxSphereGeometry Geometry(0.01f); // Dummy

		PxShape* shape = physx->CreateShape(geometry, *gPhysX().GetDefaultMaterial(), true);
		shape->SetLocalPose(toPxTransform(position, rotation));
		shape->userData = this;

		mInternal = bs_new<FPhysXCollider>(scene, shape);
	}

	PhysXMeshCollider::~PhysXMeshCollider()
	{
		bs_delete(mInternal);
	}

	void PhysXMeshCollider::SetScale(const Vector3& scale)
	{
		MeshCollider::setScale(scale);
		applyGeometry();
	}

	void PhysXMeshCollider::OnMeshChanged()
	{
		applyGeometry();
	}

	void PhysXMeshCollider::ApplyGeometry()
	{
		if (!mMesh.IsLoaded())
		{
			setGeometry(PxSphereGeometry(0.01f)); // Dummy
			return;
		}

		FPhysXMesh* physxMesh = static_cast<FPhysXMesh*>(mMesh->_getInternal());

		if (mMesh->GetType() == PhysicsMeshType::Convex)
		{
			PxConvexMeshGeometry geometry;
			geometry.scale = PxMeshScale(toPxVector(getScale()), PxIdentity);
			geometry.convexMesh = physxMesh->_getConvex();

			setGeometry(geometry);
		}
		else // Triangle
		{
			PxTriangleMeshGeometry geometry;
			geometry.scale = PxMeshScale(toPxVector(getScale()), PxIdentity);
			geometry.triangleMesh = physxMesh->_getTriangle();

			setGeometry(geometry);
		}
	}

	void PhysXMeshCollider::SetGeometry(const PxGeometry& geometry)
	{
		PxShape* shape = getInternal()->_getShape();
		if (shape->GetGeometryType() != geometry.GetType())
		{
			PxShape* newShape = gPhysX().GetPhysX()->CreateShape(geometry, *gPhysX().getDefaultMaterial(), true);
			getInternal()->_setShape(newShape);
		}
		else
			getInternal()->_getShape()->SetGeometry(geometry);
	}

	FPhysXCollider* PhysXMeshCollider::getInternal() const
	{
		return static_cast<FPhysXCollider*>(mInternal);
	}
}
