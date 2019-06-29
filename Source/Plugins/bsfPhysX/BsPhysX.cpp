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

namespace bs
{
	class PhysXAllocator : public PxAllocatorCallback
	{
	public:
		void* allocate(size_t size, const char*, const char*, int) override
		{
			void* ptr = bs_alloc_aligned16((UINT32)size);
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

			UINT32 severity = 0;

			if ((code & PxErrorCode::eDEBUG_INFO) != 0)
			{
				errorCode = "Info";
				severity = 0;
			}

			if((code & PxErrorCode::eINVALID_PARAMETER) != 0)
			{
				errorCode = "Invalid parameter";
				severity = 1;
			}

			if ((code & PxErrorCode::eINVALID_OPERATION) != 0)
			{
				errorCode = "Invalid operation";
				severity = 1;
			}

			if ((code & PxErrorCode::eDEBUG_WARNING) != 0)
			{
				errorCode = "Generic";
				severity = 1;
			}

			if ((code & PxErrorCode::ePERF_WARNING) != 0)
			{
				errorCode = "Performance";
				severity = 1;
			}

			if ((code & PxErrorCode::eOUT_OF_MEMORY) != 0)
			{
				errorCode = "Out of memory";
				severity = 2;
			}

			if ((code & PxErrorCode::eABORT) != 0)
			{
				errorCode = "Abort";
				severity = 2;
			}

			if ((code & PxErrorCode::eINTERNAL_ERROR) != 0)
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
		void onWake(PxActor** actors, PxU32 count) override { /* Do nothing */ }
		void onSleep(PxActor** actors, PxU32 count) override { /* Do nothing */ }

		void onTrigger(PxTriggerPair* pairs, PxU32 count) override
		{
			for (PxU32 i = 0; i < count; i++)
			{
				const PxTriggerPair& pair = pairs[i];
				if (pair.triggerShape->userData == nullptr)
					continue;
				
				PhysX::ContactEventType type;
				bool ignoreContact = false;
				PhysXObjectFilterFlags flags = PhysXObjectFilterFlags(pair.triggerShape->getSimulationFilterData().word2);

				if (flags.isSet(PhysXObjectFilterFlag::ReportAll))
				{
					switch ((UINT32)pair.status)
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
				else if (flags.isSet(PhysXObjectFilterFlag::ReportBasic))
				{
					switch ((UINT32)pair.status)
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

				if (ignoreContact)
					continue;

				PhysX::TriggerEvent event;
				event.trigger = (Collider*)pair.triggerShape->userData;
				event.other = (Collider*)pair.otherShape->userData;
				event.type = type;

				gPhysX()._reportTriggerEvent(event);
			}
		}

		void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 count) override
		{
			for (PxU32 i = 0; i < count; i++)
			{
				const PxContactPair& pair = pairs[i];

				PhysX::ContactEventType type;
				bool ignoreContact = false;
				switch((UINT32)pair.events)
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

				if (ignoreContact)
					continue;

				PhysX::ContactEvent event;
				event.type = type;

				PxU32 contactCount = pair.contactCount;
				const PxU8* stream = pair.contactStream;
				PxU16 streamSize = pair.contactStreamSize;

				if (contactCount > 0 && streamSize > 0)
				{
					PxU32 contactIdx = 0;
					PxContactStreamIterator iter((PxU8*)stream, streamSize);

					stream += ((streamSize + 15) & ~15);

					const PxReal* impulses = reinterpret_cast<const PxReal*>(stream);
					PxU32 hasImpulses = (pair.flags & PxContactPairFlag::eINTERNAL_HAS_IMPULSES);

					while (iter.hasNextPatch())
					{
						iter.nextPatch();
						while (iter.hasNextContact())
						{
							iter.nextContact();

							ContactPoint point;
							point.position = fromPxVector(iter.getContactPoint());
							point.separation = iter.getSeparation();
							point.normal = fromPxVector(iter.getContactNormal());

							if (hasImpulses)
								point.impulse = impulses[contactIdx];
							else
								point.impulse = 0.0f;

							event.points.push_back(point);

							contactIdx++;
						}
					}
				}

				event.colliderA = (Collider*)pair.shapes[0]->userData;
				event.colliderB = (Collider*)pair.shapes[1]->userData;

				gPhysX()._reportContactEvent(event);
			}
		}

		void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override 
		{ 
			for (UINT32 i = 0; i < count; i++)
			{
				PxConstraintInfo& constraintInfo = constraints[i];

				if (constraintInfo.type != PxConstraintExtIDs::eJOINT)
					continue;

				PxJoint* pxJoint = (PxJoint*)constraintInfo.externalReference;

				PhysX::JointBreakEvent event;
				event.joint = (Joint*)pxJoint->userData;

				if(event.joint != nullptr)
					gPhysX()._reportJointBreakEvent(event);
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

			auto runTask = [&]() { physxTask.run(); physxTask.release(); };
			SPtr<Task> task = Task::create("PhysX", runTask);

			TaskScheduler::instance().addTask(task);
		}

		PxU32 getWorkerCount() const override
		{
			return (PxU32)TaskScheduler::instance().getNumWorkers();
		}
	};

	class PhysXBroadPhaseCallback : public PxBroadPhaseCallback
	{
		void onObjectOutOfBounds(PxShape& shape, PxActor& actor) override
		{
			Collider* collider = (Collider*)shape.userData;
			if (collider != nullptr)
				BS_LOG(Warning, Physics, "Physics object out of bounds. Consider increasing broadphase region!");
		}

		void onObjectOutOfBounds(PxAggregate& aggregate) override { /* Do nothing */ }
	};

	PxFilterFlags PhysXFilterShader(PxFilterObjectAttributes attr0, PxFilterData data0, PxFilterObjectAttributes attr1, 
		PxFilterData data1, PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		PhysXObjectFilterFlags flags0 = PhysXObjectFilterFlags(data0.word2);
		PhysXObjectFilterFlags flags1 = PhysXObjectFilterFlags(data1.word2);

		if (flags0.isSet(PhysXObjectFilterFlag::ReportAll) || flags1.isSet(PhysXObjectFilterFlag::ReportAll))
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_LOST | PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eNOTIFY_CONTACT_POINTS;
		else if (flags0.isSet(PhysXObjectFilterFlag::ReportBasic) || flags1.isSet(PhysXObjectFilterFlag::ReportBasic))
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_LOST | PxPairFlag::eNOTIFY_CONTACT_POINTS;

		if (PxFilterObjectIsTrigger(attr0) || PxFilterObjectIsTrigger(attr1))
		{
			if (!pairFlags)
				return PxFilterFlag::eSUPPRESS; // Trigger with no notify flags

			pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;
			return PxFilterFlags();
		}

		UINT64 groupA = *(UINT64*)&data0.word0;
		UINT64 groupB = *(UINT64*)&data1.word0;

		bool canCollide = gPhysics().isCollisionEnabled(groupA, groupB);
		if (!canCollide)
			return PxFilterFlag::eSUPPRESS;

		if (flags0.isSet(PhysXObjectFilterFlag::CCD) || flags1.isSet(PhysXObjectFilterFlag::CCD))
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

		if (shape != nullptr && shape->getGeometryType() == PxGeometryType::eTRIANGLEMESH)
		{
			PxTriangleMeshGeometry triMeshGeometry;
			shape->getTriangleMeshGeometry(triMeshGeometry);

			if (triMeshGeometry.isValid() && triMeshGeometry.triangleMesh->getTrianglesRemap() != nullptr)
			{
				output.unmappedTriangleIdx = triMeshGeometry.triangleMesh->getTrianglesRemap()[input.faceIndex];
				return;
			}
		}

		output.unmappedTriangleIdx = input.faceIndex;
	}

	void parseHit(const PxRaycastHit& input, PhysicsQueryHit& output, PxShape* shapeHint = nullptr)
	{
		output.point = fromPxVector(input.position);
		output.normal = fromPxVector(input.normal);
		output.distance = input.distance;
		output.triangleIdx = input.faceIndex;
		setUnmappedTriangleIndex(input, output, shapeHint);
		output.uv = Vector2(input.u, input.v);

		if(input.shape)
			output.colliderRaw = (Collider*)input.shape->userData;

		if (output.colliderRaw != nullptr)
		{
			CCollider* component = (CCollider*)output.colliderRaw->_getOwner(PhysicsOwnerType::Component);
			if (component != nullptr)
				output.collider = static_object_cast<CCollider>(component->getHandle());
		}
	}

	void parseHit(const PxSweepHit& input, PhysicsQueryHit& output, PxShape* shapeHint = nullptr)
	{
		output.point = fromPxVector(input.position);
		output.normal = fromPxVector(input.normal);
		output.uv = Vector2::ZERO;
		output.distance = input.distance;
		output.triangleIdx = input.faceIndex;
		setUnmappedTriangleIndex(input, output, shapeHint);
		output.colliderRaw = (Collider*)input.shape->userData;

		if (output.colliderRaw != nullptr)
		{
			CCollider* component = (CCollider*)output.colliderRaw->_getOwner(PhysicsOwnerType::Component);
			if (component != nullptr)
				output.collider = static_object_cast<CCollider>(component->getHandle());
		}
	}

	struct PhysXRaycastQueryCallback : PxRaycastCallback
	{
		static const int MAX_HITS = 32;
		PxRaycastHit buffer[MAX_HITS];

		Vector<PhysicsQueryHit> data;

		PhysXRaycastQueryCallback()
			:PxRaycastCallback(buffer, MAX_HITS)
		{ }

		PxAgain processTouches(const PxRaycastHit* buffer, PxU32 nbHits) override
		{
			for (PxU32 i = 0; i < nbHits; i++)
			{
				data.push_back(PhysicsQueryHit());
				parseHit(buffer[i], data.back());
			}

			return true;
		}
	};

	struct PhysXSweepQueryCallback : PxSweepCallback
	{
		static const int MAX_HITS = 32;
		PxSweepHit buffer[MAX_HITS];

		Vector<PhysicsQueryHit> data;

		PhysXSweepQueryCallback()
			:PxSweepCallback(buffer, MAX_HITS)
		{ }

		PxAgain processTouches(const PxSweepHit* buffer, PxU32 nbHits) override
		{
			for (PxU32 i = 0; i < nbHits; i++)
			{
				data.push_back(PhysicsQueryHit());
				parseHit(buffer[i], data.back());
			}

			return true;
		}
	};

	struct PhysXOverlapQueryCallback : PxOverlapCallback
	{
		static const int MAX_HITS = 32;
		PxOverlapHit buffer[MAX_HITS];

		Vector<Collider*> data;

		PhysXOverlapQueryCallback()
			:PxOverlapCallback(buffer, MAX_HITS)
		{ }

		PxAgain processTouches(const PxOverlapHit* buffer, PxU32 nbHits) override
		{
			for (PxU32 i = 0; i < nbHits; i++)
				data.push_back((Collider*)buffer[i].shape->userData);

			return true;
		}
	};

	static PhysXAllocator gPhysXAllocator;
	static PhysXErrorCallback gPhysXErrorHandler;
	static PhysXCPUDispatcher gPhysXCPUDispatcher;
	static PhysXEventCallback gPhysXEventCallback;
	static PhysXBroadPhaseCallback gPhysXBroadphaseCallback;

	static const UINT32 SIZE_16K = 1 << 14;
	const UINT32 PhysX::SCRATCH_BUFFER_SIZE = SIZE_16K * 64; // 1MB by default

	PhysX::PhysX(const PHYSICS_INIT_DESC& input)
		:Physics(input), mInitDesc(input)
	{
		mScale.length = input.typicalLength;
		mScale.speed = input.typicalSpeed;

		mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gPhysXAllocator, gPhysXErrorHandler);
		mPhysics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *mFoundation, mScale);

		PxRegisterArticulations(*mPhysics);

		if (input.initCooking)
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

		if (mCooking != nullptr)
			mCooking->release();

		mPhysics->release();
		mFoundation->release();
	}

