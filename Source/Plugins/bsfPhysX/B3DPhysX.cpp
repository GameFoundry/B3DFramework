//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPhysX.h"

#include "B3DApplication.h"
#include "PxPhysicsAPI.h"
#include "B3DPhysXMaterial.h"
#include "B3DPhysXMesh.h"
#include "B3DPhysXRigidbody.h"
#include "B3DPhysXFixedJoint.h"
#include "B3DPhysXDistanceJoint.h"
#include "B3DPhysXHingeJoint.h"
#include "B3DPhysXSphericalJoint.h"
#include "B3DPhysXSliderJoint.h"
#include "B3DPhysXD6Joint.h"
#include "B3DPhysXCharacterController.h"
#include "Components/B3DCollider.h"
#include "B3DPhysXCollider.h"
#include "B3DPhysXColliderShape.h"
#include "Components/B3DRigidbody.h"
#include "Utility/B3DTime.h"
#include "Math/B3DVector3.h"
#include "Math/B3DAABox.h"
#include "Math/B3DCapsule.h"
#include "foundation/PxTransform.h"
#include "Scene/B3DSceneObject.h"

using namespace physx;

namespace b3d {
class PhysXAllocator : public PxAllocatorCallback
{
public:
	void* allocate(size_t size, const char*, const char*, int) override
	{
		void* ptr = B3DAllocateAligned16((u32)size);
		PX_ASSERT((reinterpret_cast<size_t>(ptr) & 15) == 0);
		return ptr;
	}

	void deallocate(void* ptr) override
	{
		B3DFreeAligned16(ptr);
	}
};

class PhysXErrorCallback : public PxErrorCallback
{
public:
	void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line) override
	{
		const char* errorCode = nullptr;

		u32 severity = 0;

		if((code & PxErrorCode::eDEBUG_INFO) != 0)
		{
			errorCode = "Info";
			severity = 0;
		}

		if((code & PxErrorCode::eINVALID_PARAMETER) != 0)
		{
			errorCode = "Invalid parameter";
			severity = 1;
		}

		if((code & PxErrorCode::eINVALID_OPERATION) != 0)
		{
			errorCode = "Invalid operation";
			severity = 1;
		}

		if((code & PxErrorCode::eDEBUG_WARNING) != 0)
		{
			errorCode = "Generic";
			severity = 1;
		}

		if((code & PxErrorCode::ePERF_WARNING) != 0)
		{
			errorCode = "Performance";
			severity = 1;
		}

		if((code & PxErrorCode::eOUT_OF_MEMORY) != 0)
		{
			errorCode = "Out of memory";
			severity = 2;
		}

		if((code & PxErrorCode::eABORT) != 0)
		{
			errorCode = "Abort";
			severity = 2;
		}

		if((code & PxErrorCode::eINTERNAL_ERROR) != 0)
		{
			errorCode = "Internal";
			severity = 2;
		}

		StringStream ss;

		switch(severity)
		{
		case 0:
			ss << "PhysX info (" << errorCode << "): " << message << " at " << file << ":" << line;
			B3D_LOG(Info, LogPhysics, "{0}", ss.str());
			break;
		case 1:
			ss << "PhysX warning (" << errorCode << "): " << message << " at " << file << ":" << line;
			B3D_LOG(Warning, LogPhysics, "{0}", ss.str());
			break;
		case 2:
			ss << "PhysX error (" << errorCode << "): " << message << " at " << file << ":" << line;
			B3D_LOG(Error, LogPhysics, "{0}", ss.str());
			B3D_ASSERT(false); // Halt execution on debug builds when error occurs
			break;
		}
	}
};

void PhysXEventCallback::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
	for(PxU32 i = 0; i < count; i++)
	{
		const PxTriggerPair& pair = pairs[i];
		if(pair.triggerShape->userData == nullptr)
			continue;

		PhysXScene::ContactEventType type;
		bool ignoreContact = false;
		PhysXObjectFilterFlags flags = PhysXObjectFilterFlags(pair.triggerShape->getSimulationFilterData().word2);

		if(flags.IsSet(PhysXObjectFilterFlag::ReportAll))
		{
			switch((u32)pair.status)
			{
			case PxPairFlag::eNOTIFY_TOUCH_FOUND:
				type = PhysXScene::ContactEventType::ContactBegin;
				break;
			case PxPairFlag::eNOTIFY_TOUCH_PERSISTS:
				type = PhysXScene::ContactEventType::ContactStay;
				break;
			case PxPairFlag::eNOTIFY_TOUCH_LOST:
				type = PhysXScene::ContactEventType::ContactEnd;
				break;
			default:
				ignoreContact = true;
				break;
			}
		}
		else if(flags.IsSet(PhysXObjectFilterFlag::ReportBasic))
		{
			switch((u32)pair.status)
			{
			case PxPairFlag::eNOTIFY_TOUCH_FOUND:
				type = PhysXScene::ContactEventType::ContactBegin;
				break;
			case PxPairFlag::eNOTIFY_TOUCH_LOST:
				type = PhysXScene::ContactEventType::ContactEnd;
				break;
			default:
				ignoreContact = true;
				break;
			}
		}
		else
			ignoreContact = true;

		if(ignoreContact)
			continue;

		PhysXScene::TriggerEvent event;
		event.Trigger = (ColliderShape*)pair.triggerShape->userData;
		event.Other = (ColliderShape*)pair.otherShape->userData;
		event.Type = type;

		mScene.ReportTriggerEvent(event);
	}
}

