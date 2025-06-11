//************************************ bs::framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXColliderShape.h"

#include <PxRigidDynamic.h>

#include "BsPhysX.h"
#include "BsPhysXCollider.h"
#include "BsPhysXRigidbody.h"
#include "BsPhysXMaterial.h"
#include "BsPhysXMesh.h"
#include "PxScene.h"
#include "PxShape.h"
#include "Physics/BsCollider.h"
#include "Physics/BsPhysicsMesh.h"
#include "Scene/BsTransform.h"

using namespace physx;
using namespace bs;

PhysXColliderShape::~PhysXColliderShape()
{
	DestroyShape();
}

void PhysXColliderShape::SetPosition(const Vector3& position)
{
	Super::SetPosition(position);
	UpdateTransform();
}

void PhysXColliderShape::SetRotation(const Quaternion& rotation)
{
	Super::SetRotation(rotation);
	UpdateTransform();
}

void PhysXColliderShape::SetScale(const Vector3& scale)
{
	Super::SetScale(scale);
	UpdateTransform();
}

void PhysXColliderShape::SetContactOffset(float value)
{
	mShape->setContactOffset(value);
	Super::SetContactOffset(value);
}

void PhysXColliderShape::SetRestOffset(float value)
{
	mShape->setRestOffset(value);
	Super::SetRestOffset(value);
}

void PhysXColliderShape::SetMaterial(const HPhysicsMaterial& material)
{
	Super::SetMaterial(material);

	PhysXMaterial* physXmaterial = nullptr;
	if(material.IsValid())
		physXmaterial = B3DRTTICast<PhysXMaterial>(material.Get());

	PxMaterial* materials[1];
	if(physXmaterial != nullptr)
		materials[0] = physXmaterial->GetPxMaterial();
	else
		materials[0] = GetPhysX().GetDefaultMaterial();

	mShape->setMaterials(materials, sizeof(materials) / sizeof(materials[0]));
}

void PhysXColliderShape::SetIsTrigger(bool value)
{
	if(value)
	{
		mShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		mShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
	}
	else
	{
		mShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
		mShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	}

	Super::SetIsTrigger(value);
}

void PhysXColliderShape::SetLayer(u64 layer)
{
	Super::SetLayer(layer);
	SetFilter();
}

void PhysXColliderShape::SetCollisionReportMode(CollisionReportMode mode)
{
	Super::SetCollisionReportMode(mode);
	SetFilter();
}

void PhysXColliderShape::SetContinuousCollisionDetection(bool value)
{
	Super::SetContinuousCollisionDetection(value);
	SetFilter();
}

void PhysXColliderShape::SetShape(const PlaneColliderShapeInformation& information)
{
	mShapeInformation = information;
	mShapeType = ColliderShapeType::Plane;

	const PxPlaneGeometry geometry;
	SetGeometry(geometry);
}

void PhysXColliderShape::SetShape(const BoxColliderShapeInformation& information)
{
	mShapeInformation = information;
	mShapeType = ColliderShapeType::Box;

	const PxBoxGeometry geometry(ToPxVector(Vector3::Max(information.Extents * mWorldSpaceScale, Vector3(0.0001f, 0.0001f, 0.0001f))));
	SetGeometry(geometry);
}

void PhysXColliderShape::SetShape(const SphereColliderShapeInformation& information)
{
	mShapeInformation = information;
	mShapeType = ColliderShapeType::Sphere;

	const PxSphereGeometry geometry(Math::Max(information.Radius * mWorldSpaceScale.X, 0.0001f));
	SetGeometry(geometry);
}

void PhysXColliderShape::SetShape(const CapsuleColliderShapeInformation& information)
{
	mShapeInformation = information;
	mShapeType = ColliderShapeType::Capsule;

	const PxCapsuleGeometry geometry(Math::Max(information.Radius * mWorldSpaceScale.X, 0.0001f), Math::Max(information.HalfHeight * mWorldSpaceScale.Y, 0.0001f));
	SetGeometry(geometry);
}