	void PhysX::fixedUpdate(float step)
	{
		if (mPaused)
			return;

		mUpdateInProgress = true;

		// Note: Consider delaying fetchResults one frame. This could improve performance because Physics update would be
		//       able to run parallel to the simulation thread, but at a cost to input latency.
		bs_frame_mark();
		UINT8* scratchBuffer = bs_frame_alloc_aligned(SCRATCH_BUFFER_SIZE, 16);

		for(auto& scene : mScenes)
		{
			scene->mScene->simulate(step, nullptr, scratchBuffer, SCRATCH_BUFFER_SIZE);

			UINT32 errorState;
			if (!scene->mScene->fetchResults(true, &errorState))
				BS_LOG(Warning, Physics, "Physics simulation failed. Error code: {0}" + errorState);
		}

		bs_frame_free_aligned(scratchBuffer);
		bs_frame_clear();

		// Update rigidbodies with new transforms
		for(auto& scene : mScenes)
		{
			PxU32 numActiveTransforms;
			const PxActiveTransform* activeTransforms = scene->mScene->getActiveTransforms(numActiveTransforms);

			for (PxU32 i = 0; i < numActiveTransforms; i++)
			{
				Rigidbody* rigidbody = static_cast<Rigidbody*>(activeTransforms[i].userData);

				// Note: This should never happen, as actors gets their userData set to null when they're destroyed. However
				// in some cases PhysX seems to keep those actors alive for a frame or few, and reports their state here. Until
				// I find out why I need to perform this check.
				if (activeTransforms[i].actor->userData == nullptr)
					continue;

				const PxTransform& transform = activeTransforms[i].actor2World;

				// Note: Make this faster, avoid dereferencing Rigidbody and attempt to access pos/rot destination directly,
				//       use non-temporal writes
				rigidbody->_setTransform(fromPxVector(transform.p), fromPxQuaternion(transform.q));
			}
		}

		// Note: Consider extrapolating for the remaining "simulationAmount" value
		mUpdateInProgress = false;

		triggerEvents();
	}