void PhysXEventCallback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 count)
{
	for(PxU32 i = 0; i < count; i++)
	{
		const PxContactPair& pair = pairs[i];

		PhysXScene::ContactEventType type;
		bool ignoreContact = false;
		switch((u32)pair.events)
		{
		case PxPairFlag::eNOTIFY_TOUCH_FOUND:
			type = PhysXScene::ContactEventType::ContactBegin;
			break;
		case PxPairFlag::eNOTIFY_TOUCH_PERSISTS:
			type = PhysXScene::ContactEventType::ContactStay;
			break;
		case PxPairFlag::eNOTIFY_TOUCH_LOST:
			type = PhysXScene::ContactEventType::ContactEnd;
			break;
		default:
			ignoreContact = true;
			break;
		}

		if(ignoreContact)
			continue;

		PhysXScene::ContactEvent event;
		event.Type = type;

		PxU32 contactCount = pair.contactCount;
		const PxU8* stream = pair.contactStream;
		PxU16 streamSize = pair.contactStreamSize;

		if(contactCount > 0 && streamSize > 0)
		{
			PxU32 contactIdx = 0;
			PxContactStreamIterator iter((PxU8*)stream, streamSize);

			stream += ((streamSize + 15) & ~15);

			const PxReal* impulses = reinterpret_cast<const PxReal*>(stream);
			PxU32 hasImpulses = (pair.flags & PxContactPairFlag::eINTERNAL_HAS_IMPULSES);

			while(iter.hasNextPatch())
			{
				iter.nextPatch();
				while(iter.hasNextContact())
				{
					iter.nextContact();

					ContactPoint point;
					point.Position = FromPxVector(iter.getContactPoint());
					point.Separation = iter.getSeparation();
					point.Normal = FromPxVector(iter.getContactNormal());

					if(hasImpulses)
						point.Impulse = impulses[contactIdx];
					else
						point.Impulse = 0.0f;

					event.Points.push_back(point);

					contactIdx++;
				}
			}
		}

		event.ColliderShapeA = (ColliderShape*)pair.shapes[0]->userData;
		event.ColliderShapeB = (ColliderShape*)pair.shapes[1]->userData;

		mScene.ReportContactEvent(event);
	}
}

void PhysXEventCallback::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)
{
	for(u32 i = 0; i < count; i++)
	{
		PxConstraintInfo& constraintInfo = constraints[i];

		if(constraintInfo.type != PxConstraintExtIDs::eJOINT)
			continue;

		PxJoint* pxJoint = (PxJoint*)constraintInfo.externalReference;

		PhysXScene::JointBreakEvent event;
		event.Joint = (Joint*)pxJoint->userData;

		if(event.Joint != nullptr)
			mScene.ReportJointBreakEvent(event);
	}
}

class PhysXCPUDispatcher : public PxCpuDispatcher
{
public:
	void submitTask(PxBaseTask& physxTask) override
	{
		auto runTask = [&physxTask]() { physxTask.run(); physxTask.release(); };
		GetApplication().GetTaskScheduler().Post(SchedulerTask(std::move(runTask), "PhysXWorker"));
	}

	PxU32 getWorkerCount() const override
	{
		return (PxU32)GetApplication().GetTaskScheduler().GetInformation().InternalWorkerThreadCount;
	}
};

class PhysXBroadPhaseCallback : public PxBroadPhaseCallback
{
	void onObjectOutOfBounds(PxShape& shape, PxActor& actor) override
	{
		ColliderShape* colliderShape = (ColliderShape*)shape.userData;
		if(colliderShape != nullptr)
			B3D_LOG(Warning, LogPhysics, "Physics object out of bounds. Consider increasing broadphase region!");
	}

	void onObjectOutOfBounds(PxAggregate& aggregate) override
	{ /* Do nothing */
	}
};
} // namespace b3d

using namespace b3d;

PxFilterFlags PhysXFilterShader(PxFilterObjectAttributes attr0, PxFilterData data0, PxFilterObjectAttributes attr1, PxFilterData data1, PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	PhysXObjectFilterFlags flags0 = PhysXObjectFilterFlags(data0.word2);
	PhysXObjectFilterFlags flags1 = PhysXObjectFilterFlags(data1.word2);

	if(flags0.IsSet(PhysXObjectFilterFlag::ReportAll) || flags1.IsSet(PhysXObjectFilterFlag::ReportAll))
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_LOST | PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eNOTIFY_CONTACT_POINTS;
	else if(flags0.IsSet(PhysXObjectFilterFlag::ReportBasic) || flags1.IsSet(PhysXObjectFilterFlag::ReportBasic))
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_LOST | PxPairFlag::eNOTIFY_CONTACT_POINTS;

	if(PxFilterObjectIsTrigger(attr0) || PxFilterObjectIsTrigger(attr1))
	{
		if(!pairFlags)
			return PxFilterFlag::eSUPPRESS; // Trigger with no notify flags

		pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;
		return PxFilterFlags();
	}

	u64 groupA = *(u64*)&data0.word0;
	u64 groupB = *(u64*)&data1.word0;

	bool canCollide = GetPhysics().IsCollisionEnabled(groupA, groupB);
	if(!canCollide)
		return PxFilterFlag::eSUPPRESS;

	if(flags0.IsSet(PhysXObjectFilterFlag::CCD) || flags1.IsSet(PhysXObjectFilterFlag::CCD))
		pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;

	pairFlags |= PxPairFlag::eSOLVE_CONTACT | PxPairFlag::eDETECT_DISCRETE_CONTACT;
	return PxFilterFlags();
}

