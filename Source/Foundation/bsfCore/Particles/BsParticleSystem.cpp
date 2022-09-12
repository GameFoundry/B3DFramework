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
#include "Scene/BsSceneManager.h"

namespace bs
{
	static constexpr UINT32 INITIAL_PARTICLE_CAPACITY = 1000;

	RTTITypeBase* ParticleSystemSettings::getRTTIStatic()
	{
		return ParticleSystemSettingsRTTI::Instance();
	}

	RTTITypeBase* ParticleSystemSettings::getRTTI() const
	{
		return GetRTTIStatic();
	}

	template <bool Core>
	template <class P>
	void TParticleSystemSettings<Core>::rttiEnumFields(P p)
	{
		p(gpuSimulation);
		p(simulationSpace);
		p(orientation);
		p(orientationPlaneNormal);
		p(orientationLockY);
		p(duration);
		p(isLooping);
		p(sortMode);
		p(material);
		p(useAutomaticBounds);
		p(customBounds);
		p(renderMode);
		p(mesh);
	}

	template<bool Core>
	template<class P>
	void TParticleVectorFieldSettings<Core>::rttiEnumFields(P p)
	{
		p(intensity);
		p(tightness);
		p(scale);
		p(offset);
		p(rotation);
		p(rotationRate);
		p(tilingX);
		p(tilingY);
		p(tilingZ);
		p(vectorField);
	}

	RTTITypeBase* ParticleVectorFieldSettings::getRTTIStatic()
	{
		return ParticleVectorFieldSettingsRTTI::Instance();
	}

	RTTITypeBase* ParticleVectorFieldSettings::getRTTI() const
	{
		return GetRTTIStatic();
	}

	template<class P>
	void ParticleDepthCollisionSettings::RttiEnumFields(P p)
	{
		p(enabled);
		p(restitution);
		p(dampening);
		p(radiusScale);
	}

	RTTITypeBase* ParticleDepthCollisionSettings::getRTTIStatic()
	{
		return ParticleDepthCollisionSettingsRTTI::Instance();
	}

	RTTITypeBase* ParticleDepthCollisionSettings::getRTTI() const
	{
		return GetRTTIStatic();
	}

	template<bool Core>
	template<class P>
	void TParticleGpuSimulationSettings<Core>::rttiEnumFields(P p)
	{
		p(colorOverLifetime);
		p(sizeScaleOverLifetime);
		p(acceleration);
		p(drag);
		p(depthCollision);
		p(vectorField);
	};

	RTTITypeBase* ParticleGpuSimulationSettings::getRTTIStatic()
	{
		return ParticleGpuSimulationSettingsRTTI::Instance();
	}

	RTTITypeBase* ParticleGpuSimulationSettings::getRTTI() const
	{
		return GetRTTIStatic();
	}

	ParticleSystem::ParticleSystem()
	{
		mId = ParticleManager::instance().RegisterParticleSystem(this);
		mSeed = rand();

		auto emitter = bs_shared_ptr_new<ParticleEmitter>();

		PARTICLE_SPHERE_SHAPE_DESC desc;
		desc.radius = 0.05f;

		emitter->SetShape(ParticleEmitterSphereShape::create(desc));

		mEmitters = { emitter };
	}

	ParticleSystem::~ParticleSystem()
	{
		ParticleManager::instance().UnregisterParticleSystem(this);

		if(mParticleSet)
			bs_delete(mParticleSet);
	}
		
	void ParticleSystem::SetSettings(const ParticleSystemSettings& settings)
	{
		if(settings.useAutomaticSeed != mSettings.useAutomaticSeed)
		{
			if(settings.useAutomaticSeed)
				mSeed = rand();
			else
				mSeed = settings.manualSeed;

			mRandom.SetSeed(mSeed);
		}
		else
		{
			if(!settings.useAutomaticSeed)
			{
				mSeed = settings.manualSeed;
				mRandom.SetSeed(mSeed);
			}
		}

		if(settings.maxParticles < mSettings.maxParticles)
			mParticleSet->Clear(settings.maxParticles);

		mSettings = settings;
		_markCoreDirty();
		markDependenciesDirty();
	}

	void ParticleSystem::SetGpuSimulationSettings(const ParticleGpuSimulationSettings& settings)
	{
		mGpuSimulationSettings = settings;
		_markCoreDirty();
	}

	void ParticleSystem::SetLayer(UINT64 layer)
	{
		const bool isPow2 = layer && !((layer - 1) & layer);

		if (!isPow2)
		{
			BS_LOG(Warning, Particles, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
			return;
		}

		mLayer = layer;
		_markCoreDirty();
	}	

	void ParticleSystem::SetEmitters(const Vector<SPtr<ParticleEmitter>>& emitters)
	{
		mEmitters = emitters;
		_markCoreDirty();
	}

	void ParticleSystem::SetEvolvers(const Vector<SPtr<ParticleEvolver>>& evolvers)
	{
		mEvolvers = evolvers;

		std::sort(mEvolvers.begin(), mEvolvers.end(),
			[](const SPtr<ParticleEvolver>& a, const SPtr<ParticleEvolver>& b)
		{
			INT32 priorityA = a ? a->GetProperties().priority : 0;
			INT32 priorityB = b ? b->GetProperties().priority : 0;

			if (priorityA == priorityB)
				return a > b; // Use address, at this point it doesn't matter, but sorting requires us to differentiate
			else
				return priorityA > priorityB;
		});

		_markCoreDirty();
	}

	void ParticleSystem::Play()
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
		mRandom.SetSeed(mSeed);
	}

