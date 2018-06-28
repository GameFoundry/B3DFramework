//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Particles/BsParticleSystem.h"
#include "Particles/BsParticleManager.h"
#include "Particles/BsParticleEmitter.h"
#include "Particles/BsParticleEvolver.h"
#include "Private/Particles/BsParticleSet.h"
#include "Private/RTTI/BsParticleSystemRTTI.h"
#include "Allocators/BsPoolAlloc.h"
#include "Material/BsMaterial.h"
#include "Renderer/BsCamera.h"
#include "Renderer/BsRenderer.h"

namespace bs
{
	static constexpr UINT32 INITIAL_PARTICLE_CAPACITY = 1000;

	void ParticleSystemBase::setSimulationSpace(ParticleSimulationSpace value)
	{
		if(mSimulationSpace == value)
			return;

		mSimulationSpace = value;
		_markCoreDirty();
	}

	ParticleSystem::ParticleSystem()
	{
		mId = ParticleManager::instance().registerParticleSystem(this);
		mSeed = rand();
	}

	ParticleSystem::~ParticleSystem()
	{
		ParticleManager::instance().unregisterParticleSystem(this);

		if(mParticleSet)
			bs_delete(mParticleSet);
	}

	void ParticleSystem::setManualSeed(UINT32 seed)
	{
		mManualSeed = seed;

		if(!mUseAutomaticSeed)
		{
			mSeed = seed;
			mRandom.setSeed(seed);
		}
	}

	void ParticleSystem::setUseAutomaticSeed(bool enable)
	{
		if(mUseAutomaticSeed == enable)
			return;

		if(enable)
			mSeed = rand();
		else
			mSeed = mManualSeed;

		mRandom.setSeed(mSeed);
	}

	void ParticleSystem::setMaxParticles(UINT32 value)
	{
		if(mMaxParticles == value)
			return;

		mParticleSet->clear(value);
		mMaxParticles = value;
	}

	void ParticleSystem::play()
	{
		if(mState == State::Playing)
			return;

		if(mState == State::Uninitialized)
		{
			UINT32 particleCapacity = std::min(mMaxParticles, INITIAL_PARTICLE_CAPACITY);
			mParticleSet = bs_new<ParticleSet>(particleCapacity);
		}

		mState = State::Playing;
		mTime = 0.0f;
		mRandom.setSeed(mSeed);
	}

	void ParticleSystem::pause()
	{
		if(mState == State::Playing)
			mState = State::Paused;
	}

	void ParticleSystem::stop()
	{
		if(mState != State::Playing && mState != State::Paused)
			return;

		mState = State::Stopped;
		mParticleSet->clear();
	}

	void ParticleSystem::_simulate(float timeDelta)
	{
		if(mState != State::Playing)
			return;

		float newTime = mTime;

		float timeStep = timeDelta;
		if(newTime >= mDuration)
		{
			if(mIsLooping)
				newTime = fmod(newTime, mDuration);
			else
			{
				timeStep = mTime - mDuration;
				newTime = mDuration;
			}
		}
		else
			newTime += timeDelta;

		if(timeStep < 0.00001f)
			return;

		// Spawn new particles
		ParticleEmitterState emitterState; // TODO - Needs to be populated with skinning information
		emitterState.time = newTime;
		emitterState.length = mDuration;
		emitterState.timeStep = timeStep;
		emitterState.maxParticles = mMaxParticles;
		emitterState.worldSpace = mSimulationSpace == ParticleSimulationSpace::World;
		emitterState.transform = mTransform.getMatrix();

		for(auto& emitter : mEmitters)
			emitter->spawn(mRandom, emitterState, *mParticleSet);

		// Simulate
		for(auto& evolver : mEvolvers)
			evolver->evolve(newTime, mRandom, *mParticleSet);

		// Update position
		UINT32 numParticles = mParticleSet->getParticleCount();
		const ParticleSetData& particles = mParticleSet->getParticles();

		for(UINT32 i = 0; i < numParticles; i++)
			particles.position[i] += particles.velocity[i] * timeStep;

		// Decrement lifetime
		for(UINT32 i = 0; i < numParticles; i++)
			particles.lifetime[i] -= timeStep;

		// Kill expired particles
		for(UINT32 i = 0; i < numParticles;)
		{
			// TODO - Upon freeing a particle don't immediately remove it to save on swap, since we will be immediately
			// spawning new particles. Perhaps keep a list of recently removed particles so it can immediately be
			// re-used for spawn, and then only after spawn remove extra particles
			if(particles.lifetime[i] <= 0.0f)
			{
				mParticleSet->freeParticle(i);
				numParticles--;
			}
			else
				i++;
		}

		mTime = newTime;
	}

	AABox ParticleSystem::_calculateBounds() const
	{
		// TODO - If evolvers are deterministic (as well as their properties), calculate the maximinal bounds in an
		// analytical way

		const UINT32 particleCount = mParticleSet->getParticleCount();
		if(particleCount == 0)
			return AABox::BOX_EMPTY;

		const ParticleSetData& particles = mParticleSet->getParticles();
		AABox bounds(Vector3::INF, -Vector3::INF);
		for(UINT32 i = 0; i < particleCount; i++)
			bounds.merge(particles.position[i]);

		return bounds;
	}

