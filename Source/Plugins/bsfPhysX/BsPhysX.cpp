//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysX.h"
#include "PxPhysicsAPI.h"
#include "BsPhysXMaterial.h"
#include "BsPhysXMesh.h"
#include "BsPhysXRigidbody.h"
#include "BsPhysXBoxCollider.h"
#include "BsPhysXSphereCollider.h"
#include "BsPhysXPlaneCollider.h"
#include "BsPhysXCapsuleCollider.h"
#include "BsPhysXMeshCollider.h"
#include "BsPhysXFixedJoint.h"
#include "BsPhysXDistanceJoint.h"
#include "BsPhysXHingeJoint.h"
#include "BsPhysXSphericalJoint.h"
#include "BsPhysXSliderJoint.h"
#include "BsPhysXD6Joint.h"
#include "BsPhysXCharacterController.h"
#include "Threading/BsTaskScheduler.h"
#include "Components/BsCCollider.h"
#include "BsFPhysXCollider.h"
#include "Utility/BsTime.h"
#include "Math/BsVector3.h"
#include "Math/BsAABox.h"
#include "Math/BsCapsule.h"
#include "foundation/PxTransform.h"

using namespace physx;

namespace bs {
class PhysXAllocator : public PxAllocatorCallback
{
public:
	void* allocate(size_t size, const char*, const char*, int) override
	{
		void* ptr = bs_alloc_aligned16((u32)size);
		PX_ASSERT((reinterpret_cast<size_t>(ptr) & 15) == 0);
		return ptr;
	}

	void deallocate(void* ptr) override
	{
		bs_free_aligned16(ptr);
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
			BS_LOG(Info, Physics, ss.str());
			break;
		case 1:
			ss << "PhysX warning (" << errorCode << "): " << message << " at " << file << ":" << line;
			BS_LOG(Warning, Physics, ss.str());
			break;
		case 2:
			ss << "PhysX error (" << errorCode << "): " << message << " at " << file << ":" << line;
			BS_LOG(Error, Physics, ss.str());
			BS_ASSERT(false); // Halt execution on debug builds when error occurs
			break;
		}
	}
};

class PhysXEventCallback : public PxSimulationEventCallback
{
	void onWake(PxActor** actors, PxU32 count) override
	{ /* Do nothing */
	}

	void onSleep(PxActor** actors, PxU32 count) override
	{ /* Do nothing */
	}

	void onTrigger(PxTriggerPair* pairs, PxU32 count) override
	{
		for(PxU32 i = 0; i < count; i++)
		{
			const PxTriggerPair& pair = pairs[i];
			if(pair.triggerShape->userData == nullptr)
				continue;

			PhysX::ContactEventType type;
			bool ignoreContact = false;
			PhysXObjectFilterFlags flags = PhysXObjectFilterFlags(pair.triggerShape->getSimulationFilterData().word2);

			if(flags.IsSet(PhysXObjectFilterFlag::ReportAll))
			{
				switch((u32)pair.status)
				{
				case PxPairFlag::eNOTIFY_TOUCH_FOUND:
					type = PhysX::ContactEventType::ContactBegin;
					break;
				case PxPairFlag::eNOTIFY_TOUCH_PERSISTS:
					type = PhysX::ContactEventType::ContactStay;
					break;
				case PxPairFlag::eNOTIFY_TOUCH_LOST:
					type = PhysX::ContactEventType::ContactEnd;
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
					type = PhysX::ContactEventType::ContactBegin;
					break;
				case PxPairFlag::eNOTIFY_TOUCH_LOST:
					type = PhysX::ContactEventType::ContactEnd;
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

			PhysX::TriggerEvent event;
			event.Trigger = (Collider*)pair.triggerShape->userData;
			event.Other = (Collider*)pair.otherShape->userData;
			event.Type = type;

			gPhysX().ReportTriggerEventInternal(event);
		}
	}

	void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 count) override
	{
		for(PxU32 i = 0; i < count; i++)
		{
			const PxContactPair& pair = pairs[i];

			PhysX::ContactEventType type;
			bool ignoreContact = false;
			switch((u32)pair.events)
			{
			case PxPairFlag::eNOTIFY_TOUCH_FOUND:
				type = PhysX::ContactEventType::ContactBegin;
				break;
			case PxPairFlag::eNOTIFY_TOUCH_PERSISTS:
				type = PhysX::ContactEventType::ContactStay;
				break;
			case PxPairFlag::eNOTIFY_TOUCH_LOST:
				type = PhysX::ContactEventType::ContactEnd;
				break;
			default:
				ignoreContact = true;
				break;
			}

			if(ignoreContact)
				continue;

			PhysX::ContactEvent event;
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
						point.Position = fromPxVector(iter.getContactPoint());
						point.Separation = iter.getSeparation();
						point.Normal = fromPxVector(iter.getContactNormal());

						if(hasImpulses)
							point.Impulse = impulses[contactIdx];
						else
							point.Impulse = 0.0f;

						event.Points.push_back(point);

						contactIdx++;
					}
				}
			}