	void PhysX::update()
	{
		// Note: Potentially interpolate (would mean a one frame delay needs to be introduced)
	}

	void PhysX::_reportContactEvent(const ContactEvent& event)
	{
		mContactEvents.push_back(event);
	}

	void PhysX::_reportTriggerEvent(const TriggerEvent& event)
	{
		mTriggerEvents.push_back(event);
	}

	void PhysX::_reportJointBreakEvent(const JointBreakEvent& event)
	{
		mJointBreakEvents.push_back(event);
	}

	void PhysX::triggerEvents()
	{
		CollisionDataRaw data;

		for(auto& entry : mTriggerEvents)
		{
			data.colliders[0] = entry.trigger;
			data.colliders[1] = entry.other;

			switch (entry.type)
			{
			case ContactEventType::ContactBegin:
				entry.trigger->onCollisionBegin(data);
				break;
			case ContactEventType::ContactStay:
				entry.trigger->onCollisionStay(data);
				break;
			case ContactEventType::ContactEnd:
				entry.trigger->onCollisionEnd(data);
				break;
			}
		}

		auto notifyContact = [&](Collider* obj, Collider* other, ContactEventType type, 
			const Vector<ContactPoint>& points, bool flipNormals = false)
		{
			data.colliders[0] = obj;
			data.colliders[1] = other;
			data.contactPoints = points;

			if(flipNormals)
			{
				for (auto& point : data.contactPoints)
					point.normal = -point.normal;
			}

			Rigidbody* rigidbody = obj->getRigidbody();
			if(rigidbody != nullptr)
			{
				switch (type)
				{
				case ContactEventType::ContactBegin:
					rigidbody->onCollisionBegin(data);
					break;
				case ContactEventType::ContactStay:
					rigidbody->onCollisionStay(data);
					break;
				case ContactEventType::ContactEnd:
					rigidbody->onCollisionEnd(data);
					break;
				}
			}
			else
			{
				switch (type)
				{
				case ContactEventType::ContactBegin:
					obj->onCollisionBegin(data);
					break;
				case ContactEventType::ContactStay:
					obj->onCollisionStay(data);
					break;
				case ContactEventType::ContactEnd:
					obj->onCollisionEnd(data);
					break;
				}
			}
		};

		for (auto& entry : mContactEvents)
		{
			if (entry.colliderA != nullptr)
			{
				CollisionReportMode reportModeA = entry.colliderA->getCollisionReportMode();

				if (reportModeA == CollisionReportMode::ReportPersistent)
					notifyContact(entry.colliderA, entry.colliderB, entry.type, entry.points, true);
				else if (reportModeA == CollisionReportMode::Report && entry.type != ContactEventType::ContactStay)
					notifyContact(entry.colliderA, entry.colliderB, entry.type, entry.points, true);
			}

			if (entry.colliderB != nullptr)
			{
				CollisionReportMode reportModeB = entry.colliderB->getCollisionReportMode();

				if (reportModeB == CollisionReportMode::ReportPersistent)
					notifyContact(entry.colliderB, entry.colliderA, entry.type, entry.points, false);
				else if (reportModeB == CollisionReportMode::Report && entry.type != ContactEventType::ContactStay)
					notifyContact(entry.colliderB, entry.colliderA, entry.type, entry.points, false);
			}
		}

		for(auto& entry : mJointBreakEvents)
		{
			entry.joint->onJointBreak();
		}

		mTriggerEvents.clear();
		mContactEvents.clear();
		mJointBreakEvents.clear();
	}

