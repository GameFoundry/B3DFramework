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
#include "Physics/BsPhysics.h"

namespace bs
{
	static constexpr UINT32 INITIAL_PARTICLE_CAPACITY = 1000;

	bool evolverCompareCallback(const ParticleEvolver* a, const ParticleEvolver* b)
	{
		if (a->getProperties().priority == b->getProperties().priority)
			return a > b; // Use address, at this point it doesn't matter, but std::set requires us to differentiate
		else
			return a->getProperties().priority > b->getProperties().priority;
	}

	RTTITypeBase* ParticleSystemSettings::getRTTIStatic()
	{
		return ParticleSystemSettingsRTTI::instance();
	}

	RTTITypeBase* ParticleSystemSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	RTTITypeBase* ParticleSystemEmitters::getRTTIStatic()
	{
		return ParticleSystemEmittersRTTI::instance();
	}

	RTTITypeBase* ParticleSystemEmitters::getRTTI() const
	{
		return getRTTIStatic();
	}

	ParticleSystemEvolvers::ParticleSystemEvolvers()
		: mSortedListCPU(&evolverCompareCallback), mSortedListGPU(&evolverCompareCallback)
	{ }

	RTTITypeBase* ParticleSystemEvolvers::getRTTIStatic()
	{
		return ParticleSystemEvolversRTTI::instance();
	}