			event.ColliderA = (Collider*)pair.shapes[0]->userData;
			event.ColliderB = (Collider*)pair.shapes[1]->userData;

			gPhysX().ReportContactEventInternal(event);
		}
	}

	void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override
	{
		for(u32 i = 0; i < count; i++)
		{
			PxConstraintInfo& constraintInfo = constraints[i];

			if(constraintInfo.type != PxConstraintExtIDs::eJOINT)
				continue;

			PxJoint* pxJoint = (PxJoint*)constraintInfo.externalReference;

			PhysX::JointBreakEvent event;
			event.Joint = (Joint*)pxJoint->userData;

			if(event.Joint != nullptr)
				gPhysX().ReportJointBreakEventInternal(event);
		}
	}
};

class PhysXCPUDispatcher : public PxCpuDispatcher
{
public:
	void submitTask(PxBaseTask& physxTask) override
	{
		// Note: Framework's task scheduler is pretty low granularity. Consider a better task manager in case PhysX ends
		// up submitting many tasks.
		// - PhysX's task manager doesn't seem much lighter either. But perhaps I can at least create a task pool to
		//   avoid allocating them constantly.

		auto runTask = [&]()
		{ physxTask.run(); physxTask.release(); };
		SPtr<Task> task = Task::Create("PhysX", runTask);

		TaskScheduler::Instance().AddTask(task);
	}

	PxU32 getWorkerCount() const override
	{
		return (PxU32)TaskScheduler::Instance().GetNumWorkers();
	}
};

class PhysXBroadPhaseCallback : public PxBroadPhaseCallback
{
	void onObjectOutOfBounds(PxShape& shape, PxActor& actor) override
	{
		Collider* collider = (Collider*)shape.userData;
		if(collider != nullptr)
			BS_LOG(Warning, Physics, "Physics object out of bounds. Consider increasing broadphase region!");
	}

	void onObjectOutOfBounds(PxAggregate& aggregate) override
	{ /* Do nothing */
	}
};
} // namespace bs

using namespace bs;

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

	bool canCollide = gPhysics().IsCollisionEnabled(groupA, groupB);
	if(!canCollide)
		return PxFilterFlag::eSUPPRESS;

	if(flags0.IsSet(PhysXObjectFilterFlag::CCD) || flags1.IsSet(PhysXObjectFilterFlag::CCD))
		pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;

	pairFlags |= PxPairFlag::eSOLVE_CONTACT | PxPairFlag::eDETECT_DISCRETE_CONTACT;
	return PxFilterFlags();
}

void setUnmappedTriangleIndex(const PxQueryHit& input, PhysicsQueryHit& output, PxShape* shapeHint = nullptr)
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

void parseHit(const PxRaycastHit& input, PhysicsQueryHit& output, PxShape* shapeHint = nullptr)
{
	output.Point = fromPxVector(input.position);
	output.Normal = fromPxVector(input.normal);
	output.Distance = input.distance;
	output.TriangleIdx = input.faceIndex;
	setUnmappedTriangleIndex(input, output, shapeHint);
	output.Uv = Vector2(input.u, input.v);

	if(input.shape)
		output.ColliderRaw = (Collider*)input.shape->userData;

	if(output.ColliderRaw != nullptr)
	{
		CCollider* component = (CCollider*)output.ColliderRaw->GetOwnerInternal(PhysicsOwnerType::Component);
		if(component != nullptr)
			output.Collider = static_object_cast<CCollider>(component->GetHandle());
	}
}