void SetUnmappedTriangleIndex(const PxQueryHit& input, PhysicsQueryHit& output, PxShape* shapeHint = nullptr)
{
	// We can only assign a valid unmapped triangle index if the hit geometry is a triangle mesh
	// and it was created with the flags to store the remapping.
	// As a fallback, the raw face index is used.

	PxShape* shape = shapeHint ? shapeHint : input.shape;

	if(shape != nullptr && shape->getGeometryType() == PxGeometryType::eTRIANGLEMESH)
	{
		PxTriangleMeshGeometry triMeshGeometry;
		shape->getTriangleMeshGeometry(triMeshGeometry);

		if(triMeshGeometry.isValid() && triMeshGeometry.triangleMesh->getTrianglesRemap() != nullptr)
		{
			output.UnmappedTriangleIdx = triMeshGeometry.triangleMesh->getTrianglesRemap()[input.faceIndex];
			return;
		}
	}

	output.UnmappedTriangleIdx = input.faceIndex;
}

void ParseHit(const PxRaycastHit& input, PhysicsQueryHit& output, PxShape* shapeHint = nullptr)
{
	output.Point = FromPxVector(input.position);
	output.Normal = FromPxVector(input.normal);
	output.Distance = input.distance;
	output.TriangleIdx = input.faceIndex;
	SetUnmappedTriangleIndex(input, output, shapeHint);
	output.Uv = Vector2(input.u, input.v);

	ColliderShape* colliderShape = nullptr;
	if(input.shape)
		colliderShape = (ColliderShape*)input.shape->userData;

	if(colliderShape != nullptr)
	{
		Collider* const collider = colliderShape->GetParentCollider();
		if(collider != nullptr)
			output.Collider = B3DStaticGameObjectCast<Collider>(collider->GetHandle());

		output.ColliderShape = collider->GetShapes()[colliderShape->GetShapeIndexInParent()];
	}
}

void ParseHit(const PxSweepHit& input, PhysicsQueryHit& output, PxShape* shapeHint = nullptr)
{
	output.Point = FromPxVector(input.position);
	output.Normal = FromPxVector(input.normal);
	output.Uv = Vector2::kZero;
	output.Distance = input.distance;
	output.TriangleIdx = input.faceIndex;
	SetUnmappedTriangleIndex(input, output, shapeHint);

	ColliderShape* colliderShape = nullptr;
	if(input.shape)
		colliderShape = (ColliderShape*)input.shape->userData;

	if(colliderShape != nullptr)
	{
		Collider* const collider = colliderShape->GetParentCollider();
		if(collider != nullptr)
			output.Collider = B3DStaticGameObjectCast<Collider>(collider->GetHandle());

		output.ColliderShape = collider->GetShapes()[colliderShape->GetShapeIndexInParent()];
	}
}

static CollisionData ConvertCollisionData(CollisionDataRaw& data)
{
	CollisionData output;
	output.ContactPoints = std::move(data.ContactPoints);

	ColliderShape* const colliderShape0 = data.ColliderShapes[0];
	if(colliderShape0 != nullptr)
	{
		Collider* const collider = colliderShape0->GetParentCollider();
		output.Collider[0] = B3DStaticGameObjectCast<Collider>(collider->GetHandle());
		output.ColliderShapes[0] = collider->GetShapes()[colliderShape0->GetShapeIndexInParent()];
	}

	ColliderShape* const colliderShape1 = data.ColliderShapes[1];
	if(colliderShape1 != nullptr)
	{
		Collider* const collider = colliderShape1->GetParentCollider();
		output.Collider[1] = B3DStaticGameObjectCast<Collider>(collider->GetHandle());
		output.ColliderShapes[1] = collider->GetShapes()[colliderShape1->GetShapeIndexInParent()];
	}

	return output;
}

struct PhysXRaycastQueryCallback : PxRaycastCallback
{
	static const int kMaxHits = 32;
	PxRaycastHit Buffer[kMaxHits];

	Vector<PhysicsQueryHit> Data;

	PhysXRaycastQueryCallback()
		: PxRaycastCallback(Buffer, kMaxHits)
	{}

	PxAgain processTouches(const PxRaycastHit* buffer, PxU32 nbHits) override
	{
		for(PxU32 i = 0; i < nbHits; i++)
		{
			Data.push_back(PhysicsQueryHit());
			ParseHit(buffer[i], Data.back());
		}

		return true;
	}
};

struct PhysXSweepQueryCallback : PxSweepCallback
{
	static const int kMaxHits = 32;
	PxSweepHit Buffer[kMaxHits];

	Vector<PhysicsQueryHit> Data;

	PhysXSweepQueryCallback()
		: PxSweepCallback(Buffer, kMaxHits)
	{}

	PxAgain processTouches(const PxSweepHit* buffer, PxU32 nbHits) override
	{
		for(PxU32 i = 0; i < nbHits; i++)
		{
			Data.push_back(PhysicsQueryHit());
			ParseHit(buffer[i], Data.back());
		}

		return true;
	}
};

struct PhysXOverlapQueryCallback : PxOverlapCallback
{
	static const int kMaxHits = 32;
	PxOverlapHit Buffer[kMaxHits];

	Vector<ColliderShape*> Data;

	PhysXOverlapQueryCallback()
		: PxOverlapCallback(Buffer, kMaxHits)
	{}

	PxAgain processTouches(const PxOverlapHit* buffer, PxU32 nbHits) override
	{
		for(PxU32 i = 0; i < nbHits; i++)
			Data.push_back((ColliderShape*)buffer[i].shape->userData);

		return true;
	}
};

static PhysXAllocator gPhysXAllocator;
static PhysXErrorCallback gPhysXErrorHandler;
static PhysXCPUDispatcher gPhysXCPUDispatcher;
static PhysXBroadPhaseCallback gPhysXBroadphaseCallback;