	RTTITypeBase* ParticleSystemEvolvers::getRTTI() const
	{
		return getRTTIStatic();
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
		
	void ParticleSystem::setSettings(const ParticleSystemSettings& settings)
	{
		if(settings.useAutomaticSeed != mSettings.useAutomaticSeed)
		{
			if(settings.useAutomaticSeed)
				mSeed = rand();
			else
				mSeed = settings.manualSeed;

			mRandom.setSeed(mSeed);
		}
		else
		{
			if(!settings.useAutomaticSeed)
			{
				mSeed = settings.manualSeed;
				mRandom.setSeed(mSeed);
			}
		}

		if(settings.maxParticles < mSettings.maxParticles)
			mParticleSet->clear(settings.maxParticles);

		mSettings = settings; 
		_markCoreDirty();
	}

	void ParticleSystem::play()
	{
		if(mState == State::Playing)
			return;

		if(mState == State::Uninitialized)
		{
			UINT32 particleCapacity = std::min(mSettings.maxParticles, INITIAL_PARTICLE_CAPACITY);
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

	void ParticleSystem::_simulate(float timeDelta, const EvaluatedAnimationData* animData)
	{
		if(mState != State::Playing)
			return;

		float newTime = mTime;

		float timeStep = timeDelta;
		if(newTime >= mSettings.duration)
		{
			if(mSettings.isLooping)
				newTime = fmod(newTime, mSettings.duration);
			else
			{
				timeStep = mTime - mSettings.duration;
				newTime = mSettings.duration;
			}
		}
		else
			newTime += timeDelta;

		if(timeStep < 0.00001f)
			return;

		// Generate per-frame state
		ParticleSystemState state;
		state.time = newTime;
		state.nrmTime = newTime / mSettings.duration;
		state.length = mSettings.duration;
		state.timeStep = timeStep;
		state.maxParticles = mSettings.maxParticles;
		state.worldSpace = mSettings.simulationSpace == ParticleSimulationSpace::World;
		state.localToWorld = mTransform.getMatrix();
		state.worldToLocal = state.localToWorld.inverseAffine();
		state.system = this;
		state.animData = animData;

		// For GPU simulation we only care about newly spawned particles, so clear old ones
		if(mSettings.gpuSimulation)
			mParticleSet->clear();

		// Spawn new particles
		for(auto& emitter : mEmitters.mList)
			emitter->spawn(mRandom, state, *mParticleSet);

		// Simulate if running on CPU, otherwise just pass the spawned particles off to the core thread
		if(!mSettings.gpuSimulation)
		{
			UINT32 numParticles = mParticleSet->getParticleCount();
			const ParticleSetData& particles = mParticleSet->getParticles();

			// Remember old positions
			for (UINT32 i = 0; i < numParticles; i++)
				particles.prevPosition[i] = particles.position[i];

			const auto& evolverList = mEvolvers.mSortedListCPU;

			// Evolve pre-simulation
			auto evolverIter = evolverList.begin();
			for (; evolverIter != evolverList.end(); ++evolverIter)
			{
				ParticleEvolver* evolver = *evolverIter;
				const ParticleEvolverProperties& props = evolver->getProperties();

				if (props.priority < 0)
					break;

				evolver->evolve(mRandom, state, *mParticleSet);
			}

			// Simulate
			for (UINT32 i = 0; i < numParticles; i++)
				particles.position[i] += particles.velocity[i] * timeStep;

			// Evolve post-simulation
			for (; evolverIter != evolverList.end(); ++evolverIter)
			{
				ParticleEvolver* evolver = *evolverIter;
				evolver->evolve(mRandom, state, *mParticleSet);
			}

			// Decrement lifetime
			for (UINT32 i = 0; i < numParticles; i++)
				particles.lifetime[i] -= timeStep;

			// Kill expired particles
			for (UINT32 i = 0; i < numParticles;)
			{
				// TODO - Upon freeing a particle don't immediately remove it to save on swap, since we will be immediately
				// spawning new particles. Perhaps keep a list of recently removed particles so it can immediately be
				// re-used for spawn, and then only after spawn remove extra particles
				if (particles.lifetime[i] <= 0.0f)
				{
					mParticleSet->freeParticle(i);
					numParticles--;
				}
				else
					i++;
			}
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
			rttiGetElemSize(mSettings.gpuSimulation) +
			rttiGetElemSize(mSettings.simulationSpace) +
			rttiGetElemSize(mSettings.orientation) +
			rttiGetElemSize(mSettings.orientationPlane) +
			rttiGetElemSize(mSettings.orientationLockY) +
			rttiGetElemSize(mSettings.sortMode) + 
			sizeof(SPtr<ct::Material>);

		UINT8* data = allocator->alloc(size);
		char* dataPtr = (char*)data;
		dataPtr = syncActorTo(dataPtr);
		dataPtr = rttiWriteElem(getCoreDirtyFlags(), dataPtr);
		dataPtr = rttiWriteElem(mSettings.gpuSimulation, dataPtr);
		dataPtr = rttiWriteElem(mSettings.simulationSpace, dataPtr);
		dataPtr = rttiWriteElem(mSettings.orientation, dataPtr);
		dataPtr = rttiWriteElem(mSettings.orientationPlane, dataPtr);
		dataPtr = rttiWriteElem(mSettings.orientationLockY, dataPtr);
		dataPtr = rttiWriteElem(mSettings.sortMode, dataPtr);

		SPtr<ct::Material>* material = new (dataPtr) SPtr<ct::Material>();
		if (mSettings.material.isLoaded())
			*material = mSettings.material->getCore();

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
			dataPtr = rttiReadElem(mSettings.gpuSimulation, dataPtr);
			dataPtr = rttiReadElem(mSettings.simulationSpace, dataPtr);
			dataPtr = rttiReadElem(mSettings.orientation, dataPtr);
			dataPtr = rttiReadElem(mSettings.orientationPlane, dataPtr);
			dataPtr = rttiReadElem(mSettings.orientationLockY, dataPtr);
			dataPtr = rttiReadElem(mSettings.sortMode, dataPtr);

			SPtr<Material>* material = (SPtr<Material>*)dataPtr;
			mSettings.material = *material;
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
					if(mActive)
						gRenderer()->notifyParticleSystemUpdated(this, false);
				}
			}
			else if ((dirtyFlags & ((UINT32)ActorDirtyFlag::Mobility | (UINT32)ActorDirtyFlag::Transform)) != 0)
				gRenderer()->notifyParticleSystemUpdated(this, true);
		}
	}
}