	SPtr<PhysicsMaterial> PhysX::createMaterial(float staticFriction, float dynamicFriction, float restitution)
	{
		return bs_core_ptr_new<PhysXMaterial>(mPhysics, staticFriction, dynamicFriction, restitution);
	}

	SPtr<PhysicsMesh> PhysX::createMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
	{
		return bs_core_ptr_new<PhysXMesh>(meshData, type);
	}

	SPtr<PhysicsScene> PhysX::createPhysicsScene()
	{
		SPtr<PhysXScene> scene = bs_shared_ptr_new<PhysXScene>(mPhysics, mInitDesc, mScale);
		mScenes.push_back(scene.get());

		return scene;
	}

	void PhysX::_notifySceneDestroyed(PhysXScene* scene)
	{
		auto iterFind = std::find(mScenes.begin(), mScenes.end(), scene);
		assert(iterFind != mScenes.end());

		mScenes.erase(iterFind);
	}

	void PhysX::setPaused(bool paused)
	{
		mPaused = paused;
	}

	bool PhysX::_rayCast(const Vector3& origin, const Vector3& unitDir, const Collider& collider, PhysicsQueryHit& hit,
		float maxDist) const
	{
		FPhysXCollider* physxCollider = static_cast<FPhysXCollider*>(collider._getInternal());
		PxShape* shape = physxCollider->_getShape();

		PxTransform transform = toPxTransform(collider.getPosition(), collider.getRotation());

		PxRaycastHit hitInfo;
		PxU32 maxHits = 1;
		bool anyHit = false;
		PxHitFlags hitFlags = PxHitFlag::eDEFAULT | PxHitFlag::eUV;
		PxU32 hitCount = PxGeometryQuery::raycast(toPxVector(origin), toPxVector(unitDir),
			shape->getGeometry().any(), transform,
			maxDist, hitFlags, maxHits, &hitInfo, anyHit);

		if(hitCount > 0)
			parseHit(hitInfo, hit, shape); // We have to provide a hint for the tested shape, as it is not contained in single-geometry raycast hit results

		return hitCount > 0;
	}

