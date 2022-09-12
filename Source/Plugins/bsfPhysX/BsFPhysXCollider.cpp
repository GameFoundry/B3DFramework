//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFPhysXCollider.h"
#include "BsPhysX.h"
#include "BsPhysXRigidbody.h"
#include "BsPhysXMaterial.h"
#include "PxScene.h"
#include "PxShape.h"

using namespace physx;

namespace bs
{
	FPhysXCollider::FPhysXCollider(PxScene* scene, PxShape* shape)
		:mScene(scene), mShape(shape)
	{
		mStaticBody = gPhysX().GetPhysX()->CreateRigidStatic(PxTransform(PxIdentity));
		mStaticBody->AttachShape(*mShape);

		mScene->AddActor(*mStaticBody);

		updateFilter();
	}

	FPhysXCollider::~FPhysXCollider()
	{
		if (mStaticBody != nullptr)
			mStaticBody->Release();

		mShape->userData = nullptr;
		mShape->Release();
	}

	void FPhysXCollider::_setShape(PxShape* shape)
	{
		if (mShape != nullptr)
		{
			shape->SetLocalPose(mShape->getLocalPose());
			shape->SetFlags(mShape->getFlags());
			shape->SetContactOffset(mShape->getContactOffset());
			shape->SetRestOffset(mShape->getRestOffset());

			UINT32 numMaterials = mShape->GetNbMaterials();
			UINT32 bufferSize = sizeof(PxMaterial*) * numMaterials;
			PxMaterial** materials = (PxMaterial**)bs_stack_alloc(bufferSize);

			mShape->GetMaterials(materials, bufferSize);
			shape->SetMaterials(materials, numMaterials);
			shape->userData = mShape->userData;

			bs_stack_free(materials);

			PxActor* actor = mShape->GetActor();
			if (actor != nullptr)
			{
				PxRigidActor* rigidActor = actor->is<PxRigidActor>();
				if (rigidActor != nullptr)
				{
					rigidActor->DetachShape(*mShape);
					rigidActor->AttachShape(*shape);
				}
			}
		}

		mShape = shape;

		updateFilter();
	}

	Vector3 FPhysXCollider::GetPosition() const
	{
		return FromPxVector(mShape->GetLocalPose().p);
	}

	Quaternion FPhysXCollider::GetRotation() const
	{
		return FromPxQuaternion(mShape->GetLocalPose().q);
	}

	void FPhysXCollider::SetTransform(const Vector3& pos, const Quaternion& rotation)
	{
		mShape->SetLocalPose(toPxTransform(pos, rotation));
	}

	void FPhysXCollider::SetIsTrigger(bool value)
	{
		if(value)
		{
			mShape->SetFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			mShape->SetFlag(PxShapeFlag::eTRIGGER_SHAPE, true);

			mIsTrigger = true;
		}
		else
		{
			mShape->SetFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
			mShape->SetFlag(PxShapeFlag::eSIMULATION_SHAPE, true);

			mIsTrigger = false;
		}		
	}

	bool FPhysXCollider::GetIsTrigger() const
	{
		return (UINT32)(mShape->GetFlags() & PxShapeFlag::eTRIGGER_SHAPE) != 0;
	}

	void FPhysXCollider::SetIsStatic(bool value)
	{
		if (mIsStatic == value)
			return;

		if (mStaticBody != nullptr)
		{
			mStaticBody->DetachShape(*mShape);

			mStaticBody->Release();
			mStaticBody = nullptr;
		}

		mIsStatic = value;

		if (mIsStatic)
		{
			mStaticBody = gPhysX().GetPhysX()->CreateRigidStatic(PxTransform(PxIdentity));
			mStaticBody->AttachShape(*mShape);

			mScene->AddActor(*mStaticBody);
		}
	}

	bool FPhysXCollider::GetIsStatic() const
	{
		return mIsStatic;
	}

	void FPhysXCollider::SetContactOffset(float value)
	{
		mShape->SetContactOffset(value);
	}

	float FPhysXCollider::GetContactOffset() const
	{
		return mShape->GetContactOffset();
	}

	void FPhysXCollider::SetRestOffset(float value)
	{
		mShape->SetRestOffset(value);
	}

	float FPhysXCollider::GetRestOffset() const
	{
		return mShape->GetRestOffset();
	}

	void FPhysXCollider::SetMaterial(const HPhysicsMaterial& material)
	{
		FCollider::setMaterial(material);

		PhysXMaterial* physXmaterial = nullptr;
		if(material.IsLoaded())
			physXmaterial = static_cast<PhysXMaterial*>(material.Get());

		PxMaterial* materials[1];
		if (physXmaterial != nullptr)
			materials[0] = physXmaterial->_getInternal();
		else
			materials[0] = gPhysX().GetDefaultMaterial();

		mShape->SetMaterials(materials, sizeof(materials) / sizeof(materials[0]));
	}

	UINT64 FPhysXCollider::GetLayer() const
	{
		return mLayer;
	}

	void FPhysXCollider::SetLayer(UINT64 layer)
	{
		mLayer = layer;
		updateFilter();
	}

	CollisionReportMode FPhysXCollider::GetCollisionReportMode() const
	{
		return mCollisionReportMode;
	}

	void FPhysXCollider::SetCollisionReportMode(CollisionReportMode mode)
	{
		mCollisionReportMode = mode;
		updateFilter();
	}

	void FPhysXCollider::_setCCD(bool enabled)
	{
		mCCD = enabled;
		updateFilter();
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

		if (mCCD)
			flags |= PhysXObjectFilterFlag::CCD;

		data.word2 = flags;

		mShape->SetSimulationFilterData(data);
		mShape->SetQueryFilterData(data);
	}
}