PhysX::PhysX(const PhysicsCreateInformation& input)
	: Physics(input), mInitDesc(input)
{
	mScale.length = input.TypicalLength;
	mScale.speed = input.TypicalSpeed;

	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gPhysXAllocator, gPhysXErrorHandler);
	mPhysics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *mFoundation, mScale);

	PxRegisterArticulations(*mPhysics);

	if(input.InitCooking)
	{
		// Note: PhysX supports cooking for specific platforms to make the generated results better. Consider
		// allowing the meshes to be re-cooked when target platform is changed. Right now we just use the default value.

		PxCookingParams cookingParams(mScale);
		mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *mFoundation, cookingParams);
	}

	mDefaultMaterial = mPhysics->createMaterial(1.0f, 1.0f, 0.5f);
}

PhysX::~PhysX()
{
	B3D_ASSERT(mScenes.empty() && "All scenes must be freed before physics system shutdown");

	if(mCooking != nullptr)
		mCooking->release();

	mPhysics->release();
	mFoundation->release();
}

SPtr<PhysicsMaterial> PhysX::CreateMaterial(float staticFriction, float dynamicFriction, float restitution)
{
	return B3DMakeShared<PhysXMaterial>(mPhysics, staticFriction, dynamicFriction, restitution);
}

UPtr<IPhysicsMeshImplementation> PhysX::CreateMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
{
	return B3DMakeUnique<PhysXMesh>(meshData, type);
}

SPtr<PhysicsScene> PhysX::CreatePhysicsScene()
{
	SPtr<PhysXScene> scene = B3DMakeShared<PhysXScene>(mPhysics, mInitDesc, mScale);
	mScenes.push_back(scene.get());

	return scene;
}

SPtr<ColliderShape> PhysX::CreateColliderShape()
{
	return B3DMakeShared<PhysXColliderShape>();
}

UPtr<IColliderImplementation> PhysX::CreateColliderImplementation()
{
	return B3DMakeUnique<PhysXCollider>();
}

UPtr<IRigidbodyImplementation> PhysX::CreateRigidbodyImplementation(Rigidbody& owner)
{
	return B3DMakeUnique<PhysXRigidbody>(owner);
}

void PhysX::NotifySceneDestroyed(PhysXScene* scene)
{
	auto iterFind = std::find(mScenes.begin(), mScenes.end(), scene);
	B3D_ASSERT(iterFind != mScenes.end());

	mScenes.erase(iterFind);
}

bool PhysX::RayCast(const Vector3& origin, const Vector3& unitDirection, const ColliderShape& colliderShape, PhysicsQueryHit& hit, float maxDistance) const
{
	const PhysXColliderShape& physxColliderShape = static_cast<const PhysXColliderShape&>(colliderShape);
	PxShape* const pxShape = physxColliderShape.GetPxShape();

	const PxTransform transform = ToPxTransform(colliderShape.GetPosition(), colliderShape.GetRotation());

	PxRaycastHit hitInfo;
	PxU32 maxHits = 1;
	bool anyHit = false;
	PxHitFlags hitFlags = PxHitFlag::eDEFAULT | PxHitFlag::eUV;
	PxU32 hitCount = PxGeometryQuery::raycast(ToPxVector(origin), ToPxVector(unitDirection), pxShape->getGeometry().any(), transform, maxDistance, hitFlags, maxHits, &hitInfo, anyHit);

	if(hitCount > 0)
		ParseHit(hitInfo, hit, pxShape); // We have to provide a hint for the tested shape, as it is not contained in single-geometry raycast hit results

	return hitCount > 0;
}

bool PhysX::RayCast(const Vector3& origin, const Vector3& unitDirection, const Collider& collider, PhysicsQueryHit& hit, float maxDistance) const
{
	float nearestHitDistance = FLT_MIN;
	bool isAnythingHit = false;

	const Transform& transform = collider.SceneObject()->GetTransform();
	const PxTransform colliderTransform = ToPxTransform(transform.GetPosition(), transform.GetRotation());

	const TInlineArray<SPtr<ColliderShape>, 1>& shapes = collider.GetShapes();
	for(const auto& entry : shapes)
	{
		const PhysXColliderShape& physxColliderShape = static_cast<const PhysXColliderShape&>(*entry);
		PxShape* const pxShape = physxColliderShape.GetPxShape();

		const PxTransform shapeTransform = ToPxTransform(entry->GetPosition(), entry->GetRotation());
		const PxTransform combinedTransform = colliderTransform.transform(shapeTransform);

		PxRaycastHit hitInfo;
		PxU32 maxHits = 1;
		bool anyHit = false;
		PxHitFlags hitFlags = PxHitFlag::eDEFAULT | PxHitFlag::eUV;
		PxU32 hitCount = PxGeometryQuery::raycast(ToPxVector(origin), ToPxVector(unitDirection), pxShape->getGeometry().any(), combinedTransform, maxDistance, hitFlags, maxHits, &hitInfo, anyHit);

		if(hitCount > 0 && hitInfo.distance < nearestHitDistance)
		{
			ParseHit(hitInfo, hit, pxShape);

			nearestHitDistance = hitInfo.distance;
			isAnythingHit = true;
		}
	}

	return isAnythingHit;
}

PhysXScene::PhysXScene(PxPhysics* physics, const PhysicsCreateInformation& input, const physx::PxTolerancesScale& scale)
	: mPhysics(physics), mEventCallback(*this)
{
	PxSceneDesc sceneDesc(scale); // TODO - Test out various other parameters provided by scene desc
	sceneDesc.gravity = ToPxVector(input.Gravity);
	sceneDesc.cpuDispatcher = &gPhysXCPUDispatcher;
	sceneDesc.filterShader = PhysXFilterShader;
	sceneDesc.simulationEventCallback = &mEventCallback;
	sceneDesc.broadPhaseCallback = &gPhysXBroadphaseCallback;

	// Optionally: eENABLE_KINEMATIC_STATIC_PAIRS, eENABLE_KINEMATIC_PAIRS, eENABLE_PCM
	sceneDesc.flags = PxSceneFlag::eENABLE_ACTIVETRANSFORMS;

	if(input.Flags.IsSet(PhysicsFlag::CCD_Enable))
		sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;

	// Optionally: eMBP
	sceneDesc.broadPhaseType = PxBroadPhaseType::eSAP;

	mScene = physics->createScene(sceneDesc);

	// Character controller
	mCharManager = PxCreateControllerManager(*mScene);
}