	SPtr<ct::ParticleSystem> ParticleSystem::getCore() const
	{
		return std::static_pointer_cast<ct::ParticleSystem>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> ParticleSystem::createCore() const
	{
		ct::ParticleSystem* rawPtr = new (bs_alloc<ct::ParticleSystem>()) ct::ParticleSystem(mId);
		SPtr<ct::ParticleSystem> ptr = bs_shared_ptr<ct::ParticleSystem>(rawPtr);
		ptr->_setThisPtr(ptr);

		return ptr;
	}

	void ParticleSystem::_markCoreDirty(ActorDirtyFlag flag)
	{
		markCoreDirty((UINT32)flag);
	}

	CoreSyncData ParticleSystem::syncToCore(FrameAlloc* allocator)
	{
		const UINT32 size = 
			getActorSyncDataSize() +
			rttiGetElemSize(getCoreDirtyFlags()) +
			rttiGetElemSize(mSimulationSpace) +
			rttiGetElemSize(mOrientation) +
			rttiGetElemSize(mOrientationPlane) +
			rttiGetElemSize(mOrientationLockY) +
			rttiGetElemSize(mSortMode) + 
			sizeof(SPtr<ct::Material>);

		UINT8* data = allocator->alloc(size);
		char* dataPtr = (char*)data;
		dataPtr = syncActorTo(dataPtr);
		dataPtr = rttiWriteElem(getCoreDirtyFlags(), dataPtr);
		dataPtr = rttiWriteElem(mSimulationSpace, dataPtr);
		dataPtr = rttiWriteElem(mOrientation, dataPtr);
		dataPtr = rttiWriteElem(mOrientationPlane, dataPtr);
		dataPtr = rttiWriteElem(mOrientationLockY, dataPtr);
		dataPtr = rttiWriteElem(mSortMode, dataPtr);

		SPtr<ct::Material>* material = new (dataPtr) SPtr<ct::Material>();
		if (mMaterial.isLoaded())
			*material = mMaterial->getCore();

		dataPtr += sizeof(SPtr<ct::Material>);

		return CoreSyncData(data, size);
	}
	SPtr<ParticleSystem> ParticleSystem::create()
	{
		SPtr<ParticleSystem> ptr = createEmpty();
		ptr->initialize();

		return ptr;
	}

	SPtr<ParticleSystem> ParticleSystem::createEmpty()
	{
		ParticleSystem* rawPtr = new (bs_alloc<ParticleSystem>()) ParticleSystem();
		SPtr<ParticleSystem> ptr = bs_core_ptr<ParticleSystem>(rawPtr);
		ptr->_setThisPtr(ptr);

		return ptr;
	}

	RTTITypeBase* ParticleSystem::getRTTIStatic()
	{
		return ParticleSystemRTTI::instance();
	}

	RTTITypeBase* ParticleSystem::getRTTI() const
	{
		return ParticleSystem::getRTTIStatic();
	}

	namespace ct
	{
		ParticleSystem::~ParticleSystem()
		{
			if(mActive)
				gRenderer()->notifyParticleSystemRemoved(this);
		}

		void ParticleSystem::initialize()
		{
			gRenderer()->notifyParticleSystemAdded(this);
		}

		void ParticleSystem::syncToCore(const CoreSyncData& data)
		{
			char* dataPtr = (char*)data.getBuffer();

			UINT32 dirtyFlags = 0;
			const bool oldIsActive = mActive;

			dataPtr = syncActorFrom(dataPtr);
			dataPtr = rttiReadElem(dirtyFlags, dataPtr);
			dataPtr = rttiReadElem(mSimulationSpace, dataPtr);
			dataPtr = rttiReadElem(mOrientation, dataPtr);
			dataPtr = rttiReadElem(mOrientationPlane, dataPtr);
			dataPtr = rttiReadElem(mOrientationLockY, dataPtr);
			dataPtr = rttiReadElem(mSortMode, dataPtr);

			SPtr<Material>* material = (SPtr<Material>*)dataPtr;
			mMaterial = *material;
			material->~SPtr<Material>();
			dataPtr += sizeof(SPtr<Material>);

			constexpr UINT32 updateEverythingFlag = (UINT32)ActorDirtyFlag::Everything
				| (UINT32)ActorDirtyFlag::Active
				| (UINT32)ActorDirtyFlag::Dependency;

			if ((dirtyFlags & updateEverythingFlag) != 0)
			{
				if (oldIsActive != mActive)
				{
					if (mActive)
						gRenderer()->notifyParticleSystemAdded(this);
					else
						gRenderer()->notifyParticleSystemRemoved(this);
				}
				else
				{
					gRenderer()->notifyParticleSystemRemoved(this);
					gRenderer()->notifyParticleSystemAdded(this);
				}
			}
			else if ((dirtyFlags & (UINT32)ActorDirtyFlag::Mobility) != 0)
			{
				gRenderer()->notifyParticleSystemRemoved(this);
				gRenderer()->notifyParticleSystemAdded(this);
			}
			else if ((dirtyFlags & (UINT32)ActorDirtyFlag::Transform) != 0)
			{
				if (mActive)
					gRenderer()->notifyParticleSystemUpdated(this);
			}
		}
	}
}