	void ParticleSystem::Pause()
	{
		if(mState == State::Playing)
			mState = State::Paused;
	}

	void ParticleSystem::Stop()
	{
		if(mState != State::Playing && mState != State::Paused)
			return;

		mState = State::Stopped;
		mParticleSet->Clear();
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
		state.timeStart = mTime;
		state.timeEnd = newTime;
		state.nrmTimeStart = state.timeStart / mSettings.duration;
		state.nrmTimeEnd = state.timeEnd / mSettings.duration;
		state.length = mSettings.duration;
		state.timeStep = timeStep;
		state.maxParticles = mSettings.maxParticles;
		state.worldSpace = mSettings.simulationSpace == ParticleSimulationSpace::World;
		state.gpuSimulated = mSettings.gpuSimulation;
		state.localToWorld = mTransform.GetMatrix();
		state.worldToLocal = state.localToWorld.InverseAffine();
		state.system = this;
		state.scene = (mScene && mScene->IsActive()) ? mScene.Get() : gSceneManager().getMainScene().get();
		state.animData = animData;

		// For GPU simulation we only care about newly spawned particles, so clear old ones
		if(mSettings.gpuSimulation)
			mParticleSet->Clear();

		// Spawn new particles
		for(auto& emitter : mEmitters)
		{
			if(emitter)
				emitter->Spawn(mRandom, state, *mParticleSet);
		}

		// Simulate if running on CPU, otherwise just pass the spawned particles off to the core thread
		if(!mSettings.gpuSimulation)
		{
			const UINT32 numParticles = mParticleSet->GetParticleCount();

			preSimulate(state, 0, numParticles, false, 0.0f);
			simulate(state, 0, numParticles, false, 0.0f);
			postSimulate(state, 0, numParticles, false, 0.0f);
		}

		mTime = newTime;
	}

	void ParticleSystem::preSimulate(const ParticleSystemState& state, UINT32 startIdx, UINT32 count, bool spacing,
		float spacingOffset)
	{
		const ParticleSetData& particles = mParticleSet->GetParticles();
		const float subFrameSpacing = (spacing && count > 0) ? 1.0f / count : 1.0f;
		const UINT32 endIdx = startIdx + count;

		// Decrement lifetime
		for (UINT32 i = startIdx; i < endIdx; i++)
		{
			float timeStep = state.timeStep;
			if(spacing)
			{
				// Note: We're calculating this in a few places during a single frame. Store it and re-use?
				const UINT32 localIdx = i - startIdx;
				const float subFrameOffset = ((float)localIdx + spacingOffset) * subFrameSpacing;
				timeStep *= subFrameOffset;
			}

			particles.lifetime[i] -= timeStep;
		}

		// Kill expired particles
		UINT32 numParticles = count;
		for (UINT32 i = 0; i < numParticles;)
		{
			const UINT32 particleIdx = startIdx + i;
			if (particles.lifetime[particleIdx] <= 0.0f)
			{
				mParticleSet->FreeParticle(particleIdx);
				numParticles--;
			}
			else
				i++;
		}

		// Remember old positions
		for (UINT32 i = startIdx; i < endIdx; i++)
			particles.prevPosition[i] = particles.position[i];

		// Evolve pre-simulation
		for(auto& evolver : mEvolvers)
		{
			if(!evolver)
				continue;

			const ParticleEvolverProperties& props = evolver->GetProperties();
			if (props.priority < 0)
				break;

			evolver->Evolve(mRandom, state, *mParticleSet, startIdx, count, spacing, spacingOffset);
		}
	}

	void ParticleSystem::simulate(const ParticleSystemState& state, UINT32 startIdx, UINT32 count, bool spacing,
		float spacingOffset)
	{
		const ParticleSetData& particles = mParticleSet->GetParticles();
		const float subFrameSpacing = (spacing && count > 0) ? 1.0f / count : 1.0f;
		const UINT32 endIdx = startIdx + count;

		for (UINT32 i = startIdx; i < endIdx; i++)
		{
			float timeStep = state.timeStep;
			if(spacing)
			{
				const UINT32 localIdx = i - startIdx;
				const float subFrameOffset = ((float)localIdx + spacingOffset) * subFrameSpacing;
				timeStep *= subFrameOffset;
			}

			particles.position[i] += particles.velocity[i] * timeStep;
		}
	}