PhysXScene::~PhysXScene()
{
	mCharManager->release();
	mScene->release();

	GetPhysX().NotifySceneDestroyed(this);
}

void PhysXScene::FixedUpdate(float step)
{
	static constexpr u32 kSize16K = 1 << 14;
	static constexpr u32 kScratchBufferSize = kSize16K * 64; // 1MB by default

	if(mPaused)
		return;

	mUpdateInProgress = true;

	// Note: Consider delaying fetchResults one frame. This could improve performance because Physics update would be
	//       able to run parallel to the simulation thread, but at a cost to input latency.
	B3DMarkAllocatorFrame();
	u8* scratchBuffer = B3DFrameAllocateAligned(kScratchBufferSize, 16);

	mScene->simulate(step, nullptr, scratchBuffer, kScratchBufferSize);

	u32 errorState;
	if(!mScene->fetchResults(true, &errorState))
		B3D_LOG(Warning, LogPhysics, "Physics simulation failed. Error code: {0}", errorState);

	B3DFrameFreeAligned(scratchBuffer);
	B3DClearAllocatorFrame();

	// Update rigidbodies with new transforms
	PxU32 activeTransformCount;
	const PxActiveTransform* activeTransforms = mScene->getActiveTransforms(activeTransformCount);

	for(PxU32 transformIndex = 0; transformIndex < activeTransformCount; transformIndex++)
	{
		Rigidbody* rigidbody = static_cast<Rigidbody*>(activeTransforms[transformIndex].userData);

		// Note: This should never happen, as actors gets their userData set to null when they're destroyed. However
		// in some cases PhysX seems to keep those actors alive for a frame or few, and reports their state here. Until
		// I find out why I need to perform this check.
		if(activeTransforms[transformIndex].actor->userData == nullptr)
			continue;

		const PxTransform& transform = activeTransforms[transformIndex].actor2World;

		// Note: Make this faster, avoid dereferencing Rigidbody and attempt to access pos/rot destination directly,
		//       use non-temporal writes
		const HSceneObject& sceneObject = rigidbody->SceneObject();
		sceneObject->SetWorldPosition(FromPxVector(transform.p));
		sceneObject->SetWorldRotation(FromPxQuaternion(transform.q));
	}

	// Note: Consider extrapolating for the remaining "simulationAmount" value
	mUpdateInProgress = false;

	TriggerEvents();
}

void PhysXScene::Update()
{
	// Note: Potentially interpolate (would mean a one frame delay needs to be introduced)
}

void PhysXScene::SetPaused(bool paused)
{
	mPaused = paused;
}

UPtr<IFixedJointImplementation> PhysXScene::CreateFixedJoint(Joint& owner, const FixedJointCreateInformation& createInformation)
{
	return B3DMakeUnique<PhysXFixedJoint>(mPhysics, owner, createInformation);
}

UPtr<IDistanceJointImplementation> PhysXScene::CreateDistanceJoint(Joint& owner, const DistanceJointCreateInformation& createInformation)
{
	return B3DMakeUnique<PhysXDistanceJoint>(mPhysics, owner, createInformation);
}

UPtr<IHingeJointImplementation> PhysXScene::CreateHingeJoint(Joint& owner, const HingeJointCreateInformation& createInformation)
{
	return B3DMakeUnique<PhysXHingeJoint>(mPhysics, owner, createInformation);
}

UPtr<ISphericalJointImplementation> PhysXScene::CreateSphericalJoint(Joint& owner, const SphericalJointCreateInformation& createInformation)
{
	return B3DMakeUnique<PhysXSphericalJoint>(mPhysics, owner, createInformation);
}

UPtr<ISliderJointImplementation> PhysXScene::CreateSliderJoint(Joint& owner, const SliderJointCreateInformation& createInformation)
{
	return B3DMakeUnique<PhysXSliderJoint>(mPhysics, owner, createInformation);
}

UPtr<ID6JointImplementation> PhysXScene::CreateD6Joint(Joint& owner, const D6JointCreateInformation& createInformation)
{
	return B3DMakeUnique<PhysXD6Joint>(mPhysics, owner, createInformation);
}

UPtr<ICharacterControllerImplementation> PhysXScene::CreateCharacterController(CharacterController& owner, const CharacterControllerCreateInformation& createInformation)
{
	return B3DMakeUnique<PhysXCharacterController>(mCharManager, owner, createInformation);
}

Vector<PhysicsQueryHit> PhysXScene::SweepAll(const PxGeometry& geometry, const PxTransform& tfrm, const Vector3& unitDir, u64 layer, float maxDist) const
{
	PhysXSweepQueryCallback output;

	PxQueryFilterData filterData;
	memcpy(&filterData.data.word0, &layer, sizeof(layer));

	mScene->sweep(geometry, tfrm, ToPxVector(unitDir), maxDist, output, PxHitFlag::eDEFAULT | PxHitFlag::eUV, filterData);

	return output.Data;
}

bool PhysXScene::SweepAny(const PxGeometry& geometry, const PxTransform& tfrm, const Vector3& unitDir, u64 layer, float maxDist) const
{
	PxSweepBuffer output;

	PxQueryFilterData filterData;
	filterData.flags |= PxQueryFlag::eANY_HIT;
	memcpy(&filterData.data.word0, &layer, sizeof(layer));

	return mScene->sweep(geometry, tfrm, ToPxVector(unitDir), maxDist, output, PxHitFlag::eDEFAULT | PxHitFlag::eUV | PxHitFlag::eMESH_ANY, filterData);
}

