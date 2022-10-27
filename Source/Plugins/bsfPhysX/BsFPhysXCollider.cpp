//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFPhysXCollider.h"
#include "BsPhysX.h"
#include "BsPhysXRigidbody.h"
#include "BsPhysXMaterial.h"
#include "PxScene.h"
#include "PxShape.h"

using namespace physx;

using namespace bs;

FPhysXCollider::FPhysXCollider(PxScene* scene, PxShape* shape)
	: mScene(scene), mShape(shape)
{
	mStaticBody = gPhysX().GetPhysX()->createRigidStatic(PxTransform(PxIdentity));
	mStaticBody->attachShape(*mShape);

	mScene->addActor(*mStaticBody);

	UpdateFilter();
}

FPhysXCollider::~FPhysXCollider()
{
	if(mStaticBody != nullptr)
		mStaticBody->release();

	mShape->userData = nullptr;
	mShape->release();
}

void FPhysXCollider::SetShapeInternal(PxShape* shape)
{
	if(mShape != nullptr)
	{
		shape->setLocalPose(mShape->getLocalPose());
		shape->setFlags(mShape->getFlags());
		shape->setContactOffset(mShape->getContactOffset());
		shape->setRestOffset(mShape->getRestOffset());

		u32 numMaterials = mShape->getNbMaterials();
		u32 bufferSize = sizeof(PxMaterial*) * numMaterials;
		PxMaterial** materials = (PxMaterial**)bs_stack_alloc(bufferSize);

		mShape->getMaterials(materials, bufferSize);
		shape->setMaterials(materials, numMaterials);
		shape->userData = mShape->userData;

		bs_stack_free(materials);

		PxActor* actor = mShape->getActor();
		if(actor != nullptr)
		{
			PxRigidActor* rigidActor = actor->is<PxRigidActor>();
			if(rigidActor != nullptr)
			{
				rigidActor->detachShape(*mShape);
				rigidActor->attachShape(*shape);
			}
		}
	}

	mShape = shape;

	UpdateFilter();
}

Vector3 FPhysXCollider::GetPosition() const
{
	return fromPxVector(mShape->getLocalPose().p);
}

Quaternion FPhysXCollider::GetRotation() const
{
	return fromPxQuaternion(mShape->getLocalPose().q);
}

void FPhysXCollider::SetTransform(const Vector3& pos, const Quaternion& rotation)
{
	mShape->setLocalPose(toPxTransform(pos, rotation));
}

void FPhysXCollider::SetIsTrigger(bool value)
{
	if(value)
	{
		mShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		mShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);

		mIsTrigger = true;
	}
	else
	{
		mShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
		mShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);

		mIsTrigger = false;
	}
}

bool FPhysXCollider::GetIsTrigger() const
{
	return (u32)(mShape->getFlags() & PxShapeFlag::eTRIGGER_SHAPE) != 0;
}

void FPhysXCollider::SetIsStatic(bool value)
{
	if(mIsStatic == value)
		return;

	if(mStaticBody != nullptr)
	{
		mStaticBody->detachShape(*mShape);

		mStaticBody->release();
		mStaticBody = nullptr;
	}

	mIsStatic = value;

	if(mIsStatic)
	{
		mStaticBody = gPhysX().GetPhysX()->createRigidStatic(PxTransform(PxIdentity));
		mStaticBody->attachShape(*mShape);

		mScene->addActor(*mStaticBody);
	}
}

bool FPhysXCollider::GetIsStatic() const
{
	return mIsStatic;
}

void FPhysXCollider::SetContactOffset(float value)
{
	mShape->setContactOffset(value);
}

float FPhysXCollider::GetContactOffset() const
{
	return mShape->getContactOffset();
}

void FPhysXCollider::SetRestOffset(float value)
{
	mShape->setRestOffset(value);
}

float FPhysXCollider::GetRestOffset() const
{
	return mShape->getRestOffset();
}

void FPhysXCollider::SetMaterial(const HPhysicsMaterial& material)
{
	FCollider::SetMaterial(material);

	PhysXMaterial* physXmaterial = nullptr;
	if(material.IsLoaded())
		physXmaterial = static_cast<PhysXMaterial*>(material.Get());

	PxMaterial* materials[1];
	if(physXmaterial != nullptr)
		materials[0] = physXmaterial->GetInternalInternal();
	else
		materials[0] = gPhysX().GetDefaultMaterial();

	mShape->setMaterials(materials, sizeof(materials) / sizeof(materials[0]));
}

u64 FPhysXCollider::GetLayer() const
{
	return mLayer;
}

void FPhysXCollider::SetLayer(u64 layer)
{
	mLayer = layer;
	UpdateFilter();
}

CollisionReportMode FPhysXCollider::GetCollisionReportMode() const
{
	return mCollisionReportMode;
}

void FPhysXCollider::SetCollisionReportMode(CollisionReportMode mode)
{
	mCollisionReportMode = mode;
	UpdateFilter();
}

void FPhysXCollider::SetCCDInternal(bool enabled)
{
	mCCD = enabled;
	UpdateFilter();
}

void FPhysXCollider::UpdateFilter()
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

	if(mCCD)
		flags |= PhysXObjectFilterFlag::CCD;

	data.word2 = flags;

	mShape->setSimulationFilterData(data);
	mShape->setQueryFilterData(data);
}