	PhysXScene::PhysXScene(PxPhysics* physics, const PHYSICS_INIT_DESC& input, const physx::PxTolerancesScale& scale)
		:mPhysics(physics)
	{
		PxSceneDesc sceneDesc(scale); // TODO - Test out various other parameters provided by scene desc
		sceneDesc.gravity = toPxVector(input.gravity);
		sceneDesc.cpuDispatcher = &gPhysXCPUDispatcher;
		sceneDesc.filterShader = PhysXFilterShader;
		sceneDesc.simulationEventCallback = &gPhysXEventCallback;
		sceneDesc.broadPhaseCallback = &gPhysXBroadphaseCallback;

		// Optionally: eENABLE_KINEMATIC_STATIC_PAIRS, eENABLE_KINEMATIC_PAIRS, eENABLE_PCM
		sceneDesc.flags = PxSceneFlag::eENABLE_ACTIVETRANSFORMS;

		if (input.flags.isSet(PhysicsFlag::CCD_Enable))
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

		gPhysX()._notifySceneDestroyed(this);
	}

	SPtr<Rigidbody> PhysXScene::createRigidbody(const HSceneObject& linkedSO)
	{
		return bs_shared_ptr_new<PhysXRigidbody>(mPhysics, mScene, linkedSO);
	}

	SPtr<BoxCollider> PhysXScene::createBoxCollider(const Vector3& extents, const Vector3& position,
		const Quaternion& rotation)
	{
		return bs_shared_ptr_new<PhysXBoxCollider>(mPhysics, mScene, position, rotation, extents);
	}

	SPtr<SphereCollider> PhysXScene::createSphereCollider(float radius, const Vector3& position, const Quaternion& rotation)
	{
		return bs_shared_ptr_new<PhysXSphereCollider>(mPhysics, mScene, position, rotation, radius);
	}

	SPtr<PlaneCollider> PhysXScene::createPlaneCollider(const Vector3& position, const Quaternion& rotation)
	{
		return bs_shared_ptr_new<PhysXPlaneCollider>(mPhysics, mScene, position, rotation);
	}

	SPtr<CapsuleCollider> PhysXScene::createCapsuleCollider(float radius, float halfHeight, const Vector3& position, 
		const Quaternion& rotation)
	{
		return bs_shared_ptr_new<PhysXCapsuleCollider>(mPhysics, mScene, position, rotation, radius, halfHeight);
	}

	SPtr<MeshCollider> PhysXScene::createMeshCollider(const Vector3& position, const Quaternion& rotation)
	{
		return bs_shared_ptr_new<PhysXMeshCollider>(mPhysics, mScene, position, rotation);
	}

	SPtr<FixedJoint> PhysXScene::createFixedJoint(const FIXED_JOINT_DESC& desc)
	{
		return bs_shared_ptr_new<PhysXFixedJoint>(mPhysics, desc);
	}

	SPtr<DistanceJoint> PhysXScene::createDistanceJoint(const DISTANCE_JOINT_DESC& desc)
	{
		return bs_shared_ptr_new<PhysXDistanceJoint>(mPhysics, desc);
	}

	SPtr<HingeJoint> PhysXScene::createHingeJoint(const HINGE_JOINT_DESC& desc)
	{
		return bs_shared_ptr_new<PhysXHingeJoint>(mPhysics, desc);
	}

	SPtr<SphericalJoint> PhysXScene::createSphericalJoint(const SPHERICAL_JOINT_DESC& desc)
	{
		return bs_shared_ptr_new<PhysXSphericalJoint>(mPhysics, desc);
	}

	SPtr<SliderJoint> PhysXScene::createSliderJoint(const SLIDER_JOINT_DESC& desc)
	{
		return bs_shared_ptr_new<PhysXSliderJoint>(mPhysics, desc);
	}

	SPtr<D6Joint> PhysXScene::createD6Joint(const D6_JOINT_DESC& desc)
	{
		return bs_shared_ptr_new<PhysXD6Joint>(mPhysics, desc);
	}

	SPtr<CharacterController> PhysXScene::createCharacterController(const CHAR_CONTROLLER_DESC& desc)
	{
		return bs_shared_ptr_new<PhysXCharacterController>(mCharManager, desc);
	}

	Vector<PhysicsQueryHit> PhysXScene::sweepAll(const PxGeometry& geometry, const PxTransform& tfrm, const Vector3& unitDir,
		UINT64 layer, float maxDist) const
	{
		PhysXSweepQueryCallback output;

		PxQueryFilterData filterData;
		memcpy(&filterData.data.word0, &layer, sizeof(layer));

		mScene->sweep(geometry, tfrm, toPxVector(unitDir), maxDist, output,
			PxHitFlag::eDEFAULT | PxHitFlag::eUV, filterData);

		return output.data;
	}