bool PhysXScene::RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer, float max) const
{
	PxRaycastBuffer output;

	PxQueryFilterData filterData;
	memcpy(&filterData.data.word0, &layer, sizeof(layer));

	bool wasHit = mScene->raycast(ToPxVector(origin), ToPxVector(unitDir), max, output, PxHitFlag::eDEFAULT | PxHitFlag::eUV, filterData);

	if(wasHit)
		ParseHit(output.block, hit);

	return wasHit;
}

bool PhysXScene::BoxCast(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer, float max) const
{
	PxBoxGeometry geometry(ToPxVector(box.GetExtents()));
	PxTransform transform = ToPxTransform(box.GetCenter(), rotation);

	return Sweep(geometry, transform, unitDir, hit, layer, max);
}

bool PhysXScene::SphereCast(const Sphere& sphere, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer, float max) const
{
	PxSphereGeometry geometry(sphere.Radius);
	PxTransform transform = ToPxTransform(sphere.Center, Quaternion::kIdentity);

	return Sweep(geometry, transform, unitDir, hit, layer, max);
}

bool PhysXScene::CapsuleCast(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer, float max) const
{
	PxCapsuleGeometry geometry(capsule.GetRadius(), capsule.GetHeight() * 0.5f);
	PxTransform transform = ToPxTransform(capsule.GetCenter(), Quaternion::kIdentity);

	return Sweep(geometry, transform, unitDir, hit, layer, max);
}

bool PhysXScene::ConvexCast(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer, float max) const
{
	if(mesh == nullptr)
		return false;

	if(mesh->GetType() != PhysicsMeshType::Convex)
		return false;

	PhysXMesh* physxMesh = static_cast<PhysXMesh*>(mesh->GetImplementation());
	PxConvexMeshGeometry geometry(physxMesh->GetPxConvexMesh());
	PxTransform transform = ToPxTransform(position, rotation);

	return Sweep(geometry, transform, unitDir, hit, layer, max);
}

Vector<PhysicsQueryHit> PhysXScene::RayCastAll(const Vector3& origin, const Vector3& unitDir, u64 layer, float max) const
{
	PhysXRaycastQueryCallback output;

	PxQueryFilterData filterData;
	memcpy(&filterData.data.word0, &layer, sizeof(layer));

	mScene->raycast(ToPxVector(origin), ToPxVector(unitDir), max, output, PxHitFlag::eDEFAULT | PxHitFlag::eUV | PxHitFlag::eMESH_MULTIPLE, filterData);

	return output.Data;
}

Vector<PhysicsQueryHit> PhysXScene::BoxCastAll(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer, float max) const
{
	PxBoxGeometry geometry(ToPxVector(box.GetExtents()));
	PxTransform transform = ToPxTransform(box.GetCenter(), rotation);

	return SweepAll(geometry, transform, unitDir, layer, max);
}

Vector<PhysicsQueryHit> PhysXScene::SphereCastAll(const Sphere& sphere, const Vector3& unitDir, u64 layer, float max) const
{
	PxSphereGeometry geometry(sphere.Radius);
	PxTransform transform = ToPxTransform(sphere.Center, Quaternion::kIdentity);

	return SweepAll(geometry, transform, unitDir, layer, max);
}

Vector<PhysicsQueryHit> PhysXScene::CapsuleCastAll(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer, float max) const
{
	PxCapsuleGeometry geometry(capsule.GetRadius(), capsule.GetHeight() * 0.5f);
	PxTransform transform = ToPxTransform(capsule.GetCenter(), Quaternion::kIdentity);

	return SweepAll(geometry, transform, unitDir, layer, max);
}

Vector<PhysicsQueryHit> PhysXScene::ConvexCastAll(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer, float max) const
{
	if(mesh == nullptr)
		return Vector<PhysicsQueryHit>(0);

	if(mesh->GetType() != PhysicsMeshType::Convex)
		return Vector<PhysicsQueryHit>(0);

	PhysXMesh* physxMesh = static_cast<PhysXMesh*>(mesh->GetImplementation());
	PxConvexMeshGeometry geometry(physxMesh->GetPxConvexMesh());
	PxTransform transform = ToPxTransform(position, rotation);

	return SweepAll(geometry, transform, unitDir, layer, max);
}

bool PhysXScene::RayCastAny(const Vector3& origin, const Vector3& unitDir, u64 layer, float max) const
{
	PxRaycastBuffer output;

	PxQueryFilterData filterData;
	filterData.flags |= PxQueryFlag::eANY_HIT;
	memcpy(&filterData.data.word0, &layer, sizeof(layer));

	return mScene->raycast(ToPxVector(origin), ToPxVector(unitDir), max, output, PxHitFlag::eDEFAULT | PxHitFlag::eUV | PxHitFlag::eMESH_ANY, filterData);
}

bool PhysXScene::BoxCastAny(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer, float max) const
{
	PxBoxGeometry geometry(ToPxVector(box.GetExtents()));
	PxTransform transform = ToPxTransform(box.GetCenter(), rotation);

	return SweepAny(geometry, transform, unitDir, layer, max);
}

bool PhysXScene::SphereCastAny(const Sphere& sphere, const Vector3& unitDir, u64 layer, float max) const
{
	PxSphereGeometry geometry(sphere.Radius);
	PxTransform transform = ToPxTransform(sphere.Center, Quaternion::kIdentity);

	return SweepAny(geometry, transform, unitDir, layer, max);
}

bool PhysXScene::CapsuleCastAny(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer, float max) const
{
	PxCapsuleGeometry geometry(capsule.GetRadius(), capsule.GetHeight() * 0.5f);
	PxTransform transform = ToPxTransform(capsule.GetCenter(), Quaternion::kIdentity);

	return SweepAny(geometry, transform, unitDir, layer, max);
}