void PhysXColliderShape::SetShape(const MeshColliderShapeInformation& information)
{
	mShapeInformation = information;
	mShapeType = ColliderShapeType::Mesh;

	const HPhysicsMesh& mesh = information.Mesh;
	if(!mesh.IsValid())
		return;

	FPhysXMesh* underlyingMesh = static_cast<FPhysXMesh*>(mesh->GetInternal());
	if(!B3D_ENSURE(underlyingMesh != nullptr))
		return;

	if(mesh->GetType() == PhysicsMeshType::Convex)
	{
		PxConvexMeshGeometry geometry;
		geometry.scale = PxMeshScale(ToPxVector(mWorldSpaceScale), PxIdentity);
		geometry.convexMesh = underlyingMesh->GetPxConvexMesh();

		SetGeometry(geometry);
	}
	else
	{
		PxTriangleMeshGeometry geometry;
		geometry.scale = PxMeshScale(ToPxVector(mWorldSpaceScale), PxIdentity);
		geometry.triangleMesh = underlyingMesh->GetPxTriangleMesh();

		SetGeometry(geometry);
	}
}

void PhysXColliderShape::SetGeometry(const physx::PxGeometry& geometry)
{
	if(mShape == nullptr || mShape->getGeometryType() != geometry.getType())
	{
		PhysXMaterial* material = nullptr;
		if (mMaterial.IsValid())
			material = B3DRTTICast<PhysXMaterial>(mMaterial.Get());

		PxMaterial* underlyingMaterial;
		if(material != nullptr)
			underlyingMaterial = material->GetPxMaterial();
		else
			underlyingMaterial = GetPhysX().GetDefaultMaterial();

		PxShape* const newShape = GetPhysX().GetPhysX()->createShape(geometry, *underlyingMaterial, true);
		newShape->userData = this;

		if(B3D_ENSURE(newShape != nullptr))
		{
			newShape->setLocalPose(mShape->getLocalPose());
			newShape->setFlags(mShape->getFlags());
			newShape->setContactOffset(mShape->getContactOffset());
			newShape->setRestOffset(mShape->getRestOffset());

			const u16 materialCount = mShape->getNbMaterials();
			const u32 bufferSize = sizeof(PxMaterial*) * materialCount;

			{
				StackMemory<PxMaterial*> materials = B3DManagedStackAllocate<PxMaterial*>(materialCount);

				mShape->getMaterials(materials, bufferSize);
				newShape->setMaterials(materials, materialCount);
				newShape->userData = mShape->userData;
			}

			PxActor* actor = mShape->getActor();
			if(actor != nullptr)
			{
				PxRigidActor* rigidActor = actor->is<PxRigidActor>();
				if(rigidActor != nullptr)
				{
					rigidActor->detachShape(*mShape);
					rigidActor->attachShape(*newShape);
				}
			}
		}

		DestroyShape();
		mShape = newShape;

		SetFilter();
	}
	else
	{
		mShape->setGeometry(geometry);
	}
}

void PhysXColliderShape::UpdateTransform()
{
	Transform relativeTransform(mPosition, mRotation, mScale);

	if (mParentCollider != nullptr)
	{
		const Rigidbody* const rigidbody = mParentCollider->GetRigidbody();

		if (rigidbody != nullptr)
		{
			const Transform parentTransform(mParentCollider->GetPosition(), mParentCollider->GetRotation(), mParentCollider->GetScale());
			relativeTransform.MakeWorld(parentTransform);
		}
		else
		{
			const Transform parentTransform(Vector3::kZero, Quaternion::kIdentity, mParentCollider->GetScale());
			relativeTransform.MakeWorld(parentTransform);
		}
	}

	mShape->setLocalPose(ToPxTransform(relativeTransform.GetPosition(), relativeTransform.GetRotation()));

	if (mWorldSpaceScale != relativeTransform.GetScale())
	{
		mWorldSpaceScale = relativeTransform.GetScale();
		RecreateShape();
	}
}