	bool PhysXScene::sweepAny(const PxGeometry& geometry, const PxTransform& tfrm, const Vector3& unitDir, UINT64 layer, 
		float maxDist) const
	{
		PxSweepBuffer output;

		PxQueryFilterData filterData;
		filterData.flags |= PxQueryFlag::eANY_HIT;
		memcpy(&filterData.data.word0, &layer, sizeof(layer));

		return mScene->sweep(geometry, tfrm, toPxVector(unitDir), maxDist, output, 
			PxHitFlag::eDEFAULT | PxHitFlag::eUV | PxHitFlag::eMESH_ANY, filterData);
	}

	bool PhysXScene::rayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, UINT64 layer, float max) const
	{
		PxRaycastBuffer output;

		PxQueryFilterData filterData;
		memcpy(&filterData.data.word0, &layer, sizeof(layer));

		bool wasHit = mScene->raycast(toPxVector(origin),
			toPxVector(unitDir), max, output, PxHitFlag::eDEFAULT | PxHitFlag::eUV, filterData);

		if (wasHit)
			parseHit(output.block, hit);

		return wasHit;
	}

	bool PhysXScene::boxCast(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit,
		UINT64 layer, float max) const
	{
		PxBoxGeometry geometry(toPxVector(box.getHalfSize()));
		PxTransform transform = toPxTransform(box.getCenter(), rotation);

		return sweep(geometry, transform, unitDir, hit, layer, max);
	}

	bool PhysXScene::sphereCast(const Sphere& sphere, const Vector3& unitDir, PhysicsQueryHit& hit,
		UINT64 layer, float max) const
	{
		PxSphereGeometry geometry(sphere.getRadius());
		PxTransform transform = toPxTransform(sphere.getCenter(), Quaternion::IDENTITY);

		return sweep(geometry, transform, unitDir, hit, layer, max);
	}

	bool PhysXScene::capsuleCast(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir,
		PhysicsQueryHit& hit, UINT64 layer, float max) const
	{
		PxCapsuleGeometry geometry(capsule.getRadius(), capsule.getHeight() * 0.5f);
		PxTransform transform = toPxTransform(capsule.getCenter(), Quaternion::IDENTITY);

		return sweep(geometry, transform, unitDir, hit, layer, max);
	}

	bool PhysXScene::convexCast(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
		const Vector3& unitDir, PhysicsQueryHit& hit, UINT64 layer, float max) const
	{
		if (mesh == nullptr)
			return false;

		if (mesh->getType() != PhysicsMeshType::Convex)
			return false;

		FPhysXMesh* physxMesh = static_cast<FPhysXMesh*>(mesh->_getInternal());
		PxConvexMeshGeometry geometry(physxMesh->_getConvex());
		PxTransform transform = toPxTransform(position, rotation);

		return sweep(geometry, transform, unitDir, hit, layer, max);
	}

	Vector<PhysicsQueryHit> PhysXScene::rayCastAll(const Vector3& origin, const Vector3& unitDir,
		UINT64 layer, float max) const
	{
		PhysXRaycastQueryCallback output;

		PxQueryFilterData filterData;
		memcpy(&filterData.data.word0, &layer, sizeof(layer));

		mScene->raycast(toPxVector(origin), toPxVector(unitDir), max, output,
			PxHitFlag::eDEFAULT | PxHitFlag::eUV | PxHitFlag::eMESH_MULTIPLE, filterData);

		return output.data;
	}

	Vector<PhysicsQueryHit> PhysXScene::boxCastAll(const AABox& box, const Quaternion& rotation,
		const Vector3& unitDir, UINT64 layer, float max) const
	{
		PxBoxGeometry geometry(toPxVector(box.getHalfSize()));
		PxTransform transform = toPxTransform(box.getCenter(), rotation);

		return sweepAll(geometry, transform, unitDir, layer, max);
	}

	Vector<PhysicsQueryHit> PhysXScene::sphereCastAll(const Sphere& sphere, const Vector3& unitDir,
		UINT64 layer, float max) const
	{
		PxSphereGeometry geometry(sphere.getRadius());
		PxTransform transform = toPxTransform(sphere.getCenter(), Quaternion::IDENTITY);

		return sweepAll(geometry, transform, unitDir, layer, max);
	}

	Vector<PhysicsQueryHit> PhysXScene::capsuleCastAll(const Capsule& capsule, const Quaternion& rotation,
		const Vector3& unitDir, UINT64 layer, float max) const
	{
		PxCapsuleGeometry geometry(capsule.getRadius(), capsule.getHeight() * 0.5f);
		PxTransform transform = toPxTransform(capsule.getCenter(), Quaternion::IDENTITY);

		return sweepAll(geometry, transform, unitDir, layer, max);
	}