bool PhysXScene::ConvexCastAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer, float max) const
{
	if(mesh == nullptr)
		return false;

	if(mesh->GetType() != PhysicsMeshType::Convex)
		return false;

	PhysXMesh* physxMesh = static_cast<PhysXMesh*>(mesh->GetImplementation());
	PxConvexMeshGeometry geometry(physxMesh->GetPxConvexMesh());
	PxTransform transform = ToPxTransform(position, rotation);

	return SweepAny(geometry, transform, unitDir, layer, max);
}

Vector<ColliderShape*> PhysXScene::BoxOverlapInternal(const AABox& box, const Quaternion& rotation, u64 layer) const
{
	PxBoxGeometry geometry(ToPxVector(box.GetExtents()));
	PxTransform transform = ToPxTransform(box.GetCenter(), rotation);

	return Overlap(geometry, transform, layer);
}

Vector<ColliderShape*> PhysXScene::SphereOverlapInternal(const Sphere& sphere, u64 layer) const
{
	PxSphereGeometry geometry(sphere.Radius);
	PxTransform transform = ToPxTransform(sphere.Center, Quaternion::kIdentity);

	return Overlap(geometry, transform, layer);
}

Vector<ColliderShape*> PhysXScene::CapsuleOverlapInternal(const Capsule& capsule, const Quaternion& rotation, u64 layer) const
{
	PxCapsuleGeometry geometry(capsule.GetRadius(), capsule.GetHeight() * 0.5f);
	PxTransform transform = ToPxTransform(capsule.GetCenter(), Quaternion::kIdentity);

	return Overlap(geometry, transform, layer);
}

Vector<ColliderShape*> PhysXScene::ConvexOverlapInternal(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer) const
{
	if(mesh == nullptr)
		return {};

	if(mesh->GetType() != PhysicsMeshType::Convex)
		return {};

	PhysXMesh* physxMesh = static_cast<PhysXMesh*>(mesh->GetImplementation());
	PxConvexMeshGeometry geometry(physxMesh->GetPxConvexMesh());
	PxTransform transform = ToPxTransform(position, rotation);

	return Overlap(geometry, transform, layer);
}

bool PhysXScene::BoxOverlapAny(const AABox& box, const Quaternion& rotation, u64 layer) const
{
	PxBoxGeometry geometry(ToPxVector(box.GetExtents()));
	PxTransform transform = ToPxTransform(box.GetCenter(), rotation);

	return OverlapAny(geometry, transform, layer);
}

bool PhysXScene::SphereOverlapAny(const Sphere& sphere, u64 layer) const
{
	PxSphereGeometry geometry(sphere.Radius);
	PxTransform transform = ToPxTransform(sphere.Center, Quaternion::kIdentity);

	return OverlapAny(geometry, transform, layer);
}

bool PhysXScene::CapsuleOverlapAny(const Capsule& capsule, const Quaternion& rotation, u64 layer) const
{
	PxCapsuleGeometry geometry(capsule.GetRadius(), capsule.GetHeight() * 0.5f);
	PxTransform transform = ToPxTransform(capsule.GetCenter(), Quaternion::kIdentity);

	return OverlapAny(geometry, transform, layer);
}

bool PhysXScene::ConvexOverlapAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer) const
{
	if(mesh == nullptr)
		return false;

	if(mesh->GetType() != PhysicsMeshType::Convex)
		return false;

	PhysXMesh* physxMesh = static_cast<PhysXMesh*>(mesh->GetImplementation());
	PxConvexMeshGeometry geometry(physxMesh->GetPxConvexMesh());
	PxTransform transform = ToPxTransform(position, rotation);

	return OverlapAny(geometry, transform, layer);
}

bool PhysXScene::Sweep(const PxGeometry& geometry, const PxTransform& tfrm, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer, float maxDist) const
{
	PxSweepBuffer output;

	PxQueryFilterData filterData;
	memcpy(&filterData.data.word0, &layer, sizeof(layer));

	bool wasHit = mScene->sweep(geometry, tfrm, ToPxVector(unitDir), maxDist, output, PxHitFlag::eDEFAULT | PxHitFlag::eUV, filterData);

	if(wasHit)
		ParseHit(output.block, hit);

	return wasHit;
}

bool PhysXScene::OverlapAny(const PxGeometry& geometry, const PxTransform& tfrm, u64 layer) const
{
	PxOverlapBuffer output;

	PxQueryFilterData filterData;
	filterData.flags |= PxQueryFlag::eANY_HIT;
	memcpy(&filterData.data.word0, &layer, sizeof(layer));

	return mScene->overlap(geometry, tfrm, output, filterData);
}

Vector<ColliderShape*> PhysXScene::Overlap(const PxGeometry& geometry, const PxTransform& tfrm, u64 layer) const
{
	PhysXOverlapQueryCallback output;

	PxQueryFilterData filterData;
	memcpy(&filterData.data.word0, &layer, sizeof(layer));

	mScene->overlap(geometry, tfrm, output, filterData);
	return output.Data;
}

void PhysXScene::SetFlag(PhysicsFlags flag, bool enabled)
{
	PhysicsScene::SetFlag(flag, enabled);

	mCharManager->setOverlapRecoveryModule(mFlags.IsSet(PhysicsFlag::CCT_OverlapRecovery));
	mCharManager->setPreciseSweeps(mFlags.IsSet(PhysicsFlag::CCT_PreciseSweeps));
	mCharManager->setTessellation(mFlags.IsSet(PhysicsFlag::CCT_Tesselation), mTesselationLength);
}