void parseHit(const PxSweepHit& input, PhysicsQueryHit& output, PxShape* shapeHint = nullptr)
{
	output.Point = fromPxVector(input.position);
	output.Normal = fromPxVector(input.normal);
	output.Uv = Vector2::ZERO;
	output.Distance = input.distance;
	output.TriangleIdx = input.faceIndex;
	setUnmappedTriangleIndex(input, output, shapeHint);
	output.ColliderRaw = (Collider*)input.shape->userData;

	if(output.ColliderRaw != nullptr)
	{
		CCollider* component = (CCollider*)output.ColliderRaw->GetOwnerInternal(PhysicsOwnerType::Component);
		if(component != nullptr)
			output.Collider = static_object_cast<CCollider>(component->GetHandle());
	}
}

struct PhysXRaycastQueryCallback : PxRaycastCallback
{
	static const int MAX_HITS = 32;
	PxRaycastHit Buffer[MAX_HITS];

	Vector<PhysicsQueryHit> Data;

	PhysXRaycastQueryCallback()
		: PxRaycastCallback(Buffer, MAX_HITS)
	{}

	PxAgain processTouches(const PxRaycastHit* buffer, PxU32 nbHits) override
	{
		for(PxU32 i = 0; i < nbHits; i++)
		{
			Data.push_back(PhysicsQueryHit());
			parseHit(buffer[i], Data.back());
		}

		return true;
	}
};

struct PhysXSweepQueryCallback : PxSweepCallback
{
	static const int MAX_HITS = 32;
	PxSweepHit Buffer[MAX_HITS];

	Vector<PhysicsQueryHit> Data;

	PhysXSweepQueryCallback()
		: PxSweepCallback(Buffer, MAX_HITS)
	{}

	PxAgain processTouches(const PxSweepHit* buffer, PxU32 nbHits) override
	{
		for(PxU32 i = 0; i < nbHits; i++)
		{
			Data.push_back(PhysicsQueryHit());
			parseHit(buffer[i], Data.back());
		}

		return true;
	}
};

struct PhysXOverlapQueryCallback : PxOverlapCallback
{
	static const int MAX_HITS = 32;
	PxOverlapHit Buffer[MAX_HITS];

	Vector<Collider*> Data;

	PhysXOverlapQueryCallback()
		: PxOverlapCallback(Buffer, MAX_HITS)
	{}

	PxAgain processTouches(const PxOverlapHit* buffer, PxU32 nbHits) override
	{
		for(PxU32 i = 0; i < nbHits; i++)
			Data.push_back((Collider*)buffer[i].shape->userData);

		return true;
	}
};

static PhysXAllocator gPhysXAllocator;
static PhysXErrorCallback gPhysXErrorHandler;
static PhysXCPUDispatcher gPhysXCPUDispatcher;
static PhysXEventCallback gPhysXEventCallback;
static PhysXBroadPhaseCallback gPhysXBroadphaseCallback;

static const u32 SIZE_16K = 1 << 14;
const u32 PhysX::SCRATCH_BUFFER_SIZE = SIZE_16K * 64; // 1MB by default

PhysX::PhysX(const PHYSICS_INIT_DESC& input)
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
	assert(mScenes.empty() && "All scenes must be freed before physics system shutdown");

	if(mCooking != nullptr)
		mCooking->release();

	mPhysics->release();
	mFoundation->release();
}

void PhysX::FixedUpdate(float step)
{
	if(mPaused)
		return;

	mUpdateInProgress = true;

	// Note: Consider delaying fetchResults one frame. This could improve performance because Physics update would be
	//       able to run parallel to the simulation thread, but at a cost to input latency.
	bs_frame_mark();
	u8* scratchBuffer = bs_frame_alloc_aligned(SCRATCH_BUFFER_SIZE, 16);

	for(auto& scene : mScenes)
	{
		scene->mScene->simulate(step, nullptr, scratchBuffer, SCRATCH_BUFFER_SIZE);

		u32 errorState;
		if(!scene->mScene->fetchResults(true, &errorState))
			BS_LOG(Warning, Physics, "Physics simulation failed. Error code: {0}", errorState);
	}

	bs_frame_free_aligned(scratchBuffer);
	bs_frame_clear();

	// Update rigidbodies with new transforms
	for(auto& scene : mScenes)
	{
		PxU32 numActiveTransforms;
		const PxActiveTransform* activeTransforms = scene->mScene->getActiveTransforms(numActiveTransforms);

		for(PxU32 i = 0; i < numActiveTransforms; i++)
		{
			Rigidbody* rigidbody = static_cast<Rigidbody*>(activeTransforms[i].userData);

			// Note: This should never happen, as actors gets their userData set to null when they're destroyed. However
			// in some cases PhysX seems to keep those actors alive for a frame or few, and reports their state here. Until
			// I find out why I need to perform this check.
			if(activeTransforms[i].actor->userData == nullptr)
				continue;

			const PxTransform& transform = activeTransforms[i].actor2World;

			// Note: Make this faster, avoid dereferencing Rigidbody and attempt to access pos/rot destination directly,
			//       use non-temporal writes
			rigidbody->SetTransformInternal(fromPxVector(transform.p), fromPxQuaternion(transform.q));
		}
	}

	// Note: Consider extrapolating for the remaining "simulationAmount" value
	mUpdateInProgress = false;

	TriggerEvents();
}