	Vector<PhysicsQueryHit> PhysXScene::convexCastAll(const HPhysicsMesh& mesh, const Vector3& position,
		const Quaternion& rotation, const Vector3& unitDir, UINT64 layer, float max) const
	{
		if (mesh == nullptr)
			return Vector<PhysicsQueryHit>(0);

		if (mesh->getType() != PhysicsMeshType::Convex)
			return Vector<PhysicsQueryHit>(0);

		FPhysXMesh* physxMesh = static_cast<FPhysXMesh*>(mesh->_getInternal());
		PxConvexMeshGeometry geometry(physxMesh->_getConvex());
		PxTransform transform = toPxTransform(position, rotation);

		return sweepAll(geometry, transform, unitDir, layer, max);
	}

	bool PhysXScene::rayCastAny(const Vector3& origin, const Vector3& unitDir,
		UINT64 layer, float max) const
	{
		PxRaycastBuffer output;

		PxQueryFilterData filterData;
		filterData.flags |= PxQueryFlag::eANY_HIT;
		memcpy(&filterData.data.word0, &layer, sizeof(layer));

		return mScene->raycast(toPxVector(origin),
			toPxVector(unitDir), max, output, PxHitFlag::eDEFAULT | PxHitFlag::eUV | PxHitFlag::eMESH_ANY, filterData);
	}

	bool PhysXScene::boxCastAny(const AABox& box, const Quaternion& rotation, const Vector3& unitDir,
		UINT64 layer, float max) const
	{
		PxBoxGeometry geometry(toPxVector(box.getHalfSize()));
		PxTransform transform = toPxTransform(box.getCenter(), rotation);

		return sweepAny(geometry, transform, unitDir, layer, max);
	}

	bool PhysXScene::sphereCastAny(const Sphere& sphere, const Vector3& unitDir,
		UINT64 layer, float max) const
	{
		PxSphereGeometry geometry(sphere.getRadius());
		PxTransform transform = toPxTransform(sphere.getCenter(), Quaternion::IDENTITY);

		return sweepAny(geometry, transform, unitDir, layer, max);
	}

	bool PhysXScene::capsuleCastAny(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir,
		UINT64 layer, float max) const
	{
		PxCapsuleGeometry geometry(capsule.getRadius(), capsule.getHeight() * 0.5f);
		PxTransform transform = toPxTransform(capsule.getCenter(), Quaternion::IDENTITY);

		return sweepAny(geometry, transform, unitDir, layer, max);
	}