Vector3 PhysXScene::GetGravity() const
{
	return FromPxVector(mScene->getGravity());
}

void PhysXScene::SetGravity(const Vector3& gravity)
{
	mScene->setGravity(ToPxVector(gravity));
}

void PhysXScene::SetMaxTesselationEdgeLength(float length)
{
	mTesselationLength = length;

	mCharManager->setTessellation(mFlags.IsSet(PhysicsFlag::CCT_Tesselation), mTesselationLength);
}

u32 PhysXScene::AddBroadPhaseRegion(const AABox& region)
{
	u32 id = mNextRegionIdx++;

	PxBroadPhaseRegion pxRegion;
	pxRegion.bounds = PxBounds3(ToPxVector(region.Minimum), ToPxVector(region.Maximum));
	pxRegion.userData = (void*)(u64)id;

	u32 handle = mScene->addBroadPhaseRegion(pxRegion, true);
	mBroadPhaseRegionHandles[id] = handle;

	return handle;
}

void PhysXScene::RemoveBroadPhaseRegion(u32 regionId)
{
	auto iterFind = mBroadPhaseRegionHandles.find(regionId);
	if(iterFind == mBroadPhaseRegionHandles.end())
		return;

	mScene->removeBroadPhaseRegion(iterFind->second);
	mBroadPhaseRegionHandles.erase(iterFind);
}

void PhysXScene::ClearBroadPhaseRegions()
{
	for(auto& entry : mBroadPhaseRegionHandles)
		mScene->removeBroadPhaseRegion(entry.second);

	mBroadPhaseRegionHandles.clear();
}

void PhysXScene::ReportContactEvent(const ContactEvent& event)
{
	mContactEvents.push_back(event);
}

void PhysXScene::ReportTriggerEvent(const TriggerEvent& event)
{
	mTriggerEvents.push_back(event);
}

void PhysXScene::ReportJointBreakEvent(const JointBreakEvent& event)
{
	mJointBreakEvents.push_back(event);
}

void PhysXScene::TriggerEvents()
{
	CollisionDataRaw data;

	for(auto& entry : mTriggerEvents)
	{
		data.ColliderShapes[0] = entry.Trigger;
		data.ColliderShapes[1] = entry.Other;

		Collider* const triggerCollider = entry.Trigger->GetParentCollider();

		switch(entry.Type)
		{
		case ContactEventType::ContactBegin:
			triggerCollider->OnCollisionBegin(ConvertCollisionData(data));
			break;
		case ContactEventType::ContactStay:
			triggerCollider->OnCollisionStay(ConvertCollisionData(data));
			break;
		case ContactEventType::ContactEnd:
			triggerCollider->OnCollisionEnd(ConvertCollisionData(data));
			break;
		}
	}

	auto notifyContact = [&](ColliderShape* colliderShapeA, ColliderShape* colliderShapeB, ContactEventType type, const Vector<ContactPoint>& points, bool flipNormals = false)
	{
		data.ColliderShapes[0] = colliderShapeA;
		data.ColliderShapes[1] = colliderShapeB;
		data.ContactPoints = points;

		if(flipNormals)
		{
			for(auto& point : data.ContactPoints)
				point.Normal = -point.Normal;
		}

		Collider* const colliderA = colliderShapeA->GetParentCollider();
		const HRigidbody& rigidbody = colliderA->GetRigidbody();
		if(rigidbody.IsValid())
		{
			switch(type)
			{
			case ContactEventType::ContactBegin:
				rigidbody->OnCollisionBegin(ConvertCollisionData(data));
				break;
			case ContactEventType::ContactStay:
				rigidbody->OnCollisionStay(ConvertCollisionData(data));
				break;
			case ContactEventType::ContactEnd:
				rigidbody->OnCollisionEnd(ConvertCollisionData(data));
				break;
			}
		}
		else
		{
			switch(type)
			{
			case ContactEventType::ContactBegin:
				colliderA->OnCollisionBegin(ConvertCollisionData(data));
				break;
			case ContactEventType::ContactStay:
				colliderA->OnCollisionStay(ConvertCollisionData(data));
				break;
			case ContactEventType::ContactEnd:
				colliderA->OnCollisionEnd(ConvertCollisionData(data));
				break;
			}
		}
	};

	for(auto& entry : mContactEvents)
	{
		if(entry.ColliderShapeA != nullptr)
		{
			CollisionReportMode reportModeA = entry.ColliderShapeA->GetCollisionReportMode();

			if(reportModeA == CollisionReportMode::ReportPersistent)
				notifyContact(entry.ColliderShapeA, entry.ColliderShapeB, entry.Type, entry.Points, true);
			else if(reportModeA == CollisionReportMode::Report && entry.Type != ContactEventType::ContactStay)
				notifyContact(entry.ColliderShapeA, entry.ColliderShapeB, entry.Type, entry.Points, true);
		}

		if(entry.ColliderShapeB != nullptr)
		{
			CollisionReportMode reportModeB = entry.ColliderShapeB->GetCollisionReportMode();

			if(reportModeB == CollisionReportMode::ReportPersistent)
				notifyContact(entry.ColliderShapeB, entry.ColliderShapeA, entry.Type, entry.Points, false);
			else if(reportModeB == CollisionReportMode::Report && entry.Type != ContactEventType::ContactStay)
				notifyContact(entry.ColliderShapeB, entry.ColliderShapeA, entry.Type, entry.Points, false);
		}
	}

	for(auto& entry : mJointBreakEvents)
	{
		entry.Joint->OnJointBreak();
	}

	mTriggerEvents.clear();
	mContactEvents.clear();
	mJointBreakEvents.clear();
}

namespace b3d {
PhysX& GetPhysX()
{
	return static_cast<PhysX&>(PhysX::Instance());
}
} // namespace b3d