void PhysX::Update()
{
	// Note: Potentially interpolate (would mean a one frame delay needs to be introduced)
}

void PhysX::ReportContactEventInternal(const ContactEvent& event)
{
	mContactEvents.push_back(event);
}

void PhysX::ReportTriggerEventInternal(const TriggerEvent& event)
{
	mTriggerEvents.push_back(event);
}

void PhysX::ReportJointBreakEventInternal(const JointBreakEvent& event)
{
	mJointBreakEvents.push_back(event);
}

void PhysX::TriggerEvents()
{
	CollisionDataRaw data;

	for(auto& entry : mTriggerEvents)
	{
		data.Colliders[0] = entry.Trigger;
		data.Colliders[1] = entry.Other;

		switch(entry.Type)
		{
		case ContactEventType::ContactBegin:
			entry.Trigger->OnCollisionBegin(data);
			break;
		case ContactEventType::ContactStay:
			entry.Trigger->OnCollisionStay(data);
			break;
		case ContactEventType::ContactEnd:
			entry.Trigger->OnCollisionEnd(data);
			break;
		}
	}

	auto notifyContact = [&](Collider* obj, Collider* other, ContactEventType type,
							 const Vector<ContactPoint>& points, bool flipNormals = false)
	{
		data.Colliders[0] = obj;
		data.Colliders[1] = other;
		data.ContactPoints = points;

		if(flipNormals)
		{
			for(auto& point : data.ContactPoints)
				point.Normal = -point.Normal;
		}

		Rigidbody* rigidbody = obj->GetRigidbody();
		if(rigidbody != nullptr)
		{
			switch(type)
			{
			case ContactEventType::ContactBegin:
				rigidbody->OnCollisionBegin(data);
				break;
			case ContactEventType::ContactStay:
				rigidbody->OnCollisionStay(data);
				break;
			case ContactEventType::ContactEnd:
				rigidbody->OnCollisionEnd(data);
				break;
			}
		}
		else
		{
			switch(type)
			{
			case ContactEventType::ContactBegin:
				obj->OnCollisionBegin(data);
				break;
			case ContactEventType::ContactStay:
				obj->OnCollisionStay(data);
				break;
			case ContactEventType::ContactEnd:
				obj->OnCollisionEnd(data);
				break;
			}
		}
	};

	for(auto& entry : mContactEvents)
	{
		if(entry.ColliderA != nullptr)
		{
			CollisionReportMode reportModeA = entry.ColliderA->GetCollisionReportMode();

			if(reportModeA == CollisionReportMode::ReportPersistent)
				notifyContact(entry.ColliderA, entry.ColliderB, entry.Type, entry.Points, true);
			else if(reportModeA == CollisionReportMode::Report && entry.Type != ContactEventType::ContactStay)
				notifyContact(entry.ColliderA, entry.ColliderB, entry.Type, entry.Points, true);
		}

		if(entry.ColliderB != nullptr)
		{
			CollisionReportMode reportModeB = entry.ColliderB->GetCollisionReportMode();

			if(reportModeB == CollisionReportMode::ReportPersistent)
				notifyContact(entry.ColliderB, entry.ColliderA, entry.Type, entry.Points, false);
			else if(reportModeB == CollisionReportMode::Report && entry.Type != ContactEventType::ContactStay)
				notifyContact(entry.ColliderB, entry.ColliderA, entry.Type, entry.Points, false);
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

SPtr<PhysicsMaterial> PhysX::CreateMaterial(float staticFriction, float dynamicFriction, float restitution)
{
	return bs_core_ptr_new<PhysXMaterial>(mPhysics, staticFriction, dynamicFriction, restitution);
}

SPtr<PhysicsMesh> PhysX::CreateMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
{
	return bs_core_ptr_new<PhysXMesh>(meshData, type);
}

SPtr<PhysicsScene> PhysX::CreatePhysicsScene()
{
	SPtr<PhysXScene> scene = bs_shared_ptr_new<PhysXScene>(mPhysics, mInitDesc, mScale);
	mScenes.push_back(scene.get());

	return scene;
}

void PhysX::NotifySceneDestroyedInternal(PhysXScene* scene)
{
	auto iterFind = std::find(mScenes.begin(), mScenes.end(), scene);
	assert(iterFind != mScenes.end());

	mScenes.erase(iterFind);
}

void PhysX::SetPaused(bool paused)
{
	mPaused = paused;
}

bool PhysX::RayCastInternal(const Vector3& origin, const Vector3& unitDir, const Collider& collider, PhysicsQueryHit& hit, float maxDist) const
{
	FPhysXCollider* physxCollider = static_cast<FPhysXCollider*>(collider.GetInternalInternal());
	PxShape* shape = physxCollider->GetShapeInternal();

	PxTransform transform = toPxTransform(collider.GetPosition(), collider.GetRotation());

	PxRaycastHit hitInfo;
	PxU32 maxHits = 1;
	bool anyHit = false;
	PxHitFlags hitFlags = PxHitFlag::eDEFAULT | PxHitFlag::eUV;
	PxU32 hitCount = PxGeometryQuery::raycast(toPxVector(origin), toPxVector(unitDir), shape->getGeometry().any(), transform, maxDist, hitFlags, maxHits, &hitInfo, anyHit);

	if(hitCount > 0)
		parseHit(hitInfo, hit, shape); // We have to provide a hint for the tested shape, as it is not contained in single-geometry raycast hit results

	return hitCount > 0;
}

PhysXScene::PhysXScene(PxPhysics* physics, const PHYSICS_INIT_DESC& input, const physx::PxTolerancesScale& scale)
	: mPhysics(physics)
{
	PxSceneDesc sceneDesc(scale); // TODO - Test out various other parameters provided by scene desc
	sceneDesc.gravity = toPxVector(input.Gravity);
	sceneDesc.cpuDispatcher = &gPhysXCPUDispatcher;
	sceneDesc.filterShader = PhysXFilterShader;
	sceneDesc.simulationEventCallback = &gPhysXEventCallback;
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

	gPhysX().NotifySceneDestroyedInternal(this);
}

SPtr<Rigidbody> PhysXScene::CreateRigidbody(const HSceneObject& linkedSO)
{
	return bs_shared_ptr_new<PhysXRigidbody>(mPhysics, mScene, linkedSO);
}

SPtr<BoxCollider> PhysXScene::CreateBoxCollider(const Vector3& extents, const Vector3& position, const Quaternion& rotation)
{
	return bs_shared_ptr_new<PhysXBoxCollider>(mPhysics, mScene, position, rotation, extents);
}

SPtr<SphereCollider> PhysXScene::CreateSphereCollider(float radius, const Vector3& position, const Quaternion& rotation)
{
	return bs_shared_ptr_new<PhysXSphereCollider>(mPhysics, mScene, position, rotation, radius);
}

SPtr<PlaneCollider> PhysXScene::CreatePlaneCollider(const Vector3& position, const Quaternion& rotation)
{
	return bs_shared_ptr_new<PhysXPlaneCollider>(mPhysics, mScene, position, rotation);
}

SPtr<CapsuleCollider> PhysXScene::CreateCapsuleCollider(float radius, float halfHeight, const Vector3& position, const Quaternion& rotation)
{
	return bs_shared_ptr_new<PhysXCapsuleCollider>(mPhysics, mScene, position, rotation, radius, halfHeight);
}

SPtr<MeshCollider> PhysXScene::CreateMeshCollider(const Vector3& position, const Quaternion& rotation)
{
	return bs_shared_ptr_new<PhysXMeshCollider>(mPhysics, mScene, position, rotation);
}

SPtr<FixedJoint> PhysXScene::CreateFixedJoint(const FIXED_JOINT_DESC& desc)
{
	return bs_shared_ptr_new<PhysXFixedJoint>(mPhysics, desc);
}

SPtr<DistanceJoint> PhysXScene::CreateDistanceJoint(const DISTANCE_JOINT_DESC& desc)
{
	return bs_shared_ptr_new<PhysXDistanceJoint>(mPhysics, desc);
}

SPtr<HingeJoint> PhysXScene::CreateHingeJoint(const HINGE_JOINT_DESC& desc)
{
	return bs_shared_ptr_new<PhysXHingeJoint>(mPhysics, desc);
}

SPtr<SphericalJoint> PhysXScene::CreateSphericalJoint(const SPHERICAL_JOINT_DESC& desc)
{
	return bs_shared_ptr_new<PhysXSphericalJoint>(mPhysics, desc);
}

SPtr<SliderJoint> PhysXScene::CreateSliderJoint(const SLIDER_JOINT_DESC& desc)
{
	return bs_shared_ptr_new<PhysXSliderJoint>(mPhysics, desc);
}

SPtr<D6Joint> PhysXScene::CreateD6Joint(const D6_JOINT_DESC& desc)
{
	return bs_shared_ptr_new<PhysXD6Joint>(mPhysics, desc);
}

SPtr<CharacterController> PhysXScene::CreateCharacterController(const CHAR_CONTROLLER_DESC& desc)
{
	return bs_shared_ptr_new<PhysXCharacterController>(mCharManager, desc);
}

Vector<PhysicsQueryHit> PhysXScene::SweepAll(const PxGeometry& geometry, const PxTransform& tfrm, const Vector3& unitDir, u64 layer, float maxDist) const
{
	PhysXSweepQueryCallback output;

	PxQueryFilterData filterData;
	memcpy(&filterData.data.word0, &layer, sizeof(layer));

	mScene->sweep(geometry, tfrm, toPxVector(unitDir), maxDist, output, PxHitFlag::eDEFAULT | PxHitFlag::eUV, filterData);

	return output.Data;
}

bool PhysXScene::SweepAny(const PxGeometry& geometry, const PxTransform& tfrm, const Vector3& unitDir, u64 layer, float maxDist) const
{
	PxSweepBuffer output;

	PxQueryFilterData filterData;
	filterData.flags |= PxQueryFlag::eANY_HIT;
	memcpy(&filterData.data.word0, &layer, sizeof(layer));

	return mScene->sweep(geometry, tfrm, toPxVector(unitDir), maxDist, output, PxHitFlag::eDEFAULT | PxHitFlag::eUV | PxHitFlag::eMESH_ANY, filterData);
}

bool PhysXScene::RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer, float max) const
{
	PxRaycastBuffer output;

	PxQueryFilterData filterData;
	memcpy(&filterData.data.word0, &layer, sizeof(layer));

	bool wasHit = mScene->raycast(toPxVector(origin), toPxVector(unitDir), max, output, PxHitFlag::eDEFAULT | PxHitFlag::eUV, filterData);

	if(wasHit)
		parseHit(output.block, hit);

	return wasHit;
}

bool PhysXScene::BoxCast(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer, float max) const
{
	PxBoxGeometry geometry(toPxVector(box.GetHalfSize()));
	PxTransform transform = toPxTransform(box.GetCenter(), rotation);

	return Sweep(geometry, transform, unitDir, hit, layer, max);
}

bool PhysXScene::SphereCast(const Sphere& sphere, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer, float max) const
{
	PxSphereGeometry geometry(sphere.GetRadius());
	PxTransform transform = toPxTransform(sphere.GetCenter(), Quaternion::IDENTITY);

	return Sweep(geometry, transform, unitDir, hit, layer, max);
}

bool PhysXScene::CapsuleCast(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer, float max) const
{
	PxCapsuleGeometry geometry(capsule.GetRadius(), capsule.GetHeight() * 0.5f);
	PxTransform transform = toPxTransform(capsule.GetCenter(), Quaternion::IDENTITY);

	return Sweep(geometry, transform, unitDir, hit, layer, max);
}

bool PhysXScene::ConvexCast(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer, float max) const
{
	if(mesh == nullptr)
		return false;

	if(mesh->GetType() != PhysicsMeshType::Convex)
		return false;

	FPhysXMesh* physxMesh = static_cast<FPhysXMesh*>(mesh->GetInternalInternal());
	PxConvexMeshGeometry geometry(physxMesh->GetConvexInternal());
	PxTransform transform = toPxTransform(position, rotation);

	return Sweep(geometry, transform, unitDir, hit, layer, max);
}

Vector<PhysicsQueryHit> PhysXScene::RayCastAll(const Vector3& origin, const Vector3& unitDir, u64 layer, float max) const
{
	PhysXRaycastQueryCallback output;

	PxQueryFilterData filterData;
	memcpy(&filterData.data.word0, &layer, sizeof(layer));

	mScene->raycast(toPxVector(origin), toPxVector(unitDir), max, output, PxHitFlag::eDEFAULT | PxHitFlag::eUV | PxHitFlag::eMESH_MULTIPLE, filterData);

	return output.Data;
}

Vector<PhysicsQueryHit> PhysXScene::BoxCastAll(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer, float max) const
{
	PxBoxGeometry geometry(toPxVector(box.GetHalfSize()));
	PxTransform transform = toPxTransform(box.GetCenter(), rotation);

	return SweepAll(geometry, transform, unitDir, layer, max);
}

Vector<PhysicsQueryHit> PhysXScene::SphereCastAll(const Sphere& sphere, const Vector3& unitDir, u64 layer, float max) const
{
	PxSphereGeometry geometry(sphere.GetRadius());
	PxTransform transform = toPxTransform(sphere.GetCenter(), Quaternion::IDENTITY);

	return SweepAll(geometry, transform, unitDir, layer, max);
}

Vector<PhysicsQueryHit> PhysXScene::CapsuleCastAll(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer, float max) const
{
	PxCapsuleGeometry geometry(capsule.GetRadius(), capsule.GetHeight() * 0.5f);
	PxTransform transform = toPxTransform(capsule.GetCenter(), Quaternion::IDENTITY);

	return SweepAll(geometry, transform, unitDir, layer, max);
}

Vector<PhysicsQueryHit> PhysXScene::ConvexCastAll(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer, float max) const
{
	if(mesh == nullptr)
		return Vector<PhysicsQueryHit>(0);

	if(mesh->GetType() != PhysicsMeshType::Convex)
		return Vector<PhysicsQueryHit>(0);

	FPhysXMesh* physxMesh = static_cast<FPhysXMesh*>(mesh->GetInternalInternal());
	PxConvexMeshGeometry geometry(physxMesh->GetConvexInternal());
	PxTransform transform = toPxTransform(position, rotation);

	return SweepAll(geometry, transform, unitDir, layer, max);
}

bool PhysXScene::RayCastAny(const Vector3& origin, const Vector3& unitDir, u64 layer, float max) const
{
	PxRaycastBuffer output;

	PxQueryFilterData filterData;
	filterData.flags |= PxQueryFlag::eANY_HIT;
	memcpy(&filterData.data.word0, &layer, sizeof(layer));

	return mScene->raycast(toPxVector(origin), toPxVector(unitDir), max, output, PxHitFlag::eDEFAULT | PxHitFlag::eUV | PxHitFlag::eMESH_ANY, filterData);
}

bool PhysXScene::BoxCastAny(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer, float max) const
{
	PxBoxGeometry geometry(toPxVector(box.GetHalfSize()));
	PxTransform transform = toPxTransform(box.GetCenter(), rotation);

	return SweepAny(geometry, transform, unitDir, layer, max);
}

bool PhysXScene::SphereCastAny(const Sphere& sphere, const Vector3& unitDir, u64 layer, float max) const
{
	PxSphereGeometry geometry(sphere.GetRadius());
	PxTransform transform = toPxTransform(sphere.GetCenter(), Quaternion::IDENTITY);

	return SweepAny(geometry, transform, unitDir, layer, max);
}

bool PhysXScene::CapsuleCastAny(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer, float max) const
{
	PxCapsuleGeometry geometry(capsule.GetRadius(), capsule.GetHeight() * 0.5f);
	PxTransform transform = toPxTransform(capsule.GetCenter(), Quaternion::IDENTITY);

	return SweepAny(geometry, transform, unitDir, layer, max);
}

bool PhysXScene::ConvexCastAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer, float max) const
{
	if(mesh == nullptr)
		return false;

	if(mesh->GetType() != PhysicsMeshType::Convex)
		return false;

	FPhysXMesh* physxMesh = static_cast<FPhysXMesh*>(mesh->GetInternalInternal());
	PxConvexMeshGeometry geometry(physxMesh->GetConvexInternal());
	PxTransform transform = toPxTransform(position, rotation);

	return SweepAny(geometry, transform, unitDir, layer, max);
}

Vector<Collider*> PhysXScene::BoxOverlapInternal(const AABox& box, const Quaternion& rotation, u64 layer) const
{
	PxBoxGeometry geometry(toPxVector(box.GetHalfSize()));
	PxTransform transform = toPxTransform(box.GetCenter(), rotation);

	return Overlap(geometry, transform, layer);
}

Vector<Collider*> PhysXScene::SphereOverlapInternal(const Sphere& sphere, u64 layer) const
{
	PxSphereGeometry geometry(sphere.GetRadius());
	PxTransform transform = toPxTransform(sphere.GetCenter(), Quaternion::IDENTITY);

	return Overlap(geometry, transform, layer);
}

Vector<Collider*> PhysXScene::CapsuleOverlapInternal(const Capsule& capsule, const Quaternion& rotation, u64 layer) const
{
	PxCapsuleGeometry geometry(capsule.GetRadius(), capsule.GetHeight() * 0.5f);
	PxTransform transform = toPxTransform(capsule.GetCenter(), Quaternion::IDENTITY);

	return Overlap(geometry, transform, layer);
}

Vector<Collider*> PhysXScene::ConvexOverlapInternal(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer) const
{
	if(mesh == nullptr)
		return Vector<Collider*>(0);

	if(mesh->GetType() != PhysicsMeshType::Convex)
		return Vector<Collider*>(0);

	FPhysXMesh* physxMesh = static_cast<FPhysXMesh*>(mesh->GetInternalInternal());
	PxConvexMeshGeometry geometry(physxMesh->GetConvexInternal());
	PxTransform transform = toPxTransform(position, rotation);

	return Overlap(geometry, transform, layer);
}

bool PhysXScene::BoxOverlapAny(const AABox& box, const Quaternion& rotation, u64 layer) const
{
	PxBoxGeometry geometry(toPxVector(box.GetHalfSize()));
	PxTransform transform = toPxTransform(box.GetCenter(), rotation);

	return OverlapAny(geometry, transform, layer);
}

bool PhysXScene::SphereOverlapAny(const Sphere& sphere, u64 layer) const
{
	PxSphereGeometry geometry(sphere.GetRadius());
	PxTransform transform = toPxTransform(sphere.GetCenter(), Quaternion::IDENTITY);

	return OverlapAny(geometry, transform, layer);
}

bool PhysXScene::CapsuleOverlapAny(const Capsule& capsule, const Quaternion& rotation, u64 layer) const
{
	PxCapsuleGeometry geometry(capsule.GetRadius(), capsule.GetHeight() * 0.5f);
	PxTransform transform = toPxTransform(capsule.GetCenter(), Quaternion::IDENTITY);

	return OverlapAny(geometry, transform, layer);
}

bool PhysXScene::ConvexOverlapAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer) const
{
	if(mesh == nullptr)
		return false;

	if(mesh->GetType() != PhysicsMeshType::Convex)
		return false;

	FPhysXMesh* physxMesh = static_cast<FPhysXMesh*>(mesh->GetInternalInternal());
	PxConvexMeshGeometry geometry(physxMesh->GetConvexInternal());
	PxTransform transform = toPxTransform(position, rotation);

	return OverlapAny(geometry, transform, layer);
}

bool PhysXScene::Sweep(const PxGeometry& geometry, const PxTransform& tfrm, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer, float maxDist) const
{
	PxSweepBuffer output;

	PxQueryFilterData filterData;
	memcpy(&filterData.data.word0, &layer, sizeof(layer));

	bool wasHit = mScene->sweep(geometry, tfrm, toPxVector(unitDir), maxDist, output, PxHitFlag::eDEFAULT | PxHitFlag::eUV, filterData);

	if(wasHit)
		parseHit(output.block, hit);

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

Vector<Collider*> PhysXScene::Overlap(const PxGeometry& geometry, const PxTransform& tfrm, u64 layer) const
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
	return fromPxVector(mScene->getGravity());
}

void PhysXScene::SetGravity(const Vector3& gravity)
{
	mScene->setGravity(toPxVector(gravity));
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
	pxRegion.bounds = PxBounds3(toPxVector(region.GetMin()), toPxVector(region.GetMax()));
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

namespace bs {
PhysX& gPhysX()
{
	return static_cast<PhysX&>(PhysX::Instance());
}
} // namespace bs