	void ParticleSystem::postSimulate(const ParticleSystemState& state, UINT32 startIdx, UINT32 count, bool spacing,
		float spacingOffset)
	{
		// Evolve post-simulation
		for(auto& evolver : mEvolvers)
		{
			if(!evolver)
				continue;

			const ParticleEvolverProperties& props = evolver->GetProperties();
			if(props.priority >= 0)
				continue;

			evolver->Evolve(mRandom, state, *mParticleSet, startIdx, count, spacing, spacingOffset);
		}
	}

	AABox ParticleSystem::_calculateBounds() const
	{
		// TODO - If evolvers are deterministic (as well as their properties), calculate the maximinal bounds in an
		// analytical way

		const UINT32 particleCount = mParticleSet->GetParticleCount();
		if(particleCount == 0)
			return AABox::BOX_EMPTY;

		const ParticleSetData& particles = mParticleSet->GetParticles();
		AABox Bounds(Vector3::INF, -Vector3::INF);
		for(UINT32 i = 0; i < particleCount; i++)
			bounds.Merge(particles.position[i]);

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

	SPtr<ct::ParticleSystem> ParticleSystem::GetCore() const
	{
		return std::static_pointer_cast<ct::ParticleSystem>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> ParticleSystem::CreateCore() const
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

	CoreSyncData ParticleSystem::SyncToCore(FrameAlloc* allocator)
	{
		UINT32 size = rtti_size(getCoreDirtyFlags()).bytes;
		size += csync_size((SceneActor&)*this);
		size += csync_size(mSettings);
		size += csync_size(mGpuSimulationSettings);
		size += rtti_size(mLayer).bytes;

		UINT8* data = allocator->Alloc(size);
		Bitstream Stream(data, size);
		rtti_write(getCoreDirtyFlags(), stream);
		csync_write((SceneActor&)*this, stream);
		csync_write(mSettings, stream);
		csync_write(mGpuSimulationSettings, stream);
		rtti_write(mLayer, stream);

		return CoreSyncData(data, size);
	}

	void ParticleSystem::GetCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		if (mSettings.mesh.IsLoaded())
			dependencies.push_back(mSettings.mesh.Get());

		if (mSettings.material.IsLoaded())
			dependencies.push_back(mSettings.material.Get());
	}

	SPtr<ParticleSystem> ParticleSystem::Create()
	{
		SPtr<ParticleSystem> ptr = createEmpty();
		ptr->Initialize();

		return ptr;
	}

	SPtr<ParticleSystem> ParticleSystem::CreateEmpty()
	{
		ParticleSystem* rawPtr = new (bs_alloc<ParticleSystem>()) ParticleSystem();
		SPtr<ParticleSystem> ptr = bs_core_ptr<ParticleSystem>(rawPtr);
		ptr->_setThisPtr(ptr);

		return ptr;
	}

	RTTITypeBase* ParticleSystem::getRTTIStatic()
	{
		return ParticleSystemRTTI::Instance();
	}

	RTTITypeBase* ParticleSystem::getRTTI() const
	{
		return ParticleSystem::GetRTTIStatic();
	}

	namespace ct
	{
		ParticleSystem::~ParticleSystem()
		{
			if(mActive)
				gRenderer()->NotifyParticleSystemRemoved(this);
		}

		void ParticleSystem::Initialize()
		{
			gRenderer()->NotifyParticleSystemAdded(this);
		}

		void ParticleSystem::SetLayer(UINT64 layer)
		{
			const bool isPow2 = layer && !((layer - 1) & layer);

			if (!isPow2)
			{
				BS_LOG(Warning, Particles, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
				return;
			}

			mLayer = layer;
			_markCoreDirty();
		}

		void ParticleSystem::SyncToCore(const CoreSyncData& data)
		{
			Bitstream Stream((uint8_t*)data.GetBuffer(), data.getBufferSize());

			UINT32 dirtyFlags = 0;
			const bool oldIsActive = mActive;

			rtti_read(dirtyFlags, stream);
			csync_read((SceneActor&)*this, stream);
			csync_read(mSettings, stream);
			csync_read(mGpuSimulationSettings, stream);
			rtti_read(mLayer, stream);
			
			constexpr UINT32 updateEverythingFlag = (UINT32)ActorDirtyFlag::Everything
				| (UINT32)ActorDirtyFlag::Active
				| (UINT32)ActorDirtyFlag::Dependency;

			if ((dirtyFlags & updateEverythingFlag) != 0)
			{
				if (oldIsActive != mActive)
				{
					if (mActive)
						gRenderer()->NotifyParticleSystemAdded(this);
					else
						gRenderer()->NotifyParticleSystemRemoved(this);
				}
				else
				{
					if(mActive)
						gRenderer()->NotifyParticleSystemUpdated(this, false);
				}
			}
			else if ((dirtyFlags & ((UINT32)ActorDirtyFlag::Mobility | (UINT32)ActorDirtyFlag::Transform)) != 0)
				gRenderer()->NotifyParticleSystemUpdated(this, true);
		}
	}
}
