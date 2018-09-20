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
#include "Particles/BsVectorField.h"
#include "Mesh/BsMesh.h"
#include "CoreThread/BsCoreObjectSync.h"

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
		: mSortedList(&evolverCompareCallback)
	{ }

	RTTITypeBase* ParticleSystemEvolvers::getRTTIStatic()
	{
		return ParticleSystemEvolversRTTI::instance();
	}

	RTTITypeBase* ParticleSystemEvolvers::getRTTI() const
	{
		return getRTTIStatic();
	}

	template<bool Core>
	template<class Processor>
	void TParticleSystemSettings<Core>::rttiProcess(Processor p)
	{
		p << gpuSimulation;
		p << simulationSpace;
		p << orientation;
		p << orientationPlane;
		p << orientationLockY;
		p << duration;
		p << isLooping;
		p << sortMode;
		p << material;
		p << useAutomaticBounds;
		p << customBounds;
		p << renderMode;
		p << mesh;
	}

	template<bool Core>
	template<class Processor>
	void TParticleVectorFieldSettings<Core>::rttiProcess(Processor p)
	{
		p << intensity;
		p << tightness;
		p << scale;
		p << offset;
		p << rotation;
		p << rotationRate;
		p << tilingX;
		p << tilingY;
		p << tilingZ;
		p << vectorField;
	}

	RTTITypeBase* ParticleVectorFieldSettings::getRTTIStatic()
	{
		return ParticleVectorFieldSettingsRTTI::instance();
	}

	RTTITypeBase* ParticleVectorFieldSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template<class Processor>
	void ParticleDepthCollisionSettings::rttiProcess(Processor p)
	{
		p << enabled;
		p << restitution;
		p << dampening;
		p << radiusScale;
	}

	RTTITypeBase* ParticleDepthCollisionSettings::getRTTIStatic()
	{
		return ParticleDepthCollisionSettingsRTTI::instance();
	}

	RTTITypeBase* ParticleDepthCollisionSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	RTTITypeBase* ParticleGpuSimulationSettings::getRTTIStatic()
	{
		return ParticleGpuSimulationSettingsRTTI::instance();
	}

	RTTITypeBase* ParticleGpuSimulationSettings::getRTTI() const
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

	void ParticleSystem::setGpuSimulationSettings(const ParticleGpuSimulationSettings& settings)
	{
		mGpuSimulationSettings = settings;
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

		float timeStep;
		const float newTime = _advanceTime(mTime, timeDelta, mSettings.duration, mSettings.isLooping, timeStep);

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
		state.gpuSimulated = mSettings.gpuSimulation;
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

			const auto& evolverList = mEvolvers.mSortedList;

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

	float ParticleSystem::_advanceTime(float time, float timeDelta, float duration, bool loop, float& timeStep)
	{
		timeStep = timeDelta;
		float newTime = time + timeStep;
		if(newTime >= duration)
		{
			if(loop)
				newTime = fmod(newTime, duration);
			else
			{
				timeStep = time - duration;
				newTime = duration;
			}
		}

		return newTime;
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
		UINT32 size = getActorSyncDataSize() + rttiGetElemSize(getCoreDirtyFlags());

		mSettings.rttiProcess(RttiCoreSyncSize(size));
		mGpuSimulationSettings.vectorField.rttiProcess(RttiCoreSyncSize(size));
		size += rttiGetElemSize(mGpuSimulationSettings.colorOverLifetime);
		size += rttiGetElemSize(mGpuSimulationSettings.sizeScaleOverLifetime);
		size += rttiGetElemSize(mGpuSimulationSettings.acceleration);
		size += rttiGetElemSize(mGpuSimulationSettings.drag);
		mGpuSimulationSettings.depthCollision.rttiProcess(RttiCoreSyncSize(size));

		UINT8* data = allocator->alloc(size);
		char* dataPtr = (char*)data;
		dataPtr = syncActorTo(dataPtr);
		dataPtr = rttiWriteElem(getCoreDirtyFlags(), dataPtr);

		mSettings.rttiProcess(RttiCoreSyncWriter(&dataPtr));
		mGpuSimulationSettings.vectorField.rttiProcess(RttiCoreSyncWriter(&dataPtr));

		dataPtr = rttiWriteElem(mGpuSimulationSettings.colorOverLifetime, dataPtr);
		dataPtr = rttiWriteElem(mGpuSimulationSettings.sizeScaleOverLifetime, dataPtr);
		dataPtr = rttiWriteElem(mGpuSimulationSettings.acceleration, dataPtr);
		dataPtr = rttiWriteElem(mGpuSimulationSettings.drag, dataPtr);
		mGpuSimulationSettings.depthCollision.rttiProcess(RttiCoreSyncWriter(&dataPtr));

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

			mSettings.rttiProcess(RttiCoreSyncReader(&dataPtr));
			mGpuSimulationSettings.vectorField.rttiProcess(RttiCoreSyncReader(&dataPtr));

			dataPtr = rttiReadElem(mGpuSimulationSettings.colorOverLifetime, dataPtr);
			dataPtr = rttiReadElem(mGpuSimulationSettings.sizeScaleOverLifetime, dataPtr);
			dataPtr = rttiReadElem(mGpuSimulationSettings.acceleration, dataPtr);
			dataPtr = rttiReadElem(mGpuSimulationSettings.drag, dataPtr);
			mGpuSimulationSettings.depthCollision.rttiProcess(RttiCoreSyncReader(&dataPtr));

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