	bool PhysXScene::convexCastAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
		const Vector3& unitDir, UINT64 layer, float max) const
	{
		if (mesh == nullptr)
			return false;

		if (mesh->getType() != PhysicsMeshType::Convex)
			return false;

		FPhysXMesh* physxMesh = static_cast<FPhysXMesh*>(mesh->_getInternal());
		PxConvexMeshGeometry geometry(physxMesh->_getConvex());
		PxTransform transform = toPxTransform(position, rotation);

		return sweepAny(geometry, transform, unitDir, layer, max);
	}

	Vector<Collider*> PhysXScene::_boxOverlap(const AABox& box, const Quaternion& rotation,
		UINT64 layer) const
	{
		PxBoxGeometry geometry(toPxVector(box.getHalfSize()));
		PxTransform transform = toPxTransform(box.getCenter(), rotation);

		return overlap(geometry, transform, layer);
	}

	Vector<Collider*> PhysXScene::_sphereOverlap(const Sphere& sphere, UINT64 layer) const
	{
		PxSphereGeometry geometry(sphere.getRadius());
		PxTransform transform = toPxTransform(sphere.getCenter(), Quaternion::IDENTITY);

		return overlap(geometry, transform, layer);
	}

	Vector<Collider*> PhysXScene::_capsuleOverlap(const Capsule& capsule, const Quaternion& rotation,
		UINT64 layer) const
	{
		PxCapsuleGeometry geometry(capsule.getRadius(), capsule.getHeight() * 0.5f);
		PxTransform transform = toPxTransform(capsule.getCenter(), Quaternion::IDENTITY);

		return overlap(geometry, transform, layer);
	}

	Vector<Collider*> PhysXScene::_convexOverlap(const HPhysicsMesh& mesh, const Vector3& position,
		const Quaternion& rotation, UINT64 layer) const
	{
		if (mesh == nullptr)
			return Vector<Collider*>(0);

		if (mesh->getType() != PhysicsMeshType::Convex)
			return Vector<Collider*>(0);

		FPhysXMesh* physxMesh = static_cast<FPhysXMesh*>(mesh->_getInternal());
		PxConvexMeshGeometry geometry(physxMesh->_getConvex());
		PxTransform transform = toPxTransform(position, rotation);

		return overlap(geometry, transform, layer);
	}

	bool PhysXScene::boxOverlapAny(const AABox& box, const Quaternion& rotation, UINT64 layer) const
	{
		PxBoxGeometry geometry(toPxVector(box.getHalfSize()));
		PxTransform transform = toPxTransform(box.getCenter(), rotation);

		return overlapAny(geometry, transform, layer);
	}

	bool PhysXScene::sphereOverlapAny(const Sphere& sphere, UINT64 layer) const
	{
		PxSphereGeometry geometry(sphere.getRadius());
		PxTransform transform = toPxTransform(sphere.getCenter(), Quaternion::IDENTITY);

		return overlapAny(geometry, transform, layer);
	}

	bool PhysXScene::capsuleOverlapAny(const Capsule& capsule, const Quaternion& rotation,
		UINT64 layer) const
	{
		PxCapsuleGeometry geometry(capsule.getRadius(), capsule.getHeight() * 0.5f);
		PxTransform transform = toPxTransform(capsule.getCenter(), Quaternion::IDENTITY);

		return overlapAny(geometry, transform, layer);
	}

	bool PhysXScene::convexOverlapAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
		UINT64 layer) const
	{
		if (mesh == nullptr)
			return false;

		if (mesh->getType() != PhysicsMeshType::Convex)
			return false;

		FPhysXMesh* physxMesh = static_cast<FPhysXMesh*>(mesh->_getInternal());
		PxConvexMeshGeometry geometry(physxMesh->_getConvex());
		PxTransform transform = toPxTransform(position, rotation);

		return overlapAny(geometry, transform, layer);
	}

	bool PhysXScene::sweep(const PxGeometry& geometry, const PxTransform& tfrm, const Vector3& unitDir,
		PhysicsQueryHit& hit, UINT64 layer, float maxDist) const
	{
		PxSweepBuffer output;

		PxQueryFilterData filterData;
		memcpy(&filterData.data.word0, &layer, sizeof(layer));

		bool wasHit = mScene->sweep(geometry, tfrm, toPxVector(unitDir), maxDist, output,
			PxHitFlag::eDEFAULT | PxHitFlag::eUV, filterData);

		if (wasHit)
			parseHit(output.block, hit);

		return wasHit;
	}

	bool PhysXScene::overlapAny(const PxGeometry& geometry, const PxTransform& tfrm, UINT64 layer) const
	{
		PxOverlapBuffer output;

		PxQueryFilterData filterData;
		filterData.flags |= PxQueryFlag::eANY_HIT;
		memcpy(&filterData.data.word0, &layer, sizeof(layer));

		return mScene->overlap(geometry, tfrm, output, filterData);
	}

	Vector<Collider*> PhysXScene::overlap(const PxGeometry& geometry, const PxTransform& tfrm, UINT64 layer) const
	{
		PhysXOverlapQueryCallback output;

		PxQueryFilterData filterData;
		memcpy(&filterData.data.word0, &layer, sizeof(layer));

		mScene->overlap(geometry, tfrm, output, filterData);
		return output.data;
	}

	void PhysXScene::setFlag(PhysicsFlags flag, bool enabled)
	{
		PhysicsScene::setFlag(flag, enabled);

		mCharManager->setOverlapRecoveryModule(mFlags.isSet(PhysicsFlag::CCT_OverlapRecovery));
		mCharManager->setPreciseSweeps(mFlags.isSet(PhysicsFlag::CCT_PreciseSweeps));
		mCharManager->setTessellation(mFlags.isSet(PhysicsFlag::CCT_Tesselation), mTesselationLength);
	}

	Vector3 PhysXScene::getGravity() const
	{
		return fromPxVector(mScene->getGravity());
	}

	void PhysXScene::setGravity(const Vector3& gravity)
	{
		mScene->setGravity(toPxVector(gravity));
	}

	void PhysXScene::setMaxTesselationEdgeLength(float length)
	{
		mTesselationLength = length;

		mCharManager->setTessellation(mFlags.isSet(PhysicsFlag::CCT_Tesselation), mTesselationLength);
	}

	UINT32 PhysXScene::addBroadPhaseRegion(const AABox& region)
	{
		UINT32 id = mNextRegionIdx++;

		PxBroadPhaseRegion pxRegion;
		pxRegion.bounds = PxBounds3(toPxVector(region.getMin()), toPxVector(region.getMax()));
		pxRegion.userData = (void*)(UINT64)id;

		UINT32 handle = mScene->addBroadPhaseRegion(pxRegion, true);
		mBroadPhaseRegionHandles[id] = handle;

		return handle;
	}

	void PhysXScene::removeBroadPhaseRegion(UINT32 regionId)
	{
		auto iterFind = mBroadPhaseRegionHandles.find(regionId);
		if (iterFind == mBroadPhaseRegionHandles.end())
			return;

		mScene->removeBroadPhaseRegion(iterFind->second);
		mBroadPhaseRegionHandles.erase(iterFind);
	}

	void PhysXScene::clearBroadPhaseRegions()
	{
		for(auto& entry : mBroadPhaseRegionHandles)
			mScene->removeBroadPhaseRegion(entry.second);

		mBroadPhaseRegionHandles.clear();
	}

	PhysX& gPhysX()
	{
		return static_cast<PhysX&>(PhysX::instance());
	}
}