void PhysXColliderShape::SetFilter()
{
	PxFilterData data;
	memcpy(&data.word0, &mLayer, sizeof(mLayer));

	PhysXObjectFilterFlags flags;

	switch(mCollisionReportMode)
	{
	case CollisionReportMode::None:
		flags |= PhysXObjectFilterFlag::NoReport;
		break;
	case CollisionReportMode::Report:
		flags |= PhysXObjectFilterFlag::ReportBasic;
		break;
	case CollisionReportMode::ReportPersistent:
		flags |= PhysXObjectFilterFlag::ReportAll;
		break;
	}

	if(mContinuousCollisionDetectionEnabled)
		flags |= PhysXObjectFilterFlag::CCD;

	data.word2 = flags;

	mShape->setSimulationFilterData(data);
	mShape->setQueryFilterData(data);
}

void PhysXColliderShape::RecreateShape()
{
	switch(mShapeType)
	{
	default:
	case ColliderShapeType::Plane:
		SetShape(std::get<PlaneColliderShapeInformation>(mShapeInformation));
		break;
	case ColliderShapeType::Box:
		SetShape(std::get<BoxColliderShapeInformation>(mShapeInformation));
		break;
	case ColliderShapeType::Sphere:
		SetShape(std::get<SphereColliderShapeInformation>(mShapeInformation));
		break;
	case ColliderShapeType::Capsule:
		SetShape(std::get<CapsuleColliderShapeInformation>(mShapeInformation));
		break;
	case ColliderShapeType::Mesh:
		SetShape(std::get<MeshColliderShapeInformation>(mShapeInformation));
		break;
	}
}

void PhysXColliderShape::DestroyShape()
{
	if(mShape == nullptr)
		return;

	mShape->userData = nullptr;
	mShape->release();
	mShape = nullptr;
}

void PhysXColliderShape::AttachToCollider(Collider& collider)
{
	if(!B3D_ENSURE(mParentCollider == nullptr))
		return;

	const Rigidbody* const rigidbody = collider.GetRigidbody();
	if (rigidbody != nullptr)
	{
		const PhysXRigidbody* const physxRigidbody = static_cast<const PhysXRigidbody*>(rigidbody);
		PxRigidDynamic* const dynamicBody = physxRigidbody->GetPxRigidDynamic();

		const u32 rigidbodyFlags = (u32)rigidbody->GetFlags();
		SetContinuousCollisionDetection((rigidbodyFlags & (u32)RigidbodyFlag::CCD) != 0);

		dynamicBody->attachShape(*mShape);
	}
	else
	{
		const PhysXCollider& physxCollider = static_cast<PhysXCollider&>(collider);
		PxRigidStatic* const staticBody = physxCollider.GetPxRigidStatic();

		staticBody->attachShape(*mShape);
	}

	mParentCollider = &collider;
	UpdateTransform();
}

void PhysXColliderShape::DetachFromCollider()
{
	if(mParentCollider == nullptr)
		return;

	const Rigidbody* const rigidbody = mParentCollider->GetRigidbody();
	if (rigidbody != nullptr)
	{
		const PhysXRigidbody* const physxRigidbody = static_cast<const PhysXRigidbody*>(rigidbody);
		PxRigidDynamic* const dynamicBody = physxRigidbody->GetPxRigidDynamic();

		SetContinuousCollisionDetection(false);
		dynamicBody->detachShape(*mShape);
	}
	else
	{
		const PhysXCollider& physxCollider = static_cast<const PhysXCollider&>(*mParentCollider);
		PxRigidStatic* const staticBody = physxCollider.GetPxRigidStatic();

		staticBody->detachShape(*mShape);
	}

	mParentCollider = nullptr;
	UpdateTransform();
}
